namespace REC.Packaging.PortableExecutable
{
    //internal interface IImageConfig
    //{
    //    ulong MemoryBaseAddress { get; }
    //    uint FileAlignment { get; }
    //    uint MemoryAlignment { get; }
    //}

    //internal class DelayLoadDirectoryTable
    //{
    //    public uint Attributes => 0;
    //    public uint NameRVA; // name of the DLL to be loaded. The name resides in the read-only data section of the image.
    //    public uint ModuleHandleRVA; // in the data section of the image. It is used for storage by the routine that is supplied to manage delay-loading.
    //    public uint DelayImportAddressTableRVA;
    //    public uint DelayImportNameTableRVA;
    //    public uint BoundDelayImportTableRVA;
    //    public uint UnloadDelayImportTableRVA;
    //    public uint TimeStamp; // timestamp of the DLL to which this image has been bound.
    //}

    //internal class ExportDirectoryTable
    //{
    //    public uint ExportFlags => 0;
    //    public uint TimeStamp; // timestamp the export data was created
    //    public ushort MajorVersion = 0;
    //    public ushort MinorVersion = 0;
    //    public uint NameRVA; // address of the ASCII string that contains the name of the DLL. This address is relative to the image base.
    //    public uint OrdinalBase; // starting ordinal number for exports in this image.This field specifies the starting ordinal number for the export address table.It is usually set to 1.
    //    public uint AddressTableEntries; // number of entries in the export address table.
    //    public uint NumberOfNamePointers; // number of entries in the name pointer table.This is also the number of entries in the ordinal table.
    //    public uint ExportAddressTableRVA; // address of the export address table, relative to the image base.
    //    public uint NamePointerRVA; // address of the export name pointer table, relative to the image base. The table size is given by the Number of Name Pointers field.
    //    public uint OrdinalTableRVA; // address of the ordinal table, relative to the image base.
    //}

    //enum DebugType : uint
    //{
    //    UNKNOWN = 0, COFF, CODEVIEW, FPO, MISC,
    //    EXCEPTION = 5, FIXUP, OMAP_TO_SRC, OMAP_FROM_SRC, BORLAND,
    //    CLSID = 11,
    //    REPRO = 16,
    //}

    //internal class DebugDirectory
    //{
    //    public uint Characteristics => 0;
    //    public uint TimeStamp;
    //    public ushort MajorVersion;
    //    public ushort MinorVersion;
    //    public DebugType Type;
    //    public uint SizeOfData;
    //    public uint AddressOfRawData; // Relative to image base
    //    public uint PointerToRawData; // file pointer to debug data

    //    public static uint WriteSize = 28;
    //}
}
