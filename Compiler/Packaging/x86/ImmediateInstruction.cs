using REC.Packaging.Code;
using System;

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
        public ImmediateInstructionType Type { get; set; }

        public INativeValue Immediate { get; set; }

        public override bool IsValid => Immediate != null && Immediate.IsValid && IsImmediateTypeSupported();

        protected override void Encode() {
            if (!IsValid) return;
            switch (Type) {
            case ImmediateInstructionType.Push:
                switch (Immediate.Type) {
                case NativeTypes.Byte: Encoded = new byte[] { 0x6a, Immediate.Data[0] }; return;
                case NativeTypes.Word: Encoded = new byte[] { 0x66, 0x68, Immediate.Data[0], Immediate.Data[1] }; return;
                case NativeTypes.DWord: Encoded = new byte[] { 0x68, Immediate.Data[0], Immediate.Data[1], Immediate.Data[2], Immediate.Data[3] }; return;
                }
                break;

            case ImmediateInstructionType.CallRelative:
                Encoded = new byte[] { 0xe8, Immediate.Data[0], Immediate.Data[1], Immediate.Data[2], Immediate.Data[3] };
                break;
            }
        }

        private bool IsImmediateTypeSupported() {
            switch (Type) {
            case ImmediateInstructionType.Push:
                switch (Immediate.Type) {
                case NativeTypes.Byte:
                case NativeTypes.Word:
                case NativeTypes.DWord:
                    return true;
                default: return false;
                }

            case ImmediateInstructionType.CallRelative:
                switch (Immediate.Type) {
                case NativeTypes.DWord:
                    return true;
                default: return false;
                }
            default: return false;
            }
        }
    }
}
