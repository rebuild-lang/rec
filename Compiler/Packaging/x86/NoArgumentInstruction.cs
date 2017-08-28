using System.IO;
using REC.Packaging.Code;

namespace REC.Packaging.x86
{
    internal enum NoArgumentInstructionType
    {
        NoOp,
        Return,
        PushFlags,
        PopFlags,
        PushAll,
        PopAll,
    }

    internal interface INoArgumentInstruction : IInstruction
    {
        NoArgumentInstructionType Type { get; }
    }

    internal class NoArgumentInstruction : AbstractInstruction, INoArgumentInstruction
    {
        public NoArgumentInstructionType Type { get; }

        public override InstructionFlags Flags => InstructionFlags.Fixed;

        NoArgumentInstruction(NoArgumentInstructionType type) {
            Type = type;
            UpdateSize();
        }

        private void UpdateSize() {
            Size.SetValue(1);
        }

        public override void Write(BinaryWriter binaryWriter) {
            switch (Type) {
            case NoArgumentInstructionType.NoOp:
                binaryWriter.Write(new byte[] { 0x90 }); return;

            case NoArgumentInstructionType.Return:
                binaryWriter.Write(new byte[] { 0xC3 }); return;

            case NoArgumentInstructionType.PushFlags:
                binaryWriter.Write(new byte[] { 0x9C }); return;

            case NoArgumentInstructionType.PopFlags:
                binaryWriter.Write(new byte[] { 0x9D }); return;

            case NoArgumentInstructionType.PushAll:
                binaryWriter.Write(new byte[] { 0x60 }); return;

            case NoArgumentInstructionType.PopAll:
                binaryWriter.Write(new byte[] { 0x61 }); return;
            }
        }
    }
}
