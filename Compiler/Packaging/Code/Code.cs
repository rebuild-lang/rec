using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace REC.Packaging.Code
{
    internal interface ICode : ISizeProvider
    {
        IEnumerable<IInstruction> Instructions { get; }
        ulong BaseAddress { get; }

        IEnumerable<ulong> RelocationAddresses();
        void Write(BinaryWriter bw);
    }

    internal class Code : ICode
    {
        public event SizeChangedHandler SizeChanged;
        public ulong Size { get; private set; }
 
        public IEnumerable<IInstruction> Instructions { get; private set; }
        public ulong BaseAddress { get; private set; }

        public Code(IEnumerable<IInstruction> instructions, ulong baseAddress) 
        {
            Instructions = instructions.ToArray();
            BaseAddress = baseAddress;
            Linearize();
        }

        private void Linearize() {
            foreach(var inst in Instructions) {
                inst.SetAddress(BaseAddress + Size);
                var instSize = inst.Size;
                inst.SizeChanged += OnInstructionSizeChange;
                Size += instSize;
            }
        }

        private void OnInstructionSizeChange(ISizeProvider sp, ulong oldSize) {
            var delta = sp.Size - oldSize;
            Size += delta;
            SizeChanged?.Invoke(this, Size - delta);
        }

        public void Write(BinaryWriter bw) {
            foreach (var inst in Instructions) inst.Write(bw);
        }

        public IEnumerable<ulong> RelocationAddresses() {
            foreach (var inst in Instructions) if (inst.RelocationAddress != null) yield return inst.RelocationAddress.Value;
        }
    }
}
