using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Text;
using System;
using REC.Packaging.Tools;
using REC.Packaging.Image;
using REC.Tools;
using REC.Packaging.Resource;

namespace REC.Packaging.PortableExecutable
{
    internal interface IResourceSection : ISection
    {
        IResources Resources { get; }
    }

    internal class ResourceSection : AbstractImagePart, IResourceSection
    {
        public byte[] Name => Encoding.ASCII.GetBytes(".rsrc");
        public SectionFlags Characteristics => SectionFlags.CNT_INITIALIZED_DATA | SectionFlags.MEM_READ;

        public IResources Resources { get; } = new Resources();

        public override IBindProvider<ulong> Size => Resources.Size;
        public override IBindProvider<ulong> MemoryOffset => Resources.MemoryOffset;

        public override void Write(BinaryWriter bw) => Resources.Write(bw);
    }
}
