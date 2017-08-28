namespace REC.Packaging.Image
{
    interface IImagePart : IMemoryPart, IFilePart { }

    abstract class AbstractImagePart : AbstractFilePart, IImagePart
    {
        public virtual IBindProvider<ulong> MemoryOffset { get; } = new BindProvider<ulong>();
        public IBindProvider<ulong> MemorySize { get; } = new BindProvider<ulong>();
    }
}
