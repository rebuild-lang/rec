using REC.Packaging.Image;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using REC.Packaging.Code;
using REC.Packaging.Tools;

namespace REC.Packaging.PortableExecutable
{
    internal class BaseRelocationSection
    {
        public struct Entry
        {
            public Types Type;
            public ushort Offset;

            public static uint WriteSize => 2;

            public void Write(BinaryWriter bw) {
                var word = (ushort)(((ushort)Type & 0xF) * 0x1000 + (Offset & 0x0FFF));
                bw.Write(word);
            }
        }
        public struct Block
        {
            public uint PageRVA;
            public uint Size => (8 + Entry.WriteSize * (uint)Entries.Length).AlignTo(4); // total size of block %4 == 0
            public Entry[] Entries;

            public void Write(BinaryWriter bw) {
                bw.Write(PageRVA);
                bw.Write(Size);
                foreach (var e in Entries) e.Write(bw);
                bw.PadPosition(4);
            }
        }
        public enum Types
        {
            None = 0, // skipped - use for padding the block
            HighWord = 1, // 16 higher bits of difference are added
            LowWord = 2, // 16 lower bits of difference are added
            DWord = 3, // 32bits of difference are added
            HighAdjusted = 4, // what?
            MIPS_JMPADDR = 5,
            ARM_MOV32 = 5, // MOVW/WOVT instructions of ARM/THUMB
            RISCV_HIGH20 = 5,
            THUMB_MOV32 = 7,
            RISCV_LOW12I = 7,
            RISCV_LOW12S = 8,
            MIPS_JMPADDR16 = 9,
            QWord = 10, // 32bits of difference are added to 64 bit address
        }

        public Block[] Blocks;

        public uint WriteSize => (uint)Blocks.Sum(b => b.Size);

        public void Write(BinaryWriter bw) {
            foreach (var block in Blocks) block.Write(bw);
        }
    }

    interface IRelocationSection : ISection
    {
        ICode Code { get; }
        //IData Data { get; } // TODO: Allow relocatable data
    }

    class RelocationSection : AbstractImagePart, IRelocationSection
    {
        public byte[] Name => Encoding.ASCII.GetBytes(".reloc");
        public SectionFlags Characteristics => SectionFlags.CNT_INITIALIZED_DATA | SectionFlags.MEM_READ | SectionFlags.MEM_DISCARDABLE;

        private ICode _code;
        public ICode Code {
            get => _code;
            set {
                if (_code == value) return;
                _code = value;
                FuncBinding.Create(new[] { _code.BaseAddress, _code.Size }, UpdateSize, Size);
            }
        }

        private BaseRelocationSection _data;
        private ulong? UpdateSize() {
            if (!Code.RelocationAddresses().Any()) return 0;
            if (Code.BaseAddress.Value == null) return 1;
            var baseAddress = Code.BaseAddress.Value.Value;
            _data = new BaseRelocationSection {
                Blocks = Code.RelocationAddresses().GroupBy(x => x/0x1000).Select(group => {
                    var pageAddress = group.Key * 0x1000;
                    return new BaseRelocationSection.Block {
                        PageRVA = (uint)(pageAddress - baseAddress),
                        Entries = group.Select(address => new BaseRelocationSection.Entry {
                            Offset = (ushort)(address - pageAddress),
                            Type = BaseRelocationSection.Types.DWord
                        }).ToArray()
                    };
                }).ToArray()
            };
            return _data.WriteSize;
        }

        public override void Write(BinaryWriter binaryWriter) {
            _data.Write(binaryWriter);
        }
    }
}
