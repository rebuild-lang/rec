using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace REC.Packaging.Code
{
    internal interface ICode : ISizeProvider
    {
        List<IInstruction> Instructions { get; }
        ulong BaseAddress { get; }

        IEnumerable<ulong> RelocationAddresses();
        void Write(BinaryWriter bw);
    }

    internal class Code : ICode
    {
        public event SizeChangedHandler SizeChanged;
        public ulong Size { get; private set; }
 
        public List<IInstruction> Instructions { get; private set; }
        public ulong BaseAddress { get; private set; }

        public Code(IEnumerable<IInstruction> instructions, ulong baseAddress) 
        {
            Instructions = instructions.ToList();
            BaseAddress = baseAddress;
            Linearize();
        }

        private void Linearize() {
            Size = 0;
            var index = 0;
            foreach (var inst in Instructions) {
                inst.SetAddress(BaseAddress + Size);
                var instSize = inst.Size;
                inst.SizeChanged += (s, oldSize) => {
                    OnInstructionSizeChange(inst, oldSize, index);
                };
                Size += instSize;
            }
        }

        private void OnInstructionSizeChange(IInstruction inst, ulong oldSize, int index) {
            var delta = inst.Size - oldSize;
            Size += delta;
            foreach (var follow in Instructions.Skip(1 + index)) {
                if (!follow.IsAddressValid) break;
                follow.SetAddress(follow.Address + delta);
            }
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
