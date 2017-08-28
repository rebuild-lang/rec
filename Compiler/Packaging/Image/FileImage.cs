using REC.Packaging.Tools;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace REC.Packaging.Image
{
    interface IFileImage
    {
        void AddPart(IImagePart filePart);
        void Write(BinaryWriter binaryWriter);
    }

    class FileImage : IFileImage
    {
        public struct Config
        {
            public uint FileAlignment;
            public uint MemoryAlignment;
        }

        private IList<IImagePart> _parts;
        private Config _config;

        public FileImage(Config config) {
            _parts = new List<IImagePart>();
            _config = config;
        }

        public void AddPart(IImagePart part) {
            AlignedBinding.Create(_config.FileAlignment, part.Size, part.FileSize);
            AlignedBinding.Create(_config.MemoryAlignment, part.Size, part.MemorySize);

            if (_parts.Count != 0) {
                var previous = _parts.Last();
                SumBinding.Create(new[] { previous.FileOffset, previous.FileSize }, part.FileOffset);
                SumBinding.Create(new[] { previous.MemoryOffset, previous.MemorySize }, part.MemoryOffset);
            }
            else {
                part.FileOffset.SetValue(0);
                part.MemoryOffset.SetValue(0);
            }
            _parts.Add(part);
        }

        public void Write(BinaryWriter binaryWriter) {
            foreach (var part in _parts) {
                var o = binaryWriter.BaseStream.Position;
                part.Write(binaryWriter);
                var s = binaryWriter.BaseStream.Position - o;
                var r = (uint)part.Size.Value.GetValueOrDefault(0);
                if (s != r) throw new InvalidDataException($"wrong section size: {s} bytes written, {r} reported!");

                binaryWriter.PadPosition((uint)_config.FileAlignment);
            }
        }
    }
}
