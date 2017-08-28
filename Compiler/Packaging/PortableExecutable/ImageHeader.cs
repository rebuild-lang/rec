using REC.Packaging.Image;
using REC.Packaging.Tools;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace REC.Packaging.PortableExecutable
{
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

    internal interface IDataDirectory
    {
        IBindProvider<uint> VirtualAddress { get; }
        IBindProvider<uint> Size { get; }

        void Write(BinaryWriter bw);
    }

    internal interface IImageHeader : IImagePart
    {
        uint TimeDateStamp { get; set; }
        IBindProvider<MagicNumber> Magic { get; }

        byte MajorLinkerVersion { get; set; }
        byte MinorLinkerVersion { get; set; }

        IBindProvider<ulong> AddressOfEntryPoint { get; }
        IBindProvider<ulong> BaseOfCode { get; }
        IBindProvider<ulong> BaseOfData { get; } // Only use in PE32 Mode

        IBindProvider<ulong> ImageBase { get; }
        IBindProvider<ulong> SectionAlignment { get; }
        IBindProvider<ulong> FileAlignment { get; }

        ushort MajorOperatingSystemVersion { get; set; }
        ushort MinorOperatingSystemVersion { get; set; }
        Version ImageVersion { get; set; }
        ushort MajorSubsystemVersion { get; set; }
        ushort MinorSubsystemVersion { get; set; }

        WindowsSubsystem Subsystem { get; set; }
        DllCharacteristics DllCharacteristics { get; set; }
        ulong SizeOfStackReserve { get; set; } // The size of the stack to reserve. Only SizeOfStackCommit is committed; the rest is made available one page at a time until the reserve size is reached.
        ulong SizeOfStackCommit { get; set; }
        ulong SizeOfHeapReserve { get; set; } // The size of the local heap space to reserve. Only SizeOfHeapCommit is committed; the rest is made available one page at a time until the reserve size is reached
        ulong SizeOfHeapCommit { get; set; }

        IReadOnlyList<IDataDirectory> DataDirectories { get; }

        IEnumerable<ISection> Sections { get; }

        void AddSection(ISection section);
    }


    internal class ImageHeader : AbstractImagePart, IImageHeader
    {
        public byte[] Signature => new byte[4] { (byte)'P', (byte)'E', 0, 0 };
        // COFF Header
        public MachineTypes Machine = MachineTypes.I386;
        public ushort NumberOfSections => (ushort)Sections.Count(s => s.Size.Value.HasValue);

        public uint TimeDateStamp { get; set; }
        public uint PointerToSymbolTable => 0; // deprecated
        public uint NumberOfSymbols => 0; // deprecated
        public ushort SizeOfOptionalHeader => (ushort)((Magic.Value.Value == MagicNumber.PE32 ? 28 + 68 : 24 + 88) + (DataDirectories.Count * DataDirectory.WriteSize));

        public Characteristics Characteristics =
             //Characteristics.RELOCS_STRIPPED |
             Characteristics.EXECUTABLE_IMAGE |
             Characteristics._32BIT_MACHINE |
             Characteristics.DEBUG_STRIPPED;

        // Optional Header - Standard Fields
        public IBindProvider<MagicNumber> Magic { get; set; } = new BindProvider<MagicNumber>(MagicNumber.PE32);

        public byte MajorLinkerVersion { get; set; } = 14;
        public byte MinorLinkerVersion { get; set; } = 10;
        public uint SizeOfCode => (uint)FilledSections.Where(s => s.Characteristics.HasFlag(SectionFlags.CNT_CODE)).Sum(s => (long)s.Size.Value.Value);
        public uint SizeOfInitializedData => (uint)FilledSections.Where(s => s.Characteristics.HasFlag(SectionFlags.CNT_INITIALIZED_DATA)).Sum(s => (long)s.Size.Value.Value);
        public uint SizeOfUninitializedData => (uint)FilledSections.Where(s => s.Characteristics.HasFlag(SectionFlags.CNT_UNINITIALIZED_DATA)).Sum(s => (long)s.Size.Value.Value);

        public IBindProvider<ulong> AddressOfEntryPoint { get; } = new BindProvider<ulong>();
        public IBindProvider<ulong> BaseOfCode { get; } = new BindProvider<ulong>();
        public IBindProvider<ulong> BaseOfData { get; } = new BindProvider<ulong>();

        public IBindProvider<ulong> ImageBase { get; } = new BindProvider<ulong> { Value = 0x40_00_00 };
        public IBindProvider<ulong> SectionAlignment { get; } = new BindProvider<ulong> { Value = 0x10_00 }; // Memory Alignment
        public IBindProvider<ulong> FileAlignment { get; } = new BindProvider<ulong> { Value = 0x2_00 }; // FileAlignment

        public ushort MajorOperatingSystemVersion { get; set; } = 4;
        public ushort MinorOperatingSystemVersion { get; set; }
        public Version ImageVersion { get; set; }
        public ushort MajorImageVersion => (ushort)ImageVersion.Major;
        public ushort MinorImageVersion => (ushort)ImageVersion.Minor;
        public ushort MajorSubsystemVersion { get; set; } = 4;
        public ushort MinorSubsystemVersion { get; set; }

        public uint Win32VersionValue => 0;
        public uint SizeOfImage => SizeOfHeaders.AlignTo((uint)SectionAlignment.Value.Value) + (uint)Sections.Sum(s => (long)s.MemorySize.Value.GetValueOrDefault(0).AlignTo(SectionAlignment.Value.Value)); // The size (in bytes) of the image, including all headers, as the image is loaded in memory. It must be a multiple of SectionAlignment.
        public uint SizeOfHeaders => (uint)FileSize.Value.Value; // The combined size of an MS DOS stub, PE header, and section headers rounded up to a multiple of FileAlignment.
        public uint CheckSum = 0;

        public WindowsSubsystem Subsystem { get; set; } = WindowsSubsystem.WINDOWS_CUI;
        public DllCharacteristics DllCharacteristics { get; set; } = DllCharacteristics.NX_COMPAT | DllCharacteristics.DYNAMIC_BASE | DllCharacteristics.HIGH_ENTROPY_VA;
        public ulong SizeOfStackReserve { get; set; } = 0x10_00_00;
        public ulong SizeOfStackCommit { get; set; } = 0x10_00;
        public ulong SizeOfHeapReserve { get; set; } = 0x10_00_00;
        public ulong SizeOfHeapCommit { get; set; } = 0x10_00;

        public uint LoaderFlags => 0;
        public uint NumberOfRvaAndSizes => (uint)DataDirectories.Count;

        public IReadOnlyList<IDataDirectory> DataDirectories { get; } = (new bool[16]).Select(x => new DataDirectory()).ToList();

        private IList<ISection> Sections { get; } = new List<ISection>();
        IEnumerable<ISection> IImageHeader.Sections => Sections;

        private IEnumerable<ISection> FilledSections => Sections.Where(s => s.Size.Value.GetValueOrDefault(0) != 0);
        private IEnumerable<SectionHeader> SectionHeaders => FilledSections.Select(s => new SectionHeader {
            Name = s.Name,
            Characteristics = s.Characteristics,
            VirtualSize = (uint)s.Size.Value.Value,
            VirtualAddress = (uint)s.MemoryOffset.Value.Value,
            SizeOfRawData = (uint)s.FileSize.Value.Value,
            PointerToRawData = (uint)s.FileOffset.Value.Value,
            // TODO: relocations!
        });

        public void AddSection(ISection section) {
            Sections.Add(section);
            FuncBinding.Create(Sections.Select(s => s.Size).ToArray(), CalculateSize, Size);
        }

        private ulong? CalculateSize() {
            return DosHeader.WriteSize
                    + 24u // COFF File Header
                    + SizeOfOptionalHeader
                    + (uint)(FilledSections.Count() * SectionHeader.WriteSize);
        }

        public ImageHeader() {
        }

        public override void Write(BinaryWriter bw) {
            DosHeader.Write(bw);

            // COFF Header
            bw.Write(Signature);
            bw.Write((ushort)Machine);
            bw.Write((ushort)NumberOfSections);
            bw.Write((uint)TimeDateStamp);
            bw.Write((uint)PointerToSymbolTable);
            bw.Write((uint)NumberOfSymbols);
            bw.Write((ushort)SizeOfOptionalHeader);
            bw.Write((ushort)Characteristics);

            // Standard COFF Fields
            bw.Write((ushort)Magic.Value.Value);
            bw.Write(new byte[2] { MajorLinkerVersion, MinorLinkerVersion });
            bw.Write((uint)SizeOfCode);
            bw.Write((uint)SizeOfInitializedData);
            bw.Write((uint)SizeOfUninitializedData);
            bw.Write((uint)AddressOfEntryPoint.Value.Value);
            bw.Write((uint)BaseOfCode.Value.GetValueOrDefault(0));
            if (Magic.Value.Value == MagicNumber.PE32) bw.Write((uint)BaseOfData.Value.GetValueOrDefault(0));

            void WriteSizeType(ulong ptr) {
                if (Magic.Value.Value == MagicNumber.PE32)
                    bw.Write((uint)ptr);
                else
                    bw.Write((ulong)ptr);
            }

            // Windows Specific Fields
            WriteSizeType((uint)ImageBase.Value.Value);
            bw.Write((uint)SectionAlignment.Value.Value);
            bw.Write((uint)FileAlignment.Value.Value);
            bw.Write((ushort)MajorOperatingSystemVersion);
            bw.Write((ushort)MinorOperatingSystemVersion);
            bw.Write((ushort)MajorImageVersion);
            bw.Write((ushort)MinorImageVersion);
            bw.Write((ushort)MajorSubsystemVersion);
            bw.Write((ushort)MinorSubsystemVersion);
            bw.Write((uint)Win32VersionValue);
            bw.Write((uint)SizeOfImage);
            bw.Write((uint)SizeOfHeaders);
            bw.Write((uint)CheckSum);
            bw.Write((ushort)Subsystem);
            bw.Write((ushort)DllCharacteristics);
            WriteSizeType(SizeOfStackReserve);
            WriteSizeType(SizeOfStackCommit);
            WriteSizeType(SizeOfHeapReserve);
            WriteSizeType(SizeOfHeapCommit);
            bw.Write((uint)LoaderFlags);
            bw.Write((uint)NumberOfRvaAndSizes);
            foreach (var d in DataDirectories) d.Write(bw);
            foreach (var s in SectionHeaders) s.Write(bw);
        }

        internal static class DosHeader
        {
            static public uint WriteSize = 128;
            static public void Write(BinaryWriter bw) {
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

        internal class DataDirectory : IDataDirectory
        {
            public IBindProvider<uint> VirtualAddress { get; } = new BindProvider<uint>();
            public IBindProvider<uint> Size { get; } = new BindProvider<uint>();

            public static uint WriteSize = 8;

            public void Write(BinaryWriter bw) {
                if (VirtualAddress.Value.GetValueOrDefault(0) != 0 && Size.Value.GetValueOrDefault(0) != 0) {
                    bw.Write((uint)VirtualAddress.Value.Value);
                    bw.Write((uint)Size.Value.Value);
                }
                else {
                    bw.Write((uint)0);
                    bw.Write((uint)0);
                }
            }
        }

        internal class SectionHeader
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

            public void Write(BinaryWriter bw) {
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
}
