using System.IO;

namespace REC.Packaging.Image
{
    interface ISizeWritable
    {
        IBindProvider<ulong> Size { get; }

        void Write(BinaryWriter binaryWriter); // writes exactly Size bytes!
    }

    abstract class AbstractSizeWritable : ISizeWritable
    {
        public virtual IBindProvider<ulong> Size { get; } = new BindProvider<ulong>();

        public abstract void Write(BinaryWriter binaryWriter);
    }
}
