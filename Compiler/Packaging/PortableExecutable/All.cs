using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Text;
using System;

namespace REC.Packaging.PortableExecutable
{
    internal class Image
    {
        ImageHeader Header { get; } = new ImageHeader();
        byte[] Code;
        List<List<Image_IAT_Header>> IAT_Headers = new List<List<Image_IAT_Header>>();
        List<Image_Import_Header> IIH_Headers = new List<Image_Import_Header>();

        public void Write(BinaryWriter bw)
        {
            DosHeader.Write(bw);
        
            Header.Write(bw);
            // Padding

            bw.Write(Code);
            // Padding


        }

        internal class Image_IAT_Header
        {
            public uint Address;

            public byte[] ToBytes()
            {
                return BitConverter.GetBytes(Address);
            }
        }

        internal class Image_Import_Header
        {
            public uint ImportLookUpTableAddress;
            public uint TimeDateStamp = 0;
            public uint ForwarderChain = 0;
            public uint NameAddress;
            public uint ImportAddressTableAddress;

            public byte[] ToBytes()
            {
                return BitConverter.GetBytes(ImportLookUpTableAddress)
                    .Concat(BitConverter.GetBytes(TimeDateStamp))
                    .Concat(BitConverter.GetBytes(ForwarderChain))
                    .Concat(BitConverter.GetBytes(NameAddress))
                    .Concat(BitConverter.GetBytes(ImportAddressTableAddress))
                    .ToArray();
            }
        }
    }

    internal static class DosHeader
    {
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
        public ushort NumberOfSections => (ushort)Sections.Count;
        public uint TimeDateStamp = 0;
        public uint PointerToSymbolTable => 0; // deprecated
        public uint NumberOfSymbols => 0; // deprecated
        public ushort SizeOfOptionalHeader = 0x00E0;
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
        public uint ImageBase = 0x00400000;
        public uint SectionAlignment = 0x00001000;
        public uint FileAlignment = 0x00000200;

        public ushort MajorOperatingSystemVersion = 0x0004;
        public ushort MinorOperatingSystemVersion = 0x0000;
        public ushort MajorImageVersion = 0x00;
        public ushort MinorImageVersion = 0x00;
        public ushort MajorSubsystemVersion = 0x0004;
        public ushort MinorSubsystemVersion = 0x0000;
        public uint Win32VersionValue => 0;
        public uint SizeOfImage; // The size (in bytes) of the image, including all headers, as the image is loaded in memory. It must be a multiple of SectionAlignment.
        public uint SizeOfHeaders = 0x00000200; // The combined size of an MS DOS stub, PE header, and section headers rounded up to a multiple of FileAlignment.
        public uint CheckSum = 0x00;

        public WindowsSubsystem Subsystem = WindowsSubsystem.WINDOWS_CUI;
        public DllCharacteristics DllCharacteristics = DllCharacteristics.None;
        public uint SizeOfStackReserve = 0x00100000; // The size of the stack to reserve. Only SizeOfStackCommit is committed; the rest is made available one page at a time until the reserve size is reached.
        public uint SizeOfStackCommit = 0x00001000;
        public uint SizeOfHeapReserve = 0x00100000; // The size of the local heap space to reserve. Only SizeOfHeapCommit is committed; the rest is made available one page at a time until the reserve size is reached
        public uint SizeOfHeapCommit = 0x00001000;
        public uint LoaderFlags => 0;
        public uint NumberOfRvaAndSizes => (uint)DataDirectories.Length;

        public DataDirectory[] DataDirectories { get; } = new DataDirectory[16];

        public IList<Section> Sections { get; } = new List<Section>();

        static readonly byte[] TEXT_SECTION_NAME = Encoding.ASCII.GetBytes(".text");
        public Section TextSection => Sections.Single(s => s.Name == TEXT_SECTION_NAME);

        static readonly byte[] RDATA_SECTION_NAME = Encoding.ASCII.GetBytes(".rdata");
        public Section RDataSection => Sections.Single(s => s.Name == RDATA_SECTION_NAME);

        static readonly byte[] DATA_SECTION_NAME = Encoding.ASCII.GetBytes(".data");
        public Section DataSection => Sections.Single(s => s.Name == DATA_SECTION_NAME);

        public static ImageHeader WithDefaultSections()
        {
            var ih = new ImageHeader();
            ih.Sections.Add(new Section
            {
                Name = TEXT_SECTION_NAME,
                VirtualSize = ih.SectionAlignment,
                SizeOfRawData = ih.FileAlignment,
                Characteristics = SectionFlags.MEM_READ | SectionFlags.MEM_EXECUTE | SectionFlags.CNT_CODE,
                PointerToRawData = 1 * ih.FileAlignment,
                VirtualAddress = 1 * ih.SectionAlignment
            });
            ih.Sections.Add(new Section
            {
                Name = RDATA_SECTION_NAME,
                VirtualSize = ih.SectionAlignment,
                SizeOfRawData = ih.FileAlignment,
                Characteristics = SectionFlags.MEM_READ | SectionFlags.CNT_INITIALIZED_DATA,
                PointerToRawData = 2 * ih.FileAlignment,
                VirtualAddress = 2 * ih.SectionAlignment
            });
            ih.Sections.Add(new Section
            {
                Name = DATA_SECTION_NAME,
                VirtualSize = ih.SectionAlignment,
                SizeOfRawData = ih.FileAlignment,
                Characteristics = SectionFlags.MEM_WRITE | SectionFlags.MEM_READ | SectionFlags.CNT_INITIALIZED_DATA,
                PointerToRawData = 3 * ih.FileAlignment,
                VirtualAddress = 3 * ih.SectionAlignment
            });
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

            // Windows Specific Fields
            bw.Write(ImageBase);
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
            bw.Write(SizeOfStackReserve);
            bw.Write(SizeOfStackCommit);
            bw.Write(SizeOfHeapReserve);
            bw.Write(SizeOfHeapCommit);
            bw.Write(LoaderFlags);
            bw.Write(NumberOfRvaAndSizes);
            foreach (var d in DataDirectories) d.Write(bw);

            foreach (var s in Sections) s.Write(bw);
        }

        internal class DataDirectory
        {
            public uint VirtualAddress;
            public uint Size;

            public void Write(BinaryWriter bw)
            {
                bw.Write(VirtualAddress);
                bw.Write(Size);
            }
        }

        [Flags]
        internal enum SectionFlags : uint
        {
            CNT_CODE = 0x00000020,
            CNT_INITIALIZED_DATA = 0x00000040,
            CNT_UNINITIALIZED_DATA = 0x00000080,
            GPREL = 0x00008000, // (IA64 only) The section contains data referenced through the global pointer (GP).
            LNK_NRELOC_OVFL = 0x01000000, // The section contains extended relocations.
            MEM_DISCARDABLE = 0x02000000, // The section can be discarded as needed.
            MEM_NOT_CACHED = 0x04000000, // The section cannot be cached.
            MEM_NOT_PAGED = 0x08000000, // The section is not pageable.
            MEM_SHARED = 0x10000000, // The section can be shared in memory.
            MEM_EXECUTE = 0x20000000,
            MEM_READ = 0x40000000,
            MEM_WRITE = 0x80000000
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
            public uint NumberOfRelocations = 0;
            public uint NumberOfLinenumbers = 0;
            public SectionFlags Characteristics;

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
}
