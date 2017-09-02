using REC.Packaging.Data;
using REC.Packaging.Image;
using System.IO;
using System.Text;

namespace REC.Packaging.PortableExecutable
{
    internal interface IDataSection : ISection
    {
        IData Data { get; }
    }

    internal class DataSection : AbstractImagePart, IDataSection
    {
        public byte[] Name => Encoding.ASCII.GetBytes(".data");
        public SectionFlags Characteristics => SectionFlags.CNT_INITIALIZED_DATA | SectionFlags.MEM_READ | SectionFlags.MEM_WRITE;

        public IData Data { get; } = new Data.Data();

        public override IBindProvider<ulong> Size => Data.Size;
        public override IBindProvider<ulong> MemoryOffset => Data.MemoryOffset;

        public override void Write(BinaryWriter bw) => Data.Write(bw);
    }
}
