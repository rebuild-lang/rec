using System;
using REC.Packaging.Code;

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

    internal class LabelInstruction : AbstractInstruction, ILabelInstruction, IAddressConsumer
    {
        private ILabel _label;
        public LabelInstructionType Type { get; set; }

        public ILabel Label {
            get { return _label; }
            set {
                if (_label == value) return;
                _label?.RemoveAddressConsumer(this);
                _label = value;
                _label?.AddAddressConsumer(this);
            }
        }

        public override bool IsValid => Label != null && Label.IsAddressValid;

        public void ConsumeAddress(ulong address) {
            Encode();
        }

        protected override void Encode() {
            if (!IsValid) return;
            var target = _label.Address;
            var relative = (long)target - (long)Address;
            switch (Type) {
            case LabelInstructionType.CallRelative:
                if (relative >= 4 + short.MinValue && relative <= 4 + short.MaxValue) {
                    var relativeData = BitConverter.GetBytes((short)(relative - 4));
                    Encoded = new byte[] { 0x66, 0xe8, relativeData[0], relativeData[1] };
                }
                else {
                    var relativeData = BitConverter.GetBytes((int)(relative - 5));
                    Encoded = new byte[] { 0xe8, relativeData[0], relativeData[1], relativeData[2], relativeData[3] };
                }
                return;

            case LabelInstructionType.JumpRelative:
                if (relative >= 2 + sbyte.MinValue && relative <= 2 + sbyte.MaxValue) {
                    Encoded = new byte[] { 0xeb, (byte)(relative - 2) };
                }
                else if (relative >= 4 + short.MinValue && relative <= 4 + short.MaxValue) {
                    var relativeData = BitConverter.GetBytes((short)(relative - 4));
                    Encoded = new byte[] { 0x66, 0xe9, relativeData[0], relativeData[1] };
                }
                else {
                    var relativeData = BitConverter.GetBytes((int)(relative - 5));
                    Encoded = new byte[] { 0xe9, relativeData[0], relativeData[1], relativeData[2], relativeData[3] };
                }
                return;

            }
        }
    }
}
