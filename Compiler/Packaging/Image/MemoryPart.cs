namespace REC.Packaging.Image
{
    interface IMemoryPart : ISizeWritable
    {
        IBindProvider<ulong> MemoryOffset { get; }
        IBindProvider<ulong> MemorySize { get; }
    }

    abstract class AbstractMemoryPart : AbstractSizeWritable, IMemoryPart
    {
        public IBindProvider<ulong> MemoryOffset { get; } = new BindProvider<ulong>();
        public IBindProvider<ulong> MemorySize { get; } = new BindProvider<ulong>();
    }
}
