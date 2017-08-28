using REC.Packaging.Code;
using System;
using System.IO;

namespace REC.Packaging.x86
{
    internal enum ImmediateInstructionType
    {
        Push,
        CallRelative, // can be used to push address of next instruction to stack (CallRelative 0)
    }
    internal interface IImmediateInstruction : IInstruction
    {
        ImmediateInstructionType Type { get; }
        INativeValue Immediate { get; }
    }

    internal class ImmediateInstruction : AbstractInstruction, IImmediateInstruction
    {
        public ImmediateInstructionType Type { get; }
        public INativeValue Immediate { get; }

        public override InstructionFlags Flags => InstructionFlags.Fixed;

        public ImmediateInstruction(ImmediateInstructionType type, INativeValue immediate) {
            Type = type;
            Immediate = immediate;
            UpdateSize();
        }

        private void UpdateSize() {
            switch (Type) {
            case ImmediateInstructionType.Push:
                switch (Immediate.Type) {
                case NativeTypes.Byte: Size.SetValue(2); return;
                case NativeTypes.Word: Size.SetValue(4); return;
                case NativeTypes.DWord: Size.SetValue(5); return;
                default: throw new InvalidOperationException("Invalid Immediate Type for Push");
                }

            case ImmediateInstructionType.CallRelative:
                switch (Immediate.Type) {
                case NativeTypes.DWord: Size.SetValue(5); return;
                default: throw new InvalidOperationException("Invalid Immediate Type for CallRelative");
                }
            default: throw new InvalidOperationException("Invalid Type");
            }
        }

        public override void Write(BinaryWriter binaryWriter) {
            switch (Type) {
            case ImmediateInstructionType.Push:
                switch (Immediate.Type) {
                case NativeTypes.Byte: binaryWriter.Write(new byte[] { 0x6a, Immediate.Data[0] }); return;
                case NativeTypes.Word: binaryWriter.Write(new byte[] { 0x66, 0x68, Immediate.Data[0], Immediate.Data[1] }); return;
                case NativeTypes.DWord: binaryWriter.Write(new byte[] { 0x68, Immediate.Data[0], Immediate.Data[1], Immediate.Data[2], Immediate.Data[3] }); return;
                default: throw new InvalidOperationException("Invalid Type");
                }

            case ImmediateInstructionType.CallRelative:
                binaryWriter.Write(new byte[] { 0xe8, Immediate.Data[0], Immediate.Data[1], Immediate.Data[2], Immediate.Data[3] });
                return;

            default: throw new InvalidOperationException("Invalid Type");
            }
        }
    }
}
