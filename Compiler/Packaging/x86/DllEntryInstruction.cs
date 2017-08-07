using REC.Packaging.Code;
using System;

namespace REC.Packaging.x86
{
    internal enum DllEntryInstructionType
    {
        Call,
    }
    internal interface IDllEntryInstruction : IInstruction, IAddressConsumer
    {
        IImportDllEntry DllEntry { get; }
    }

    internal class DllEntryInstruction : AbstractInstruction, IDllEntryInstruction
    {
        private IImportDllEntry _dllEntry;
        private ulong _dllEntryAddress = ulong.MaxValue;

        public override bool IsValid => DllEntry != null;
        public override ulong? RelocationAddress => Address + 2;

        public IImportDllEntry DllEntry {
            get => _dllEntry;
            set {
                if (_dllEntry == value) return;
                _dllEntry?.RemoveAddressConsumer(this);
                _dllEntry = value;
                _dllEntry?.AddAddressConsumer(this);
            }
        }

        public void ConsumeAddress(ulong address) {
            _dllEntryAddress = address;
            Encode();
        }

        protected override void Encode() {
            if (!IsValid) return;
            var addressBytes = BitConverter.GetBytes(_dllEntryAddress);
            Encoded = new byte[] { 0xff, 0x15, addressBytes[0], addressBytes[1], addressBytes[2], addressBytes[3] };
        }
    }
}
