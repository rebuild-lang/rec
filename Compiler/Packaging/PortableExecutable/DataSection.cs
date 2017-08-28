using REC.Packaging.Image;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace REC.Packaging.PortableExecutable
{
    interface IDataSection : ISection
    {
        // TODO
    }

    class DataSection : AbstractImagePart, IDataSection
    {
        public byte[] Name => Encoding.ASCII.GetBytes(".data");
        public SectionFlags Characteristics => SectionFlags.CNT_INITIALIZED_DATA | SectionFlags.MEM_READ | SectionFlags.MEM_WRITE;

        public override void Write(BinaryWriter binaryWriter) {
            // TODO
        }
    }
}
