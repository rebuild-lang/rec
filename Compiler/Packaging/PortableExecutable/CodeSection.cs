using System.IO;
using System.Text;
using REC.Packaging.Code;
using REC.Packaging.Image;

namespace REC.Packaging.PortableExecutable
{
    internal interface ICodeSection : ISection
    {
        ICode Code { get; }
    }

    internal class CodeSection : AbstractImagePart, ICodeSection
    {
        public byte[] Name => Encoding.ASCII.GetBytes(".text");
        public SectionFlags Characteristics => SectionFlags.CNT_CODE | SectionFlags.MEM_READ | SectionFlags.MEM_EXECUTE;

        public ICode Code { get; } = new Code.Code();

        public override IBindProvider<ulong> Size => Code.Size;
        public override IBindProvider<ulong> MemoryOffset => Code.MemoryOffset;

        public override void Write(BinaryWriter bw) => Code.Write(bw);
    }
}
