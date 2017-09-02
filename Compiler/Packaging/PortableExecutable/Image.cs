using REC.Packaging.Code;
using REC.Packaging.Data;
using REC.Packaging.Image;
using REC.Packaging.Resource;
using System.IO;

namespace REC.Packaging.PortableExecutable
{
    internal interface IImage : ISizeWritable
    {
        IImageHeader Header { get; }
        ICode Code { get; }
        ILabel EntryLabel { get; set; }
        IImports Imports { get; }
        IData Data { get; }
        IResources Resources { get; }
    }

    internal class Image : AbstractSizeWritable, IImage
    {
        private IFileImage _fileImage;
        private ICodeSection _codeSection = new CodeSection();
        private IImportSection _importSection = new ImportSection();
        private IDataSection _dataSection = new DataSection();
        private IResourceSection _resourceSection = new ResourceSection();
        private IRelocationSection _relocationSection;

        public IImageHeader Header { get; } = new ImageHeader();
        public ICode Code => _codeSection.Code;
        public IData Data => _dataSection.Data;
        public IImports Imports => _importSection;
        public IResources Resources => _resourceSection.Resources;

        private ILabel _entryLabel;
        public ILabel EntryLabel {
            get => _entryLabel;
            set {
                if (_entryLabel == value) return;
                _entryLabel = value;
                FuncBinding.Create(_entryLabel.MemoryAddress, Header.ImageBase, (a, b) => a - b, Header.AddressOfEntryPoint);
            }
        }

        public Image() {
            _relocationSection = new RelocationSection {
                Code = _codeSection.Code
            };

            Header.AddSection(_codeSection);
            Header.AddSection(_importSection);
            Header.AddSection(_dataSection);
            Header.AddSection(_resourceSection);
            Header.AddSection(_relocationSection);

            FuncBinding.Create(new[] { Header.FileAlignment, Header.SectionAlignment }, UpdateSize, Size);

            ValueBinding.Create(Header.ImageBase, Code.BaseAddress);
            ValueBinding.Create(Header.ImageBase, Data.BaseAddress);
            ValueBinding.Create(Header.ImageBase, Imports.BaseAddress);

            ValueBinding.Create(_codeSection.MemoryOffset, Header.BaseOfCode);
            ValueBinding.Create(_dataSection.MemoryOffset, Header.BaseOfData);
            //ValueBinding.Create(_codeSection.MemoryOffset, Header.AddressOfEntryPoint);

            ValueBinding.Create(Header.Magic, _importSection.Magic);
            ValueBinding.Create(_importSection.ImportTableRVA, Header.DataDirectories[(int)DataDirectoryIndex.ImportTable].VirtualAddress);
            ValueBinding.Create(_importSection.ImportTableSize, Header.DataDirectories[(int)DataDirectoryIndex.ImportTable].Size);
            ValueBinding.Create(_importSection.ImportAddressTableRVA, Header.DataDirectories[(int)DataDirectoryIndex.ImportAddressTable].VirtualAddress);
            ValueBinding.Create(_importSection.ImportAddressTableSize, Header.DataDirectories[(int)DataDirectoryIndex.ImportAddressTable].Size);

            CastBinding.Create(_resourceSection.MemoryOffset, x => (uint?)x, Header.DataDirectories[(int)DataDirectoryIndex.ResourceTable].VirtualAddress);
            CastBinding.Create(_resourceSection.Size, x => (uint?)x, Header.DataDirectories[(int)DataDirectoryIndex.ResourceTable].Size);

            CastBinding.Create(_relocationSection.MemoryOffset, x => (uint?)x, Header.DataDirectories[(int)DataDirectoryIndex.BaseRelocationTable].VirtualAddress);
            CastBinding.Create(_relocationSection.Size, x => (uint?)x, Header.DataDirectories[(int)DataDirectoryIndex.BaseRelocationTable].Size);
        }

        private ulong? UpdateSize() {
            _fileImage = new FileImage(new FileImage.Config {
                FileAlignment = (uint)Header.FileAlignment.Value.Value,
                MemoryAlignment = (uint)Header.SectionAlignment.Value.Value
            });
            _fileImage.AddPart(Header);
            _fileImage.AddPart(_codeSection);
            _fileImage.AddPart(_importSection);
            _fileImage.AddPart(_dataSection);
            _fileImage.AddPart(_resourceSection);
            _fileImage.AddPart(_relocationSection);
            return null;
        }

        //private static uint TimeStamp() {
        //    return (uint)DateTime.UtcNow.Subtract(new DateTime(1970, 1, 1)).TotalSeconds;
        //}

        //private void InjectAddress(byte[] code, uint r, ulong address) {
        //    if (Header.Magic == MagicNumber.PE32) {
        //        code[r + 0] = (byte)(address >> 0);
        //        code[r + 1] = (byte)(address >> 8);
        //        code[r + 2] = (byte)(address >> 16);
        //        code[r + 3] = (byte)(address >> 24);
        //    }
        //    else {
        //        code[r + 0] = (byte)(address >> 0);
        //        code[r + 1] = (byte)(address >> 8);
        //        code[r + 2] = (byte)(address >> 16);
        //        code[r + 3] = (byte)(address >> 24);
        //        code[r + 4] = (byte)(address >> 32);
        //        code[r + 5] = (byte)(address >> 40);
        //        code[r + 6] = (byte)(address >> 48);
        //        code[r + 7] = (byte)(address >> 56);
        //    }
        //}

        public override void Write(BinaryWriter binaryWriter) {
            _fileImage.Write(binaryWriter);
        }
    }
}
