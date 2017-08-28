using REC.Packaging.Image;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Collections;

namespace REC.Packaging.Code
{
    internal interface ICode : IMemoryPart, IEnumerable<IInstruction>
    {
        IEnumerable<IInstruction> Instructions { get; }
        IBindProvider<ulong> BaseAddress { get; }

        void Add(IInstruction instruction);

        IEnumerable<ulong> RelocationAddresses();
    }

    internal class Code : AbstractMemoryPart, ICode
    {
        IList<IInstruction> Instructions { get; } = new List<IInstruction>();
        IEnumerable<IInstruction> ICode.Instructions => Instructions;

        public IBindProvider<ulong> BaseAddress { get; } = new BindProvider<ulong>();

        public override void Write(BinaryWriter bw) {
            foreach (var inst in Instructions) inst.Write(bw);
        }

        public IEnumerable<ulong> RelocationAddresses() {
            foreach (var inst in Instructions) if (inst.RelocationAddress != null) yield return inst.RelocationAddress.Value;
        }

        public void Add(IInstruction instruction) {
            if (Instructions.Count == 0) {
                SumBinding.Create(new[] { MemoryOffset, BaseAddress }, instruction.MemoryAddress);
            }
            else {
                var previous = Instructions.Last();
                SumBinding.Create(new[] { previous.MemoryAddress, previous.Size }, instruction.MemoryAddress);
            }
            Instructions.Add(instruction);
            SumBinding.Create(Instructions.Select(i => i.Size).ToArray(), Size);
        }

        public IEnumerator<IInstruction> GetEnumerator() => throw new System.NotImplementedException();
        IEnumerator IEnumerable.GetEnumerator() => throw new System.NotImplementedException();
    }
}
