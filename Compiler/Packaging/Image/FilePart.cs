namespace REC.Packaging.Image
{
    interface IFilePart : ISizeWritable
    {
        IBindProvider<ulong> FileOffset { get; }
        IBindProvider<ulong> FileSize { get; }
    }

    abstract class AbstractFilePart : AbstractSizeWritable, IFilePart
    {
        public virtual IBindProvider<ulong> FileOffset { get; } = new BindProvider<ulong>();
        public IBindProvider<ulong> FileSize { get; } = new BindProvider<ulong>();
    }
}
