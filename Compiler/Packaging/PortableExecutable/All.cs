using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Text;
using System;
using REC.Tools;
using REC.Packaging.Tools;

namespace REC.Packaging.PortableExecutable
{
    internal class Image
    {
        ImageHeader Header { get; } = ImageHeader.WithDefaultSections();
        byte[] Code;
        byte[] IData;
        byte[] Data;
        byte[] RSrc;
        byte[] Reloc;

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

            // *** fill contents ***
            var CodeSize = (uint)Code.Length;
            var CodeFileSize = CodeSize.AlignTo(Header.FileAlignment);
            var CodeVirtualSize = CodeSize.AlignTo(Header.SectionAlignment);
            Header.TextSection.VirtualSize = CodeSize;
            Header.TextSection.SizeOfRawData = CodeFileSize;

            // TODO: partial layout of Import Data + fixup with address
            Header.IDataSection.VirtualSize = 1; // marker that this section is present

            using (var ms = new MemoryStream())
            {
                Data = ms.ToArray();
            }
            var DataSize = (uint)Data.Length;
            var DataFileSize = DataSize.AlignTo(Header.FileAlignment);
            var DataVirtualSize = DataSize.AlignTo(Header.SectionAlignment);
            Header.DataSection.VirtualSize = DataSize;
            Header.DataSection.SizeOfRawData = DataFileSize;

            // TODO: partial layout for resources
            Header.RSrcSection.VirtualSize = 1; // marker that this section is present

            // TODO: partial layout for base relocations
            Header.RelocSection.VirtualSize = 1; // marker that this section is present

            // *** do layout ***

            // headers
            var HeaderFileSize = Header.SizeOfHeaders;
            var HeaderVirtualSize = HeaderFileSize.AlignTo(Header.SectionAlignment);

            // code
            var CodeFileOffset = HeaderFileSize;
            var CodeVirtualAddress = HeaderVirtualSize;
            Header.BaseOfCode = CodeVirtualAddress;
            {
                var text = Header.TextSection;
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

                    Header.DataDirectories[(uint)DataDirectoryIndex.ImportAddressTable].VirtualAddress = import.ImportAddressTableRVA;
                    Header.DataDirectories[(uint)DataDirectoryIndex.ImportAddressTable].Size = import.ImportAddressTableSize;

                    var bindRVA = import.FunctionBindRVA(0, 0);
                    InjectAddress(Code, codeAddressReplacement, Header.ImageBase + bindRVA);

                    import.Write(bw);
                }
                IData = ms.ToArray();
            }
            var IDataSize = (uint)IData.Length;
            var IDataFileSize = IDataSize.AlignTo(Header.FileAlignment);
            var IDataVirtualSize = IDataSize.AlignTo(Header.SectionAlignment);
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
            Header.BaseOfData = DataVirtualAddress;
            Header.DataSection.VirtualAddress = DataVirtualAddress;
            Header.DataSection.PointerToRawData = DataFileOffset;

            // resource section
            var RSrcFileOffset = DataFileOffset + DataFileSize;
            var RSrcVirtualAddress = DataVirtualAddress + DataVirtualSize;
            using (var ms = new MemoryStream())
            {
                using (var bw = new BinaryWriter(ms))
                {
                    var resources = new Resources();
                    resources.AddIcon(new Resources.IconParameters
                    {
                        Name = "DESK1",
                        Stream = new FileStream("R:\\main.ico", FileMode.Open)
                    });
                    resources.AddVersionInfo(new Resources.VersionParameters
                    {
                        FixedData = {
                            FileVersion = (1,2,3,4),
                            ProductVersion = (5,6,7,8),
                        },
                        StringTables = {
                            {
                                new Resources.VersionParameters.LanguageCodePage {
                                    CodePage = Resources.CodePages.Unicode,
                                    Language = Resources.Languages.UsEnglish
                                },
                                new Dictionary<Resources.VersionKeys, string> {
                                    { Resources.VersionKeys.ProductName, "Rebuild Test Executable" },
                                    { Resources.VersionKeys.ProductVersion, "Awesome" }
                                }
                            }
                        }
                    });
                    resources.AddManifest(new Resources.ManifestParameters
                    {
                        Stream = new FileStream("R:\\main.manifest", FileMode.Open)
                    });

                    var rsrc = new ResourceSection(resources);

                    rsrc.Write(bw, RSrcVirtualAddress);
                }
                RSrc = ms.ToArray();
            }
            var RSrcSize = (uint)RSrc.Length;
            var RSrcFileSize = RSrcSize.AlignTo(Header.FileAlignment);
            var RSrcVirtualSize = RSrcSize.AlignTo(Header.SectionAlignment);
            {
                var text = Header.RSrcSection;
                text.VirtualSize = RSrcSize;
                text.SizeOfRawData = RSrcFileSize;
                text.VirtualAddress = RSrcVirtualAddress;
                text.PointerToRawData = RSrcFileOffset;
            }
            Header.DataDirectories[(uint)DataDirectoryIndex.ResourceTable].VirtualAddress = RSrcVirtualAddress;
            Header.DataDirectories[(uint)DataDirectoryIndex.ResourceTable].Size = RSrcSize;

            // reloc section
            var RelocFileOffset = RSrcFileOffset + RSrcFileSize;
            var RelocVirtualAddress = RSrcVirtualAddress + RSrcVirtualSize;
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


                    reloc.Write(bw);
                }
                Reloc = ms.ToArray();
            }
            var RelocSize = (uint)Reloc.Length;
            var RelocFileSize = RelocSize.AlignTo(Header.FileAlignment);
            var RelocVirtualSize = RelocSize.AlignTo(Header.SectionAlignment);
            {
                var text = Header.RelocSection;
                text.VirtualSize = RelocSize;
                text.SizeOfRawData = RelocFileSize;
                text.VirtualAddress = RelocVirtualAddress;
                text.PointerToRawData = RelocFileOffset;
            }
            Header.DataDirectories[(uint)DataDirectoryIndex.BaseRelocationTable].VirtualAddress = RelocVirtualAddress;
            Header.DataDirectories[(uint)DataDirectoryIndex.BaseRelocationTable].Size = RelocSize;

            // summary

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
        
            Header.Write(bw); bw.PadPosition(Header.FileAlignment);

            if (Code != null && Code.Length != 0)
            {
                bw.Write(Code); bw.PadPosition(Header.FileAlignment);
            }
            if (IData != null && IData.Length != 0)
            {
                bw.Write(IData); bw.PadPosition(Header.FileAlignment);
            }
            if (Data != null && Data.Length != 0)
            {
                bw.Write(Data); bw.PadPosition(Header.FileAlignment);
            }
            if (RSrc != null && RSrc.Length != 0)
            {
                bw.Write(RSrc); bw.PadPosition(Header.FileAlignment);
            }
            if (Reloc != null && Reloc.Length != 0)
            {
                bw.Write(Reloc); bw.PadPosition(Header.FileAlignment);
            }
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
        ImportAddressTable = 12, // IAT
        DelayImportDescriptor = 13,
        ClrRuntimeHeader = 14,
        // 15 for proper alignment
    }

    internal class ImageHeader
    {
        public byte[] Signature => new byte[4] { (byte)'P', (byte)'E', 0, 0 };
        // COFF Header
        public MachineTypes Machine = MachineTypes.I386;
        public ushort NumberOfSections => (ushort)Sections.Count(s => s.VirtualSize != 0);
        public uint TimeDateStamp = 0;
        public uint PointerToSymbolTable => 0; // deprecated
        public uint NumberOfSymbols => 0; // deprecated
        public ushort SizeOfOptionalHeader => (ushort)((Magic == MagicNumber.PE32 ? 28 + 68 : 24 + 88) + (DataDirectories.Length * DataDirectory.WriteSize));
        public Characteristics Characteristics =
            //Characteristics.RELOCS_STRIPPED |
            Characteristics.EXECUTABLE_IMAGE |
            Characteristics._32BIT_MACHINE |
            Characteristics.DEBUG_STRIPPED;

        // Optional Header - Standard Fields
        public MagicNumber Magic = MagicNumber.PE32;
        public byte MajorLinkerVersion = 14;
        public byte MinorLinkerVersion = 10;
        public uint SizeOfCode => (uint)Sections.Where(s => s.VirtualSize != 0 && s.Characteristics.HasFlag(SectionFlags.CNT_CODE)).Sum(s => s.SizeOfRawData);
        public uint SizeOfInitializedData => (uint)Sections.Where(s => s.VirtualSize != 0 && s.Characteristics.HasFlag(SectionFlags.CNT_INITIALIZED_DATA)).Sum(s => s.SizeOfRawData);
        public uint SizeOfUninitializedData => (uint)Sections.Where(s => s.VirtualSize != 0 && s.Characteristics.HasFlag(SectionFlags.CNT_UNINITIALIZED_DATA)).Sum(s => s.SizeOfRawData);
        public uint AddressOfEntryPoint = 0x1000;
        public uint BaseOfCode;
        public uint BaseOfData; // Only use in PE32 Mode

        // Optional Header - Windows Specific Fields
        public ulong ImageBase = 0x40_0000;
        public uint SectionAlignment = 0x1000; // 4096
        public uint FileAlignment = 512;

        public ushort MajorOperatingSystemVersion = 4;
        public ushort MinorOperatingSystemVersion = 0;
        public ushort MajorImageVersion = 0;
        public ushort MinorImageVersion = 0;
        public ushort MajorSubsystemVersion = 4;
        public ushort MinorSubsystemVersion = 0;
        public uint Win32VersionValue => 0;
        public uint SizeOfImage => SizeOfHeaders.AlignTo(SectionAlignment) + (uint)Sections.Sum(s => s.VirtualSize.AlignTo(SectionAlignment)); // The size (in bytes) of the image, including all headers, as the image is loaded in memory. It must be a multiple of SectionAlignment.
        public uint SizeOfHeaders => (DosHeader.WriteSize + WriteSize).AlignTo(FileAlignment); // The combined size of an MS DOS stub, PE header, and section headers rounded up to a multiple of FileAlignment.
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

        static readonly byte[] RSRC_SECTION_NAME = Encoding.ASCII.GetBytes(".rsrc");
        public Section RSrcSection => Sections.Single(s => s.Name == RSRC_SECTION_NAME);

        static readonly Dictionary<byte[], SectionFlags> DefaultSectionFlags = new Dictionary<byte[], SectionFlags>
        {
            { TEXT_SECTION_NAME, SectionFlags.CNT_CODE | SectionFlags.MEM_READ | SectionFlags.MEM_EXECUTE },
            { IDATA_SECTION_NAME, SectionFlags.CNT_INITIALIZED_DATA | SectionFlags.MEM_READ },
            { DATA_SECTION_NAME, SectionFlags.CNT_INITIALIZED_DATA | SectionFlags.MEM_READ | SectionFlags.MEM_WRITE },
            { RSRC_SECTION_NAME, SectionFlags.CNT_INITIALIZED_DATA | SectionFlags.MEM_READ },
            { RELOC_SECTION_NAME, SectionFlags.CNT_INITIALIZED_DATA | SectionFlags.MEM_READ | SectionFlags.MEM_DISCARDABLE },
        };

        public uint WriteSize => 4u // signature
                    + 18u // COFF File Header
                    + SizeOfOptionalHeader
                    + (uint)(Sections.Count * Section.WriteSize);

        public static ImageHeader WithDefaultSections()
        {
            var ih = new ImageHeader();
            var sections = new byte[][] {
                TEXT_SECTION_NAME, IDATA_SECTION_NAME, DATA_SECTION_NAME, RSRC_SECTION_NAME, RELOC_SECTION_NAME };
            var i = 1u;
            foreach(var s in sections)
            {
                ih.Sections.Add(new Section
                {
                    Name = s,
                    VirtualSize = 0,
                    SizeOfRawData = ih.FileAlignment,
                    Characteristics = DefaultSectionFlags.Fetch(s, (SectionFlags)0),
                    PointerToRawData = i * ih.FileAlignment,
                    VirtualAddress = 0
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

            foreach (var s in Sections) if (s.VirtualSize != 0) s.Write(bw);
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
                bw.Write(new byte[WriteSize]);
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
            ImportAddressTableRVA = lookupRVA;
            ImportAddressTableSize = lookupSize;
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
        public uint ImportAddressTableRVA { get; internal set; }
        public uint ImportAddressTableSize { get; internal set; }

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
            public uint Size => (8 + Entry.WriteSize* (uint)Entries.Length).AlignTo(4); // total size of block %4 == 0
            public Entry[] Entries;

            public void Write(BinaryWriter bw)
            {
                bw.Write(PageRVA);
                bw.Write(Size);
                foreach (var e in Entries) e.Write(bw);
                bw.PadPosition(4);
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

    public class Resources
    {
        public enum Types
        {
            CURSOR = 1, GROUP_CURSOR = CURSOR + 11,
            BITMAP = 2,
            ICON = 3, GROUP_ICON = ICON + 11,
            MENU = 4,
            DIALOG = 5,
            STRING = 6, // String-table entry.
            FONTDIR = 7,
            FONT = 8,
            ACCELERATOR = 9,
            RCDATA = 10, // Raw Data
            MESSAGETABLE = 11,
            VERSION = 16,
            DLGINCLUDE = 17,
            PLUGPLAY = 19,
            VXD = 20,
            ANICURSOR = 21, // Animated cursor.
            ANIICON = 22, // Animated icon.
            HTML = 23,
            MANIFEST = 24,
            FirstUserType = 256,
        }
        public enum Languages : uint
        {
            // see https://msdn.microsoft.com/en-us/library/windows/desktop/aa381058(v=vs.85).aspx
            German = 0x407,
            UsEnglish = 0x409,
        }
        public enum CodePages : uint
        {
            Ascii = 0,
            Unicode = 1200, // 0x4B0
            MultiLingual = 1252,
        }

        public abstract class IData
        {
            abstract public uint WriteSize { get; }
            abstract public void Write(BinaryWriter bw);

            internal uint PaddedWriteSize => WriteSize.AlignTo(16);
            internal void PaddedWrite(BinaryWriter bw)
            {
                Write(bw);
                bw.PadPosition(16);
            }
        }

        public class PlainData : IData
        {
            public byte[] Data;

            public override uint WriteSize => (uint)Data.Length;

            public override void Write(BinaryWriter bw)
            {
                bw.Write(Data);
            }
        }

        public class IconGroup : IData
        {
            public ushort Reserved => 0;
            public ushort ResourceType => 1;
            public ushort Count => (ushort)Entries.Length;
            public ushort Padding => 0;

            public struct Entry
            {
                public byte Width;
                public byte Height;
                public byte ColorCount;
                public byte Reserved => 0;
                public ushort Planes;
                public ushort BitCount;
                public uint Size;
                public ushort Ordinal;
                //public ushort Padding => 0;

                public static uint WriteSize => 14;
                public void Write(BinaryWriter bw)
                {
                    bw.Write(Width);
                    bw.Write(Height);
                    bw.Write(ColorCount);
                    bw.Write(Reserved);
                    bw.Write(Planes);
                    bw.Write(BitCount);
                    bw.Write(Size);
                    bw.Write(Ordinal);
                    //bw.Write(Padding);
                }
            }
            public Entry[] Entries = new Entry[0];

            public override uint WriteSize => 6 + (uint)Entries.Length * Entry.WriteSize;

            public override void Write(BinaryWriter bw)
            {
                bw.Write(Reserved);
                bw.Write(ResourceType);
                bw.Write(Count);
                foreach (var entry in Entries) entry.Write(bw);
            }
        }

        public struct Entry
        {
            public Types Type;
            public string Name;
            public uint Ordinal; // used when Name is null or empty
            public Languages Language;
            public CodePages CodePage;
            public IData Data;
        }

        private Dictionary<Types, uint> _nextOrdinal = new Dictionary<Types, uint>();
        public uint NextOrdinal(Types type) {
            var result = _nextOrdinal.GetOrAdd(type, () => 1u);
            _nextOrdinal[type]++;
            return result;
        }

        public IList<Entry> Entries { get; set; } = new List<Entry>();

        public class IconParameters
        {
            public string Name;
            public uint Ordinal;
            public Languages Language = Languages.UsEnglish;
            public CodePages CodePage = 0;
            public Stream Stream;
        }
        public void AddIcon(IconParameters p)
        {
            using (var br = new BinaryReader(p.Stream))
            {
                var dirReserved = br.ReadUInt16();
                var dirType = br.ReadUInt16();
                if (dirReserved != 0 || dirType != 1) throw new ArgumentException("Not an Icon");
                var numEntries = br.ReadUInt16();

                var groupEntries = new List<IconGroup.Entry>();

                for (var i = 0; i < numEntries; i++)
                {
                    var width = br.ReadByte();
                    var height = br.ReadByte();
                    var colorCount = br.ReadByte();
                    var reserved = br.ReadByte();
                    var planes = br.ReadUInt16();
                    var bitCount = br.ReadUInt16();
                    var size = br.ReadUInt32();
                    var offset = br.ReadUInt32();

                    var streamPosition = br.BaseStream.Position;
                    br.BaseStream.Seek(offset, SeekOrigin.Begin);
                    var data = br.ReadBytes((int)size);
                    br.BaseStream.Seek(streamPosition, SeekOrigin.Begin);

                    var ordinal = NextOrdinal(Types.ICON);

                    groupEntries.Add(new IconGroup.Entry
                    {
                        Width = width,
                        Height = height,
                        ColorCount = colorCount,
                        Planes = planes,
                        BitCount = bitCount,
                        Size = size,
                        Ordinal = (ushort)ordinal,
                    });
                    Entries.Add(new Entry
                    {
                        Type = Types.ICON,
                        Ordinal = ordinal,
                        Language = p.Language,
                        CodePage = p.CodePage,
                        Data = new PlainData { Data = data }
                    });                   
                }

                if (string.IsNullOrEmpty(p.Name) && p.Ordinal == 0)
                    p.Ordinal = NextOrdinal(Types.GROUP_ICON);

                Entries.Add(new Entry
                {
                    Type = Types.GROUP_ICON,
                    Name = p.Name,
                    Ordinal = p.Ordinal,
                    Language = p.Language,
                    CodePage = p.CodePage,
                    Data = new IconGroup { Entries = groupEntries.ToArray() }
                });
            }
        }

        public class ManifestParameters
        {
            public string Name;
            public uint Ordinal;
            public Languages Language = Languages.UsEnglish;
            public CodePages CodePage = 0;
            //public string Text;
            public Stream Stream;
        }
        public void AddManifest(ManifestParameters p)
        {
            if (string.IsNullOrEmpty(p.Name) && p.Ordinal == 0)
                p.Ordinal = NextOrdinal(Types.MANIFEST);

            using (var br = new BinaryReader(p.Stream))
            {
                var data = br.ReadBytes((int)br.BaseStream.Length);
                //var data = Encoding.GetEncoding((int)p.CodePage).GetBytes(p.Text);
                //var data = Encoding.ASCII.GetBytes(p.Text);
                Entries.Add(new Entry
                {
                    Type = Types.MANIFEST,
                    Name = p.Name,
                    Ordinal = p.Ordinal,
                    Language = p.Language,
                    CodePage = p.CodePage,
                    Data = new PlainData { Data = data }
                });
            }
        }

        [Flags]
        public enum FileFlag : uint
        {
            Debug = 1 << 0,
            PreRelease = 1 << 1,
            Patched = 1 << 2,
            PrivateBuild = 1 << 3,
            InfoInferred = 1 << 4,
            SpecialBuild = 1 << 5,
        }

        public enum FileOS : uint
        {
            UNKNOWN = 0,
            DOS = 0x0001_0000,
            OS216 = 0x0002_0000, // OS/2
            OS232 = 0x0003_0000, // OS/2
            NT = 0x0004_0000,

            WINDOWS16 = 0x0000_0001,
            PM16 = 0x0000_0002, // Presentation Manager
            PM32 = 0x0000_0003, // Presentation Manager
            WINDOWS32 = 0x0000_0004,

            // Valid Combos
            DOS_WINDOWS16 = 0x0001_0001,
            DOS_WINDOWS32 = 0x0001_0004,
            NT_WINDOWS32 = 0x0004_0004,
            OS216_PM16 = 0x0002_0002,
            OS232_PM32 = 0x0003_0003,
        }

        public enum FileType
        {
            UNKNOWN,
            APP,
            DLL,
            DRV,
            FONT,
            VXD,
            STATIC_LIB = 7,
        }

        public enum VersionKeys
        {
            Comments,
            CompanyName,
            FileDescription,
            FileVersion,
            InternalName,
            LegalCopyright,
            LegalTrademarks,
            OriginalFilename,
            PrivateBuild,
            ProductName,
            ProductVersion,
            SpecialBuild,
        }

        public class VersionParameters
        {
            public string Name;
            public uint Ordinal;
            public Languages Language = Languages.UsEnglish;
            public CodePages CodePage = 0;

            public Fixed FixedData { get; } = new Fixed();
            public Dictionary<LanguageCodePage, Dictionary<VersionKeys, string>> StringTables { get; } = new Dictionary<LanguageCodePage, Dictionary<VersionKeys, string>>();

            private IEnumerable<(LanguageCodePage, IEnumerable<KeyValuePair<VersionKeys, string>>)> ValidStringTables
                => StringTables
                    .Select(e => (e.Key, e.Value.Where(x => !string.IsNullOrEmpty(x.Value))))
                    .Where(e => e.Item2.Count() != 0);
            
            public struct LanguageCodePage
            {
                public Languages Language;
                public CodePages CodePage;

                static internal uint WriteSize => 4;
                internal void Write(BinaryWriter bw)
                {
                    bw.Write((ushort)Language);
                    bw.Write((ushort)CodePage);
                }
            }

            public class Fixed
            {
                public uint Signature => 0xFEEF04BD;
                public uint StrucVersion = 0x0001_0000;
                public (ushort,ushort,ushort,ushort) FileVersion; // 1.0.5.7
                public (ushort,ushort,ushort,ushort) ProductVersion;
                public FileFlag FileFlagsMask = (FileFlag)Enum.GetValues(typeof(FileFlag)).Cast<int>().Sum(v => v);
                public FileFlag FileFlags;
                public FileOS FileOS = FileOS.NT_WINDOWS32;
                public FileType FileType = FileType.APP;
                public uint FileSubtype => 0; // only used for DRV & FONT
                public ulong FileDate => 0; // not used (encoding unknown)

                public static uint WriteSize => 13 * 4;

                public void Write(BinaryWriter bw)
                {
                    bw.Write(Signature);
                    bw.Write(StrucVersion);
                    bw.Write((((uint)FileVersion.Item1) << 16) + FileVersion.Item2);
                    bw.Write((((uint)FileVersion.Item3) << 16) + FileVersion.Item4);
                    bw.Write((((uint)ProductVersion.Item1) << 16) + ProductVersion.Item2);
                    bw.Write((((uint)ProductVersion.Item3) << 16) + ProductVersion.Item4);
                    bw.Write((uint)FileFlagsMask);
                    bw.Write((uint)FileFlags);
                    bw.Write((uint)FileOS);
                    bw.Write((uint)FileType);
                    bw.Write(FileSubtype);
                    bw.Write(FileDate);
                }
            }

            uint TranslationsWriteVarSize => (uint)(32 + ValidStringTables.Count() * LanguageCodePage.WriteSize);
            void TranslationsWriteVar(BinaryWriter bw)
            {
                bw.Write((ushort)TranslationsWriteVarSize);
                bw.Write((ushort)(StringTables.Count * LanguageCodePage.WriteSize));
                bw.Write((ushort)0);
                bw.Write(Encoding.Unicode.GetBytes("Translation\0"));
                bw.PadPosition(4);
                foreach (var t in ValidStringTables) t.Item1.Write(bw);
            }

            uint TranslationsWriteSize => StringTables.IsEmpty() ? 0 : 32 + TranslationsWriteVarSize;
            void TranslationsWrite(BinaryWriter bw)
            {
                if (StringTables.IsEmpty()) return;
                bw.Write((ushort)TranslationsWriteSize);
                bw.Write((ushort)0);
                bw.Write((ushort)1); // Type
                bw.Write(Encoding.Unicode.GetBytes("VarFileInfo\0"));
                bw.PadPosition(4);
                TranslationsWriteVar(bw);
            }

            uint StringTablesWriteSize => (6u + 15 * 2).AlignTo(4)
                        + (uint)ValidStringTables.Sum(e => StringTableWriteSize(e.Item1, e.Item2));
            void StringTablesWrite(BinaryWriter bw)
            {
                if (StringTables.IsEmpty()) return;
                bw.Write((ushort)StringTablesWriteSize);
                bw.Write((ushort)0);
                bw.Write((ushort)1); // Type
                bw.Write(Encoding.Unicode.GetBytes("StringFileInfo\0"));
                bw.PadPosition(4);
                foreach (var entry in ValidStringTables)
                    StringTableWrite(entry.Item1, entry.Item2, bw);
            }

            private static uint StringTableWriteSize(LanguageCodePage key, IEnumerable<KeyValuePair<VersionKeys, string>> dict)
            {
                return (6u + (8+1)*2).AlignTo(4)
                    + (uint)dict.Sum(e => StringTableEntryWriteSize(e.Key, e.Value));
            }
            private static void StringTableWrite(LanguageCodePage key, IEnumerable<KeyValuePair<VersionKeys, string>> dict, BinaryWriter bw)
            {
                bw.Write((ushort)StringTableWriteSize(key, dict)); // Size
                bw.Write((ushort)0);
                bw.Write((ushort)1); // Type
                bw.Write(Encoding.Unicode.GetBytes($"{(ushort)key.Language:X4}{(ushort)key.CodePage:X4}\0"));
                bw.PadPosition(4);
                foreach (var entry in dict)
                    StringTableEntryWrite(entry.Key, entry.Value, bw);
            }

            private static uint StringTableEntryWriteSize(VersionKeys key, string value)
            {
                return (6 + (uint)Encoding.Unicode.GetByteCount(key.ToString()) + 2).AlignTo(4)
                    + ((uint)Encoding.Unicode.GetByteCount(value) + 2).AlignTo(4);
            }
            private static void StringTableEntryWrite(VersionKeys key, string value, BinaryWriter bw)
            {
                var keyString = key.ToString();
                var keyBytes = Encoding.Unicode.GetBytes(keyString);
                var valueBytes = Encoding.Unicode.GetBytes(value);
                bw.Write((ushort)StringTableEntryWriteSize(key, value)); // Size
                bw.Write((ushort)(value.Length + 1)); // includes the zero
                bw.Write((ushort)1); // Type
                bw.Write(keyBytes);
                bw.Write((ushort)0);
                bw.PadPosition(4);
                bw.Write(valueBytes);
                bw.Write((ushort)0);
                bw.PadPosition(4);
            }

            public uint WriteSize => (6u + (15 + 1)*2).AlignTo(4) + Fixed.WriteSize + StringTablesWriteSize + TranslationsWriteSize;
           public void Write(BinaryWriter bw)
            {
                bw.Write((ushort)(WriteSize));
                bw.Write((ushort)Fixed.WriteSize);
                bw.Write((ushort)0);
                bw.Write(Encoding.Unicode.GetBytes("VS_VERSION_INFO\0"));
                bw.PadPosition(4);
                FixedData.Write(bw);
                bw.PadPosition(4);
                StringTablesWrite(bw);
                TranslationsWrite(bw);
            }
        }

        public void AddVersionInfo(VersionParameters p)
        {
            if (string.IsNullOrEmpty(p.Name) && p.Ordinal == 0)
                p.Ordinal = NextOrdinal(Types.VERSION);

            using (var s = new MemoryStream()) {
                using (var bw = new BinaryWriter(s))
                {
                    p.Write(bw);
                }
                var data = s.ToArray();
                Entries.Add(new Entry
                {
                    Type = Types.VERSION,
                    Name = p.Name,
                    Ordinal = p.Ordinal,
                    Language = p.Language,
                    CodePage = p.CodePage,
                    Data = new PlainData { Data = data }
                });
            }
        }
    }

    internal class ResourceSection
    {
        public ResourceSection(Resources resources)
        {
            var rootTable = new DirectoryTable();
            var tables = new List<DirectoryTable> { rootTable };
            var names = new List<string>();
            var descriptions = new List<DataDescription>();
            Data = new Resources.IData[resources.Entries.Count];

            var orderedResources = resources.Entries.OrderBy(e => e.Type)
                .ThenBy(e => string.IsNullOrEmpty(e.Name))
                .ThenBy(e => e.Name).ThenBy(e => e.Ordinal)
                .ThenBy(e => e.Language);

            var typeTables = new List<ValueTuple<DirectoryTable, IEnumerable<Resources.Entry>>>();
            rootTable.OrdinalEntries = orderedResources.GroupBy(e => e.Type, (type, typeGroup) =>
            {
                var subTypeTableIndex = (uint)tables.Count;
                var typeTable = new DirectoryTable();
                tables.Add(typeTable);
                typeTables.Add((typeTable, typeGroup));

                return new DirectoryTable.Entry
                {
                    IsName = false,
                    Ordinal = (uint)type,
                    IsLeaf = false,
                    SubTableIndex = subTypeTableIndex,
                };
            }).ToArray();

            var nameTables = new List<ValueTuple<DirectoryTable, IEnumerable<Resources.Entry>>>();
            foreach (var (typeTable, typeGroup) in typeTables)
            {
                foreach (var entry in typeGroup.GroupBy(
                    e => (e.Name, (string.IsNullOrEmpty(e.Name) ? e.Ordinal : 0)),
                    (id, nameGroup) =>
                    {
                        var subNameTableIndex = (uint)tables.Count;
                        var nameTable = new DirectoryTable();
                        tables.Add(nameTable);
                        nameTables.Add((nameTable, nameGroup));

                        var isName = !string.IsNullOrEmpty(id.Item1);
                        var nameIndex = names.Count;
                        if (isName)
                            names.Add(id.Item1);

                        return new DirectoryTable.Entry
                        {
                            IsName = isName,
                            NameIndex = (uint)nameIndex,
                            Ordinal = id.Item2,
                            IsLeaf = false,
                            SubTableIndex = subNameTableIndex,
                        };
                    }).GroupBy(e => e.IsName))
                {
                    if (entry.Key)
                        typeTable.NameEntries = entry.ToArray();
                    else
                        typeTable.OrdinalEntries = entry.ToArray();
                }
            }

            foreach (var (nameTable, nameGroup) in nameTables)
            {
                nameTable.OrdinalEntries = nameGroup.Select(entry =>
                {
                    var dataIndex = (uint)descriptions.Count;

                    descriptions.Add(new DataDescription
                    {
                        DataIndex = dataIndex,
                        Size = entry.Data.WriteSize,
                        CodePage = (uint)entry.CodePage
                    });

                    Data[dataIndex] = entry.Data;

                    return new DirectoryTable.Entry
                    {
                        IsName = false,
                        Ordinal = (uint)entry.Language,
                        IsLeaf = true,
                        DataIndex = dataIndex,
                    };
                }).ToArray();
            }

            DirectoryTables = tables.ToArray();
            DirectoryStrings = names.ToArray();
            DataDescriptions = descriptions.ToArray();
        }

        internal class DirectoryTable
        {
            public uint Characteristics => 0;
            public uint TimeStamp;
            public ushort MajorVersion = 0;
            public ushort MinorVersion = 0;
            public ushort NumberOfNameEntries => (ushort)NameEntries.Length;
            public ushort NumberOfOrdinalEntries => (ushort)OrdinalEntries.Length;

            internal class Entry
            {
                public bool IsName;
                public uint NameIndex;
                public uint Ordinal;

                public bool IsLeaf;
                public uint DataIndex;
                public uint SubTableIndex;

                public static uint WriteSize => 8;
                public void Write(BinaryWriter bw, Func<uint, uint> nameOffset, Func<uint, uint> descriptionOffset, Func<uint, uint> tableOffset)
                {
                    if (IsName)
                        bw.Write(0x8000_0000u | nameOffset(NameIndex)); // actually an offset in Section
                    else
                        bw.Write(Ordinal);

                    if (IsLeaf)
                        bw.Write(descriptionOffset(DataIndex)); // actually an offset in Section
                    else
                        bw.Write(0x8000_0000u | tableOffset(SubTableIndex)); // actually an offset in Section
                }
            }
            public Entry[] NameEntries = new Entry[0];
            public Entry[] OrdinalEntries = new Entry[0];

            public uint WriteSize => 16u + (uint)(NameEntries.Length + OrdinalEntries.Length) * Entry.WriteSize;

            public void Write(BinaryWriter bw, Func<uint, uint> nameOffset, Func<uint, uint> descriptionOffset, Func<uint, uint> tableOffset)
            {
                bw.Write(Characteristics);
                bw.Write(TimeStamp);
                bw.Write(MajorVersion);
                bw.Write(MinorVersion);
                bw.Write(NumberOfNameEntries);
                bw.Write(NumberOfOrdinalEntries);
                foreach (var e in NameEntries) e.Write(bw, nameOffset, descriptionOffset, tableOffset);
                foreach (var e in OrdinalEntries) e.Write(bw, nameOffset, descriptionOffset, tableOffset);
            }
        }

        static uint StringWriteSize(string str)
        {
            return 2u + (uint)Encoding.Unicode.GetByteCount(str);
        }

        static void StringWrite(BinaryWriter bw, string str)
        {
            var bytes = Encoding.Unicode.GetBytes(str);
            bw.Write((ushort)str.Length);
            bw.Write(bytes);
        }

        internal class DataDescription
        {
            public uint DataIndex;
            public uint Size;
            public uint CodePage;
            public uint Reserved => 0;

            public static uint WriteSize => 16;
            public void Write(BinaryWriter bw, Func<uint, uint> indexToRVA)
            {
                bw.Write(indexToRVA(DataIndex));
                bw.Write(Size);
                bw.Write(CodePage);
                bw.Write(Reserved);
            }
        }

        // 3 Nested Tables: Type, Name, Language
        private DirectoryTable[] DirectoryTables;
        private string[] DirectoryStrings;
        private DataDescription[] DataDescriptions;
        private Resources.IData[] Data;

        private uint TablesSize => (uint)(DirectoryTables.Sum(dt => dt.WriteSize));
        private uint DescriptionSize => (uint)(DataDescriptions.Length * DataDescription.WriteSize);
        private uint RawStringSize => (uint)DirectoryStrings.Sum(ds => StringWriteSize(ds));
        private uint StringsSize => RawStringSize.AlignTo(16);
        private uint DataSize => (uint)(Data.Sum(dt => dt.PaddedWriteSize));

        public uint WriteSize => TablesSize + DescriptionSize + StringsSize + DataSize;

        public void Write(BinaryWriter bw, ulong baseRVA)
        {
            var tableOffset = 0u;
            var descriptionOffset = tableOffset + TablesSize;
            var stringOffset = descriptionOffset + DescriptionSize;
            var dataRVA = (uint)(baseRVA + stringOffset + StringsSize);
            foreach (var table in DirectoryTables)
                table.Write(bw,
                    (index) => stringOffset + (uint)DirectoryStrings.Take((int)index).Sum(ds => StringWriteSize(ds)),
                    (index) => descriptionOffset + index * DataDescription.WriteSize,
                    (index) => tableOffset + (uint)DirectoryTables.Take((int)index).Sum(dt => dt.WriteSize));

            foreach (var dataDescription in DataDescriptions)
                dataDescription.Write(bw, 
                    (index) => dataRVA + (uint)Data.Take((int)index).Sum(d => d.PaddedWriteSize));

            foreach (var str in DirectoryStrings) StringWrite(bw, str);
            bw.PadPosition(16);

            foreach (var data in Data) data.PaddedWrite(bw);
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
