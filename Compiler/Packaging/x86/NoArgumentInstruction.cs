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
        public NoArgumentInstructionType Type { get; set; }

        public override bool IsValid => true;

        protected override void Encode() {
            if (!IsValid) return;
            switch (Type) {
            case NoArgumentInstructionType.NoOp:
                Encoded = new byte[] { 0x90 }; return;

            case NoArgumentInstructionType.Return:
                Encoded = new byte[] { 0xC3 }; return;

            case NoArgumentInstructionType.PushFlags:
                Encoded = new byte[] { 0x9C }; return;

            case NoArgumentInstructionType.PopFlags:
                Encoded = new byte[] { 0x9D }; return;

            case NoArgumentInstructionType.PushAll:
                Encoded = new byte[] { 0x60 }; return;

            case NoArgumentInstructionType.PopAll:
                Encoded = new byte[] { 0x61 }; return;
            }
        }
    }
}
