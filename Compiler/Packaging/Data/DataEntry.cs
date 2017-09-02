using REC.Packaging.Image;
using System;

namespace REC.Packaging.Data
{
    [Flags]
    enum DataEntryFlags
    {
        Fixed = 1 << 0, // content does not depend on address
    }

    internal interface IDataEntry : ISizeWritable
    {
        DataEntryFlags Flags { get; }
        IBindProvider<ulong> MemoryAddress { get; }
        ulong? RelocationAddress { get; }
    }

    internal abstract class AbstractDataEntry : AbstractSizeWritable, IDataEntry
    {
        public abstract DataEntryFlags Flags { get; }
        public IBindProvider<ulong> MemoryAddress { get; } = new BindProvider<ulong>();
        public virtual ulong? RelocationAddress => null;
    }
}
