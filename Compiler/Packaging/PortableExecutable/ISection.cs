using REC.Packaging.Image;
using System;

namespace REC.Packaging.PortableExecutable
{
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

    internal interface ISection : IImagePart
    {
        byte[] Name { get; }
        SectionFlags Characteristics { get; }
    }
}
