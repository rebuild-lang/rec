using System;
using System.IO;
using REC.Packaging.Code;
using REC.Packaging.Image;

namespace REC.Packaging.x86
{
    internal enum LabelInstructionType
    {
        CallRelative,
        JumpRelative,
    }

    internal interface ILabelInstruction : IInstruction
    {
        LabelInstructionType Type { get; }
        ILabel Label { get; }
    }

    internal class LabelInstruction : AbstractInstruction, ILabelInstruction
    {
        private ILabel _label;
        public LabelInstructionType Type { get; }

        public ILabel Label {
            get { return _label; }
            set {
                if (_label == value) return;
                _label = value;
                FuncBinding.Create(_label?.MemoryAddress, MemoryAddress, AddressToSize, Size);
            }
        }

        public LabelInstruction(LabelInstructionType type) {
            Type = type;
        }

        private ulong? AddressToSize(ulong? targetAddress, ulong? sourceAddress) {
            if (targetAddress.HasValue && sourceAddress.HasValue) {
                var relative = (long)targetAddress - (long)sourceAddress;
                switch (Type) {
                case LabelInstructionType.CallRelative:
                    if (relative >= 4 + short.MinValue && relative <= 4 + short.MaxValue) {
                        return 4;
                    }
                    else {
                        return 5;
                    }

                case LabelInstructionType.JumpRelative:
                    if (relative >= 2 + sbyte.MinValue && relative <= 2 + sbyte.MaxValue) {
                        return 2;
                    }
                    else if (relative >= 4 + short.MinValue && relative <= 4 + short.MaxValue) {
                        return 4;
                    }
                    else {
                        return 5;
                    }
                }
            }
            return null;
        }

        public override InstructionFlags Flags => 0;

        public override void Write(BinaryWriter binaryWriter) {
            var target = _label.MemoryAddress.Value.Value;
            var relative = (long)target - (long)MemoryAddress.Value.Value;
            switch (Type) {
            case LabelInstructionType.CallRelative:
                if (relative >= 4 + short.MinValue && relative <= 4 + short.MaxValue) {
                    var relativeData = BitConverter.GetBytes((short)(relative - 4));
                    binaryWriter.Write(new byte[] { 0x66, 0xe8, relativeData[0], relativeData[1] });
                }
                else {
                    var relativeData = BitConverter.GetBytes((int)(relative - 5));
                    binaryWriter.Write(new byte[] { 0xe8, relativeData[0], relativeData[1], relativeData[2], relativeData[3] });
                }
                return;

            case LabelInstructionType.JumpRelative:
                if (relative >= 2 + sbyte.MinValue && relative <= 2 + sbyte.MaxValue) {
                    binaryWriter.Write(new byte[] { 0xeb, (byte)(relative - 2) });
                }
                else if (relative >= 4 + short.MinValue && relative <= 4 + short.MaxValue) {
                    var relativeData = BitConverter.GetBytes((short)(relative - 4));
                    binaryWriter.Write(new byte[] { 0x66, 0xe9, relativeData[0], relativeData[1] });
                }
                else {
                    var relativeData = BitConverter.GetBytes((int)(relative - 5));
                    binaryWriter.Write(new byte[] { 0xe9, relativeData[0], relativeData[1], relativeData[2], relativeData[3] });
                }
                return;

            }
        }
    }
}
