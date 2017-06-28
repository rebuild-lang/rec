﻿using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Text;
using System;
using REC.Tools;

namespace REC.Packaging.PortableExecutable
{
    internal class Image
    {
        ImageHeader Header { get; } = ImageHeader.WithDefaultSections();
        byte[] Code;
        private byte[] IData;
        byte[] Data;
        byte[] Reloc;

        private static uint Padding(uint value, uint alignment)
        {
            return (alignment - (value % alignment));
        }

        private static uint AlignTo(uint value, uint alignment)
        {
            return value + Padding(value, alignment);
        }

        private static uint TimeStamp()
        {
            return (uint)DateTime.UtcNow.Subtract(new DateTime(1970, 1, 1)).TotalSeconds;
        }

        public Image()
        {
            // dummy data
            Code = new byte[] {
                //0x6A, 0x05, // push 5
                0xE8, 0x00, 0x00, 0x00, 0x00, // call ip + 0x5
                0xFF, 0x15, 0x00, 0x00, 0x00, 0x00 // call dword ptr ds:0
            };
            var codeAddressReplacement = 7u; // 4u;

            // headers
            var HeaderDataSize = DosHeader.WriteSize + Header.WriteSize;
            var HeaderFileSize = AlignTo(HeaderDataSize, Header.FileAlignment);
            var HeaderVirtualSize = AlignTo(HeaderDataSize, Header.SectionAlignment);
            Header.SizeOfHeaders = HeaderFileSize;

            // code
            var CodeFileOffset = HeaderFileSize;
            var CodeVirtualAddress = HeaderVirtualSize;
            var CodeSize = (uint)Code.Length;
            var CodeFileSize = AlignTo(CodeSize, Header.FileAlignment);
            var CodeVirtualSize = AlignTo(CodeSize, Header.SectionAlignment);
            Header.BaseOfCode = CodeVirtualAddress;
            Header.SizeOfCode = CodeFileSize;
            {
                var text = Header.TextSection;
                text.VirtualSize = CodeSize;
                text.SizeOfRawData = CodeFileSize;
                text.VirtualAddress = CodeVirtualAddress;
                text.PointerToRawData = CodeFileOffset;
            }
            Header.AddressOfEntryPoint = CodeVirtualAddress;

            // import data
            var IDataFileOffset = CodeFileOffset + CodeFileSize;
            var IDataVirtualAddress = CodeVirtualAddress + CodeVirtualSize;
            using (var ms = new MemoryStream())
            {
                using (var bw = new BinaryWriter(ms))
                {
                    var import = new ImportSection(new ImportSection.DLL[]{
                        new ImportSection.DLL {
                            Name = "kernel32.dll",
                            Functions = new ImportSection.HintNameEntry[]
                            {
                                new ImportSection.HintNameEntry
                                {
                                    Hint = 346,
                                    Name = "ExitProcess"
                                }
                            }
                        }
                    }, Header.Magic, IDataVirtualAddress);

                    Header.DataDirectories[(uint)DataDirectoryIndex.ImportTable].VirtualAddress = import.ImportTableRVA;
                    Header.DataDirectories[(uint)DataDirectoryIndex.ImportTable].Size = import.ImportTableSize;

                    Header.DataDirectories[(uint)DataDirectoryIndex.ImportBindTable].VirtualAddress = import.BindTableRVA;
                    Header.DataDirectories[(uint)DataDirectoryIndex.ImportBindTable].Size = import.BindTableSize;

                    var bindRVA = import.FunctionBindRVA(0, 0);
                    InjectAddress(Code, codeAddressReplacement, Header.ImageBase + bindRVA);

                    import.Write(bw);
                }
                IData = ms.ToArray();
            }
            var IDataSize = (uint)IData.Length;
            var IDataFileSize = AlignTo(IDataSize, Header.FileAlignment);
            var IDataVirtualSize = AlignTo(IDataSize, Header.SectionAlignment);
            {
                var idata = Header.IDataSection;
                idata.VirtualSize = IDataSize;
                idata.SizeOfRawData = IDataFileSize;
                idata.VirtualAddress = IDataVirtualAddress;
                idata.PointerToRawData = IDataFileOffset;
            }

            // data section
            var DataFileOffset = IDataFileOffset + IDataFileSize;
            var DataVirtualAddress = IDataVirtualAddress + IDataVirtualSize;
            using (var ms = new MemoryStream())
            {
                Data = ms.ToArray();
            }
            var DataSize = (uint)Data.Length;
            var DataFileSize = AlignTo(DataSize, Header.FileAlignment);
            var DataVirtualSize = AlignTo(DataSize, Header.SectionAlignment);
            Header.BaseOfData = DataVirtualAddress;
            {
                var data = Header.DataSection;
                data.VirtualSize = DataSize;
                data.SizeOfRawData = DataFileSize;
                data.VirtualAddress = DataVirtualAddress;
                data.PointerToRawData = DataFileOffset;
            }
            Header.SizeOfInitializedData = DataFileSize;

            // reloc section
            var RelocFileOffset = DataFileOffset + DataFileSize;
            var RelocVirtualAddress = DataVirtualAddress + DataVirtualSize;
            using (var ms = new MemoryStream())
            {
                using (var bw = new BinaryWriter(ms))
                {
                    var reloc = new BaseRelocationSection
                    {
                        Blocks = new BaseRelocationSection.Block[]
                        {
                            new BaseRelocationSection.Block {
                                PageRVA = CodeVirtualAddress,
                                Entries = new BaseRelocationSection.Entry[]
                                {
                                    new BaseRelocationSection.Entry
                                    {
                                        Offset = (ushort)codeAddressReplacement,
                                        Type = BaseRelocationSection.Types.DWord
                                    }
                                }
                            }
                        }
                    };

                    Header.DataDirectories[(uint)DataDirectoryIndex.BaseRelocationTable].VirtualAddress = RelocVirtualAddress;
                    Header.DataDirectories[(uint)DataDirectoryIndex.BaseRelocationTable].Size = reloc.WriteSize;

                    reloc.Write(bw);
                }
                Reloc = ms.ToArray();
            }
            var RelocSize = (uint)Reloc.Length;
            var RelocFileSize = AlignTo(RelocSize, Header.FileAlignment);
            var RelocVirtualSize = AlignTo(RelocSize, Header.SectionAlignment);

            // summary

            Header.SizeOfUninitializedData = 0;
 
            Header.SizeOfImage = RelocVirtualAddress + RelocVirtualSize;
            //Header.TimeDateStamp = TimeStamp();
        }

        private void InjectAddress(byte[] code, uint r, ulong address)
        {
            if (Header.Magic == MagicNumber.PE32)
            {
                code[r + 0] = (byte)(address >> 0);
                code[r + 1] = (byte)(address >> 8);
                code[r + 2] = (byte)(address >> 16);
                code[r + 3] = (byte)(address >> 24);
            }
            else
            {
                code[r + 0] = (byte)(address >> 0);
                code[r + 1] = (byte)(address >> 8);
                code[r + 2] = (byte)(address >> 16);
                code[r + 3] = (byte)(address >> 24);
                code[r + 4] = (byte)(address >> 32);
                code[r + 5] = (byte)(address >> 40);
                code[r + 6] = (byte)(address >> 48);
                code[r + 7] = (byte)(address >> 56);
            }
        }

        public void Write(BinaryWriter bw)
        {
            DosHeader.Write(bw);
        
            Header.Write(bw);
            WritePadding(bw, Header.FileAlignment);

            bw.Write(Code);
            WritePadding(bw, Header.FileAlignment);

            bw.Write(IData);
            WritePadding(bw, Header.FileAlignment);

            bw.Write(Data);
            WritePadding(bw, Header.FileAlignment);

            bw.Write(Reloc);
            WritePadding(bw, Header.FileAlignment);
        }

        private void WritePadding(BinaryWriter bw, uint alignment)
        {
            var pos = bw.BaseStream.Position;
            var data = new byte[Padding((uint)pos, alignment)];
            bw.Write(data);
        }
    }

    internal static class DosHeader
    {
        static public uint WriteSize = 128;
        static public void Write(BinaryWriter bw)
        {
            bw.Write(new byte[] {
                0x4D, 0x5A, //'MZ'
                0x90, 0, //Bytes on last page of file
                0x03, 0, //Pages in file
                0, 0, //Relocations

                0x04, 0, //Size of header in paragraphs
                0, 0, //Minimum extra paragraphs needed
                0xFF, 0xFF, //Maximum extra paragraphs needed

                0, 0, //Initial SS value
                0xB8, 0, //Initial SP value
                0, 0, //Checksum
                0, 0, //Initial IP value
                0, 0, //Initial CS value
                0x40, 0, //File Address of relocation table
                0, 0, //Overlay number

                0, 0, 0, 0, //[Reserved]
                0, 0, 0, 0, //[Reserved]

                0, 0, //OEM Identifier
                0, 0, //OEM Information

                0, 0, 0, 0, 0, //[Reserved]
                0, 0, 0, 0, 0, //[Reserved]
                0, 0, 0, 0, 0, //[Reserved]
                0, 0, 0, 0, 0, //[Reserved]

                0x80, 0, //File Address of PE header
                0, 0, //

                // Code
                0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd, 0x21, 0xb8, 0x01, 0x4c, 0xcd, 0x21, 0x54, 0x68,
                0x69, 0x73, 0x20, 0x70, 0x72, 0x6f, 0x67, 0x72, 0x61, 0x6d, 0x20, 0x63, 0x61, 0x6e, 0x6e, 0x6f,
                0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6e, 0x20, 0x69, 0x6e, 0x20, 0x44, 0x4f, 0x53, 0x20,
                0x6d, 0x6f, 0x64, 0x65, 0x2e, 0x0d, 0x0d, 0x0a, 0x24, 0, 0, 0, 0, 0, 0, 0
            });
        }
    }

    internal enum MachineTypes : ushort
    {
        Unknown = 0,
        AM33 = 0x1d3,
        AMD64 = 0x8664,
        ARM = 0x1c0, // ARM little endian
        ARM64 = 0xaa64, // ARM64 little endian
        ARMNT = 0x1c4, // ARM Thumb-2 little endian
        EBC = 0xebc, // EFI byte code
        I386 = 0x14c, // Intel 386 or later processors and compatible processors
        IA64 = 0x200, // Intel Itanium processor family
        M32R = 0x9041, // Mitsubishi M32R little endian
        MIPS16 = 0x266,
        MIPSFPU = 0x366, // MIPS with FPU
        MIPSFPU16 = 0x466,
        POWERPC = 0x1f0, // Power PC little endian
        POWERPCFP = 0x1f1, // Power PC with floating point support
        R4000 = 0x166, // MIPS little endian
        RISCV32 = 0x5032, // RISC-V 32-bit address space
        RISCV64 = 0x5064, // RISC-V 64-bit address space
        RISCV128 = 0x5128, // RISC-V 128-bit address space
        SH3 = 0x1a2, // Hitachi SH3
        SH3DSP = 0x1a3, // Hitachi SH3 DSP
        SH4 = 0x1a6,
        SH5 = 0x1a8,
        THUMB = 0x1c2,
        WCEMIPSV2 = 0x169, // MIPS little-endian WCE v2
    }

    [Flags]
    internal enum Characteristics : ushort
    {
        RELOCS_STRIPPED = 0x0001,
        EXECUTABLE_IMAGE = 0x0002,
        LINE_NUMS_STRIPPED = 0, // 0x0004 - deprecated & not used
        LOCAL_SYMS_STRIPPED = 0, // 0x0008 - deprecated & not used
        AGGRESSIVE_WS_TRIM = 0x0010, // valid only before Windows 2000
        LARGE_ADDRESS_AWARE = 0x0020, // >2GB addresses
        BYTES_REVERSED_LO = 0, // 0x0080 - deprecated & not used
        _32BIT_MACHINE = 0x0100,
        DEBUG_STRIPPED = 0x0200,
        REMOVABLE_RUN_FROM_SWAP = 0x0400, // copy to swap if launched from removable disk
        NET_RUN_FROM_SWAP = 0x0800, // copy to swap if launched from networek
        SYSTEM = 0x1000, // not a user program
        DLL = 0x2000,
        UP_SYSTEM_ONLY = 0x4000, // file should be run only on a uniprocessor machine.
        BYTES_REVERSED_HI = 0, // 0x8000 - deprecated & not used (Big endian)
    }

    internal enum MagicNumber : ushort
    {
        PE32 = 0x10b,
        PE32Plus = 0x20b, // 64 Bit address space (image <= 2GBR
    }

    internal enum WindowsSubsystem : ushort
    {
        UNKNOWN = 0,
        NATIVE = 1, // Device drivers and native Windows processes
        WINDOWS_GUI = 2, // The Windows graphical user interface (GUI) subsystem
        WINDOWS_CUI = 3, // The Windows character subsystem
        OS2_CUI = 5, // The OS/2 character subsystem
        POSIX_CUI = 7, // The Posix character subsystem
        NATIVE_WINDOWS = 8, // Native Win9x driver
        WINDOWS_CE_GUI = 9,
        EFI_APPLICATION = 10,
        EFI_BOOT_SERVICE_DRIVER = 11,
        EFI_RUNTIME_DRIVER = 12,
        EFI_ROM = 13,
        XBOX = 14,
        WINDOWS_BOOT_APPLICATION = 16,
    }

    [Flags]
    internal enum DllCharacteristics : ushort
    {
        None = 0,
        HIGH_ENTROPY_VA = 0x0020, // Image can handle a high entropy 64-bit virtual address space.
        DYNAMIC_BASE = 0x0040, // DLL can be relocated at load time.
        FORCE_INTEGRITY = 0x0080, // Code Integrity checks are enforced.
        NX_COMPAT = 0x0100, // No Execution Flag Compatible
        NO_ISOLATION = 0x0200, // Isolation aware, but do not isolate the image.
        NO_SEH = 0x0400, // Does not use structured exception (SE) handling. No SE handler may be called in this image.
        NO_BIND = 0x0800, // Do not bind the image.
        APPCONTAINER = 0x1000, // Image must execute in an AppContainer.
        WDM_DRIVER = 0x2000,
        GUARD_CF = 0x4000, // Image supports Control Flow Guard.
        TERMINAL_SERVER_AWARE = 0x8000,
    }

    internal enum DataDirectoryIndex
    {
        ExportTable = 0,
        ImportTable = 1,
        ResourceTable = 2,
        ExceptionTable = 3,
        CertificateTable = 4,
        BaseRelocationTable = 5,
        Debug = 6,
        Architecture = 7, // unsused
        GlobalPtr = 8, // no size!
        TlsTable = 9,
        LoadConfigTable = 10,
        BoundImport = 11,
        ImportBindTable = 12, // IAT
        DelayImportDescriptor = 13,
        ClrRuntimeHeader = 14,
        // 15 for proper alignment
    }

    internal class ImageHeader
    {
        public byte[] Signature => new byte[4] { (byte)'P', (byte)'E', 0, 0 };
        // COFF Header
        public MachineTypes Machine = MachineTypes.I386;
        public ushort NumberOfSections => (ushort)Sections.Count;
        public uint TimeDateStamp = 0;
        public uint PointerToSymbolTable => 0; // deprecated
        public uint NumberOfSymbols => 0; // deprecated
        public ushort SizeOfOptionalHeader => (ushort)((Magic == MagicNumber.PE32 ? 28 + 68 : 24 + 88) + (DataDirectories.Length * DataDirectory.WriteSize));
        public Characteristics Characteristics =
            Characteristics.RELOCS_STRIPPED |
            Characteristics.EXECUTABLE_IMAGE |
            Characteristics._32BIT_MACHINE |
            Characteristics.DEBUG_STRIPPED;

        // Optional Header - Standard Fields
        public MagicNumber Magic = MagicNumber.PE32;
        public byte MajorLinkerVersion = 0x01;
        public byte MinorLinkerVersion = 0x00;
        public uint SizeOfCode;
        public uint SizeOfInitializedData;
        public uint SizeOfUninitializedData = 0x00;
        public uint AddressOfEntryPoint = 0x1000;
        public uint BaseOfCode = 0x1000;
        public uint BaseOfData = 0x2000; // Only use in PE32 Mode

        // Optional Header - Windows Specific Fields
        public ulong ImageBase = 0x400_000;
        public uint SectionAlignment = 4096;
        public uint FileAlignment = 512;

        public ushort MajorOperatingSystemVersion = 4;
        public ushort MinorOperatingSystemVersion = 0;
        public ushort MajorImageVersion = 0;
        public ushort MinorImageVersion = 0;
        public ushort MajorSubsystemVersion = 4;
        public ushort MinorSubsystemVersion = 0;
        public uint Win32VersionValue => 0;
        public uint SizeOfImage; // The size (in bytes) of the image, including all headers, as the image is loaded in memory. It must be a multiple of SectionAlignment.
        public uint SizeOfHeaders = 512; // The combined size of an MS DOS stub, PE header, and section headers rounded up to a multiple of FileAlignment.
        public uint CheckSum = 0;

        public WindowsSubsystem Subsystem = WindowsSubsystem.WINDOWS_CUI;
        public DllCharacteristics DllCharacteristics = DllCharacteristics.NX_COMPAT | DllCharacteristics.DYNAMIC_BASE | DllCharacteristics.HIGH_ENTROPY_VA;
        public ulong SizeOfStackReserve = 0x10_0000; // The size of the stack to reserve. Only SizeOfStackCommit is committed; the rest is made available one page at a time until the reserve size is reached.
        public ulong SizeOfStackCommit = 0x1000;
        public ulong SizeOfHeapReserve = 0x10_0000; // The size of the local heap space to reserve. Only SizeOfHeapCommit is committed; the rest is made available one page at a time until the reserve size is reached
        public ulong SizeOfHeapCommit = 0x1000;
        public uint LoaderFlags => 0;
        public uint NumberOfRvaAndSizes => (uint)DataDirectories.Length;

        public DataDirectory[] DataDirectories { get; } = new DataDirectory[16];

        public IList<Section> Sections { get; } = new List<Section>();

        static readonly byte[] TEXT_SECTION_NAME = Encoding.ASCII.GetBytes(".text");
        public Section TextSection => Sections.Single(s => s.Name == TEXT_SECTION_NAME);

        static readonly byte[] IDATA_SECTION_NAME = Encoding.ASCII.GetBytes(".idata");
        public Section IDataSection => Sections.Single(s => s.Name == IDATA_SECTION_NAME);

        static readonly byte[] DATA_SECTION_NAME = Encoding.ASCII.GetBytes(".data");
        public Section DataSection => Sections.Single(s => s.Name == DATA_SECTION_NAME);

        static readonly byte[] RELOC_SECTION_NAME = Encoding.ASCII.GetBytes(".reloc");
        public Section RelocSection => Sections.Single(s => s.Name == RELOC_SECTION_NAME);

        static readonly Dictionary<byte[], SectionFlags> DefaultSectionFlags = new Dictionary<byte[], SectionFlags>
        {
            { TEXT_SECTION_NAME, SectionFlags.MEM_READ | SectionFlags.MEM_EXECUTE | SectionFlags.CNT_CODE },
            { IDATA_SECTION_NAME, SectionFlags.MEM_READ },
            { DATA_SECTION_NAME, SectionFlags.MEM_WRITE | SectionFlags.MEM_READ | SectionFlags.CNT_INITIALIZED_DATA },
            { RELOC_SECTION_NAME, SectionFlags.MEM_DISCARDABLE }
        };

        public uint WriteSize => 4u // signature
                    + 18u // COFF File Header
                    + SizeOfOptionalHeader
                    + (uint)(Sections.Count * Section.WriteSize);

        public static ImageHeader WithDefaultSections()
        {
            var ih = new ImageHeader();
            var sections = new byte[][] { TEXT_SECTION_NAME, IDATA_SECTION_NAME, DATA_SECTION_NAME, RELOC_SECTION_NAME };
            var i = 1u;
            foreach(var s in sections)
            {
                ih.Sections.Add(new Section
                {
                    Name = s,
                    VirtualSize = ih.SectionAlignment,
                    SizeOfRawData = ih.FileAlignment,
                    Characteristics = DefaultSectionFlags.Fetch(s, (SectionFlags)0),
                    PointerToRawData = i * ih.FileAlignment,
                    VirtualAddress = i * ih.SectionAlignment
                });
                i++;
            }
            return ih;
        }

        public void Write(BinaryWriter bw)
        {
            // COFF Header
            bw.Write(Signature);
            bw.Write((ushort)Machine);
            bw.Write(NumberOfSections);
            bw.Write(TimeDateStamp);
            bw.Write(PointerToSymbolTable);
            bw.Write(NumberOfSymbols);
            bw.Write(SizeOfOptionalHeader);
            bw.Write((ushort)Characteristics);

            // Standard COFF Fields
            bw.Write((ushort)Magic);
            bw.Write(new[] { MajorLinkerVersion, MinorLinkerVersion });
            bw.Write(SizeOfCode);
            bw.Write(SizeOfInitializedData);
            bw.Write(SizeOfUninitializedData);
            bw.Write(AddressOfEntryPoint);
            bw.Write(BaseOfCode);
            if (Magic == MagicNumber.PE32) bw.Write(BaseOfData);

            void WriteSizeType(ulong ptr) {
                if (Magic == MagicNumber.PE32)
                    bw.Write((uint)ptr);
                else
                    bw.Write(ptr);
            }

            // Windows Specific Fields
            WriteSizeType(ImageBase);
            bw.Write(SectionAlignment);
            bw.Write(FileAlignment);
            bw.Write(MajorOperatingSystemVersion);
            bw.Write(MinorOperatingSystemVersion);
            bw.Write(MajorImageVersion);
            bw.Write(MinorImageVersion);
            bw.Write(MajorSubsystemVersion);
            bw.Write(MinorSubsystemVersion);
            bw.Write(Win32VersionValue);
            bw.Write(SizeOfImage);
            bw.Write(SizeOfHeaders);
            bw.Write(CheckSum);
            bw.Write((ushort)Subsystem);
            bw.Write((ushort)DllCharacteristics);
            WriteSizeType(SizeOfStackReserve);
            WriteSizeType(SizeOfStackCommit);
            WriteSizeType(SizeOfHeapReserve);
            WriteSizeType(SizeOfHeapCommit);
            bw.Write(LoaderFlags);
            bw.Write(NumberOfRvaAndSizes);
            foreach (var d in DataDirectories) d.Write(bw);

            foreach (var s in Sections) s.Write(bw);
        }

        internal struct DataDirectory
        {
            public uint VirtualAddress;
            public uint Size;

            public static uint WriteSize = 8;

            public void Write(BinaryWriter bw)
            {
                bw.Write(VirtualAddress);
                bw.Write(Size);
            }
        }

        [Flags]
        internal enum SectionFlags : uint
        {
            CNT_CODE = 0x0000_0020,
            CNT_INITIALIZED_DATA = 0x0000_0040,
            CNT_UNINITIALIZED_DATA = 0x0000_0080,
            GPREL = 0x0000_8000, // (IA64 only) The section contains data referenced through the global pointer (GP).
            LNK_NRELOC_OVFL = 0x0100_0000, // The section contains extended relocations.
            MEM_DISCARDABLE = 0x0200_0000, // The section can be discarded as needed.
            MEM_NOT_CACHED = 0x0400_0000, // The section cannot be cached.
            MEM_NOT_PAGED = 0x0800_0000, // The section is not pageable.
            MEM_SHARED = 0x1000_0000, // The section can be shared in memory.
            MEM_EXECUTE = 0x2000_0000,
            MEM_READ = 0x4000_0000,
            MEM_WRITE = 0x8000_0000
        }

        internal class Section
        {
            public byte[] Name; // maximum of 8 characters
            public uint VirtualSize = 0; // The total size of the section when loaded into memory. If this value is greater than SizeOfRawData, the section is zero-padded.
            public uint VirtualAddress = 0; // the address of the first byte before relocation is applied; for simplicity, compilers should set this to zero. Otherwise, it is an arbitrary value that is subtracted from offsets during relocation.
            public uint SizeOfRawData = 0; // size of the initialized data on disk. this must be a multiple of FileAlignment from the optional header. If this is less than VirtualSize, the remainder of the section is zero-filled. 
            public uint PointerToRawData = 0; // For executable images, this must be a multiple of FileAlignment from the optional header. . When a section contains only uninitialized data, this field should be zero.
            public uint PointerToRelocations = 0; // The file pointer to the beginning of relocation entries for the section. This is set to zero for executable images or if there are no relocations.
            public uint PointerToLinenumbers = 0; // This is set to zero if there are no COFF line numbers. 
            public ushort NumberOfRelocations = 0;
            public ushort NumberOfLinenumbers = 0;
            public SectionFlags Characteristics;

            public static uint WriteSize = 40;

            public void Write(BinaryWriter bw)
            {
                bw.Write(Name.Concat(new byte[8]).Take(8).ToArray());
                bw.Write(VirtualSize);
                bw.Write(VirtualAddress);
                bw.Write(SizeOfRawData);
                bw.Write(PointerToRawData);
                bw.Write(PointerToRelocations);
                bw.Write(PointerToLinenumbers);
                bw.Write(NumberOfRelocations);
                bw.Write(NumberOfLinenumbers);
                bw.Write((uint)Characteristics);
            }
        }
    }

    internal class ImportSection
    {
        class DirectoryTable
        {
            public uint LookupTableRVA;
            public uint TimeStamp = 0;
            public uint ForwarderChain = 0;
            public uint NameRVA; // name of DLL
            public uint BindTableRVA; // 

            public static uint WriteSize = 20;

            public void Write(BinaryWriter bw)
            {
                bw.Write(LookupTableRVA);
                bw.Write(TimeStamp);
                bw.Write(ForwarderChain);
                bw.Write(NameRVA);
                bw.Write(BindTableRVA);
            }

            public static void WriteTerminator(BinaryWriter bw)
            {
                bw.Write(new byte[20]);
            }
        }
        class LookupEntry
        {
            public bool IsOrdinal;
            public ushort OrdinalNumber;
            public uint NameTableRVA;

            public static uint WriteSize(MagicNumber magic) => magic == MagicNumber.PE32 ? 4u : 8u;

            public void Write(BinaryWriter bw, MagicNumber magic)
            {
                if (magic == MagicNumber.PE32)
                {
                    if (IsOrdinal)
                        bw.Write(0x80000000u + OrdinalNumber);
                    else
                        bw.Write(NameTableRVA);
                }
                else
                {
                    if (IsOrdinal)
                        bw.Write(0x8000000000000000u + OrdinalNumber);
                    else
                        bw.Write((ulong)NameTableRVA);
                }
            }
            public static void WriteTerminator(BinaryWriter bw, MagicNumber magic)
            {
                if (magic == MagicNumber.PE32)
                    bw.Write((uint)0);
                else
                    bw.Write((ulong)0);
            }
        }

        static uint StringWriteSize(string str)
        {
            var bytes = Encoding.ASCII.GetBytes(str);
            return (uint)(bytes.Length + (bytes.Length % 2 == 0 ? 2 : 1));
        }
        static void StringWrite(BinaryWriter bw, string str)
        {
            var bytes = Encoding.ASCII.GetBytes(str);
            bw.Write(bytes);
            if (bytes.Length % 2 == 0) bw.Write((ushort)0); else bw.Write((byte)0);
        }

        public class HintNameEntry
        {
            public ushort Hint;
            public string Name;

            public uint WriteSize()
            {
                var bytes = Encoding.ASCII.GetBytes(Name);
                return (uint)(2 + StringWriteSize(Name));
            }

            public void Write(BinaryWriter bw)
            {
                bw.Write(Hint);
                StringWrite(bw, Name);
            }
        }
        public class DLL
        {
            public string Name;
            public HintNameEntry[] Functions;
        }

        MagicNumber magic;
        DirectoryTable[] tables;
        LookupEntry[][] lookupEntries;
        HintNameEntry[] hintNames;
        string[] dllNames;

        public ImportSection(DLL[] imports, MagicNumber _magic, uint rva)
        {
            magic = _magic;
            tables = new DirectoryTable[imports.Length];
            lookupEntries = new LookupEntry[imports.Length][];
            dllNames = new string[imports.Length];
            var _hintNames = new List<HintNameEntry>();
            // Import Lookup Tables
            var firstLookupRVA = rva + (uint)(tables.Length + 1) * DirectoryTable.WriteSize;
            var lookupRVA = firstLookupRVA;
            var i = 0;
            foreach (var import in imports)
            {
                tables[i] = new DirectoryTable();
                lookupEntries[i] = new LookupEntry[import.Functions.Length];
                dllNames[i] = import.Name;
                tables[i].LookupTableRVA = lookupRVA;
                lookupRVA += (uint)(import.Functions.Length + 1) * LookupEntry.WriteSize(_magic);
                i++;
            }
            var lookupSize = lookupRVA - firstLookupRVA;
            ImportTableRVA = rva;
            ImportTableSize = firstLookupRVA - rva;
            // bindTables
            BindTableRVA = lookupRVA;
            BindTableSize = lookupSize;
            foreach (var table in tables)
            {
                table.BindTableRVA = table.LookupTableRVA + (lookupRVA - firstLookupRVA);
            }
            var hintRVA = lookupRVA + lookupSize;
            i = 0;
            foreach (var import in imports)
            {
                var j = 0;
                foreach (var entry in import.Functions)
                {
                    lookupEntries[i][j] = new LookupEntry();
                    var hasName = (entry.Name.Length != 0);
                    lookupEntries[i][j].IsOrdinal = !hasName;
                    if (hasName)
                    {
                        _hintNames.Add(entry);
                        lookupEntries[i][j].NameTableRVA = hintRVA;
                        hintRVA += entry.WriteSize();
                    }
                    else
                    {
                        lookupEntries[i][j].OrdinalNumber = entry.Hint;
                    }
                    j++;
                }
                i++;
            }

            // dllNames
            var namesRVA = hintRVA;
            i = 0;
            foreach (var import in imports)
            {
                tables[i].NameRVA = namesRVA;
                namesRVA += StringWriteSize(import.Name);
                i++;
            }
            WriteSize = namesRVA;
            hintNames = _hintNames.ToArray();
        }

        public uint WriteSize { get; internal set; }
        public uint ImportTableRVA { get; internal set; }
        public uint ImportTableSize { get; internal set; }
        public uint BindTableRVA { get; internal set; }
        public uint BindTableSize { get; internal set; }

        public uint FunctionBindRVA(uint dll, uint func)
        {
            return tables[dll].BindTableRVA + LookupEntry.WriteSize(magic) * func;
        }

        public void Write(BinaryWriter bw)
        {
            foreach (var table in tables) table.Write(bw);
            DirectoryTable.WriteTerminator(bw);

            foreach (var lookups in lookupEntries) // lookup Tables
            {
                foreach (var lookup in lookups) lookup.Write(bw, magic);
                LookupEntry.WriteTerminator(bw, magic);
            }
            foreach (var lookups in lookupEntries) // bindTables
            {
                foreach (var lookup in lookups) lookup.Write(bw, magic);
                LookupEntry.WriteTerminator(bw, magic);
            }
            foreach (var hint in hintNames) hint.Write(bw);

            foreach (var name in dllNames)
            {
                StringWrite(bw, name);
            }
        }
    }

    internal class BaseRelocationSection
    {
        public struct Entry
        {
            public Types Type;
            public ushort Offset;

            public static uint WriteSize => 2;

            public void Write(BinaryWriter bw)
            {
                var word = (ushort)(((ushort)Type&0xF)*0x1000 + (Offset&0x0FFF));
                bw.Write(word);
            }
        }
        public struct Block
        {
            public uint PageRVA;
            public uint Size => (uint)(8 + Entry.WriteSize*(Entries.Length + Entries.Length%2)); // total size of block %4 == 0
            public Entry[] Entries;

            public void Write(BinaryWriter bw)
            {
                bw.Write(PageRVA);
                bw.Write(Size);
                foreach (var e in Entries) e.Write(bw);
                if (0 != Entries.Length % 2) bw.Write((ushort)0);
            }
        }
        public enum Types
        {
            None = 0, // skipped - use for padding the block
            HighWord = 1, // 16 higher bits of difference are added
            LowWord = 2, // 16 lower bits of difference are added
            DWord = 3, // 32bits of difference are added
            HighAdjusted = 4, // what?
            MIPS_JMPADDR = 5,
            ARM_MOV32 = 5, // MOVW/WOVT instructions of ARM/THUMB
            RISCV_HIGH20 = 5,
            THUMB_MOV32 = 7,
            RISCV_LOW12I = 7,
            RISCV_LOW12S = 8,
            MIPS_JMPADDR16 = 9,
            QWord = 10, // 32bits of difference are added to 64 bit address
        }

        public Block[] Blocks;

        public uint WriteSize => (uint)Blocks.Sum(b => b.Size);

        public void Write(BinaryWriter bw)
        {
            foreach (var block in Blocks) block.Write(bw);
        }
    }

    internal class LoadConfigurationLayout
    {
        public uint Characteristics => 0;
        public uint TimeStamp;
        public ushort MajorVersion;
        public ushort MinorVersion;
        public uint GlobalFlagsClear; // The global loader flags to clear for this process as the loader starts the process.
        public uint GlobalFlagsSet; // The global loader flags to set for this process as the loader starts the process.
        public uint CriticalSectionDefaultTimeout; // The default timeout value to use for this process’s critical sections that are abandoned.
        public ulong DeCommitFreeBlockThreshold; // Memory that must be freed before it is returned to the system, in bytes.
        public ulong DeCommitTotalFreeThreshold; // Total amount of free memory, in bytes.
        public ulong LockPrefixTable; // [x86 only] The VA of a list of addresses where the LOCK prefix is used so that they can be replaced with NOP on single processor machines.
        public ulong MaximumAllocationSize; // Maximum allocation size, in bytes.
        public ulong VirtualMemoryThreshold; // Maximum virtual memory size, in bytes.
        public ulong ProcessAffinityMask; // Setting this field to a non-zero value is equivalent to calling SetProcessAffinityMask with this value during process startup (.exe only)
        public uint ProcessHeapFlags; // Process heap flags that correspond to the first argument of the HeapCreate function. These flags apply to the process heap that is created during process startup.
        public ushort CSDVersion;
        public ushort Reserved => 0;
        public ulong EditList => 0;
        public ulong SecurityCookie; // A pointer to a cookie that is used by Visual C++ or GS implementation.
        public ulong SEHandlerTable; //[x86 only] The VA of the sorted table of RVAs of each valid, unique SE handler in the image.
        public ulong SEHandlerCount; //[x86 only] The count of unique handlers in the table.
        public ulong GuardCFCheckFunctionPointer; // The VA where Control Flow Guard check-function pointer is stored.
        public ulong GuardCFDispatchFunctionPointer; // The VA where Control Flow Guard  dispatch-function pointer is stored.
        public ulong GuardCFFunctionTable; // The VA of the sorted table of RVAs of each Control Flow Guard function in the image.
        public ulong GuardCFFunctionCount; // The count of unique RVAs in the above table.
        public uint GuardFlags;
        private readonly MagicNumber magic;

        LoadConfigurationLayout(MagicNumber _magic)
        {
            magic = _magic;
        }

        public uint WriteSize => magic == MagicNumber.PE32 ? 92u : 148u;

        public void Write(BinaryWriter bw)
        {
            void WriteSizeT(ulong d)
            {
                if (magic == MagicNumber.PE32)
                    bw.Write((uint)d);
                else
                    bw.Write(d);
            }

            bw.Write(Characteristics);
            bw.Write(TimeStamp);
            bw.Write(MajorVersion);
            bw.Write(MinorVersion);
            bw.Write(GlobalFlagsClear);
            bw.Write(GlobalFlagsSet);
            bw.Write(CriticalSectionDefaultTimeout);
            WriteSizeT(DeCommitFreeBlockThreshold);
            WriteSizeT(DeCommitTotalFreeThreshold);
            WriteSizeT(LockPrefixTable);
            WriteSizeT(MaximumAllocationSize);
            WriteSizeT(VirtualMemoryThreshold);
            WriteSizeT(ProcessAffinityMask);
            bw.Write(ProcessHeapFlags);
            bw.Write(CSDVersion);
            bw.Write(Reserved);
            WriteSizeT(EditList);
            WriteSizeT(SecurityCookie);
            WriteSizeT(SEHandlerTable);
            WriteSizeT(SEHandlerCount);
            WriteSizeT(GuardCFCheckFunctionPointer);
            WriteSizeT(GuardCFDispatchFunctionPointer);
            WriteSizeT(GuardCFFunctionTable);
            WriteSizeT(GuardCFFunctionCount);
            bw.Write(GuardFlags);
        }
    }

    internal class ResourceSection
    {
        // 3 Nested Tables: Type, Name, Language
        public DirectoryTable[] DirectoryTables;
        public string[] DirectoryStrings;
        public DataDescription[] DataDescriptions;
        public byte[][] Data;

        internal class DirectoryTable
        {
            public uint Characteristics => 0;
            public uint TimeStamp;
            public ushort MajorVersion = 0;
            public ushort MinorVersion = 0;
            public ushort NumberOfNameEntries => (ushort)NameEntries.Length;
            public ushort NumberOfIdEntries => (ushort)NumberEntries.Length;

            internal class Entry
            {
                public bool NameOrNumber;
                public uint NameIndex;
                public uint Number;

                public bool IsLeaf;
                public uint dataIndex;
                public uint subTableIndex;

                public static uint WriteSize => 8;

            }
            public Entry[] NameEntries;
            public Entry[] NumberEntries;
        }

        internal class DataDescription
        {
            public uint DataRVA;
            public uint Size;
            public uint CodePage;
            public uint Reserved => 0;

            public static uint WriteSize => 16;
        }
    }

    internal class DelayLoadDirectoryTable
    {
        public uint Attributes => 0;
        public uint NameRVA; // name of the DLL to be loaded. The name resides in the read-only data section of the image.
        public uint ModuleHandleRVA; // in the data section of the image. It is used for storage by the routine that is supplied to manage delay-loading.
        public uint DelayImportAddressTableRVA;
        public uint DelayImportNameTableRVA;
        public uint BoundDelayImportTableRVA;
        public uint UnloadDelayImportTableRVA;
        public uint TimeStamp; // timestamp of the DLL to which this image has been bound.
    }

    internal class ExportDirectoryTable
    {
        public uint ExportFlags => 0;
        public uint TimeStamp; // timestamp the export data was created
        public ushort MajorVersion = 0;
        public ushort MinorVersion = 0;
        public uint NameRVA; // address of the ASCII string that contains the name of the DLL. This address is relative to the image base.
        public uint OrdinalBase; // starting ordinal number for exports in this image.This field specifies the starting ordinal number for the export address table.It is usually set to 1.
        public uint AddressTableEntries; // number of entries in the export address table.
        public uint NumberOfNamePointers; // number of entries in the name pointer table.This is also the number of entries in the ordinal table.
        public uint ExportAddressTableRVA; // address of the export address table, relative to the image base.
        public uint NamePointerRVA; // address of the export name pointer table, relative to the image base. The table size is given by the Number of Name Pointers field.
        public uint OrdinalTableRVA; // address of the ordinal table, relative to the image base.
    }

    enum DebugType : uint
    {
        UNKNOWN = 0, COFF, CODEVIEW, FPO, MISC,
        EXCEPTION = 5, FIXUP, OMAP_TO_SRC, OMAP_FROM_SRC, BORLAND,
        CLSID = 11,
        REPRO = 16,
    }

    internal class DebugDirectory
    {
        public uint Characteristics => 0;
        public uint TimeStamp;
        public ushort MajorVersion;
        public ushort MinorVersion;
        public DebugType Type;
        public uint SizeOfData;
        public uint AddressOfRawData; // Relative to image base
        public uint PointerToRawData; // file pointer to debug data

        public static uint WriteSize = 28;
    }
}