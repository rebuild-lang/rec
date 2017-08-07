using System;
using System.IO;

namespace REC.Packaging.Code
{
    internal interface IInstruction : IAddressProvider, ISizeProvider
    {
        bool IsValid { get; }
        ulong? RelocationAddress { get; }

        void SetAddress(ulong address);
        void Write(BinaryWriter bw);
    }

    internal abstract class AbstractInstruction : AbstractAddressProvider, IInstruction
    {
        private byte[] _encoded;

        protected byte[] Encoded {
            get => _encoded;
            set {
                if (_encoded == value) return;
                var oldSize = CurrentSize();
                _encoded = value;
                var newSize = CurrentSize();
                if (oldSize != newSize) SizeChanged?.Invoke(this, oldSize);
            }
        }

        public abstract bool IsValid { get; }

        private ulong CurrentSize() => (ulong?) Encoded?.LongLength ?? 0;
        public ulong Size {
            get {
                if (Encoded == null) Encode();
                return CurrentSize();
            }
        }

        public virtual ulong? RelocationAddress => null;
        public event SizeChangedHandler SizeChanged;

        protected abstract void Encode();

        public void SetAddress(ulong address) { Address = address; }

        public void Write(BinaryWriter bw) {
            if (Encoded == null) Encode();
            if (Encoded == null) throw new InvalidOperationException("invalid encoded instruction");
            bw.Write(Encoded);
        }
    }
}
