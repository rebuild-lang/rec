using REC.Packaging.Image;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace REC.Packaging.Data
{
    internal interface IData : IMemoryPart, IEnumerable<IDataEntry>
    {
        IEnumerable<IDataEntry> Entries { get; }
        IBindProvider<ulong> BaseAddress { get; }

        void Add(IDataEntry entry);

        IEnumerable<ulong> RelocationAddresses();
    }

    internal class Data : AbstractMemoryPart, IData
    {
        IList<IDataEntry> Entries { get; } = new List<IDataEntry>();
        IEnumerable<IDataEntry> IData.Entries => Entries;

        public IBindProvider<ulong> BaseAddress { get; } = new BindProvider<ulong>();

        public override void Write(BinaryWriter bw) {
            foreach (var inst in Entries) inst.Write(bw);
        }

        public IEnumerable<ulong> RelocationAddresses() {
            foreach (var inst in Entries) if (inst.RelocationAddress != null) yield return inst.RelocationAddress.Value;
        }

        public void Add(IDataEntry entry) {
            if (Entries.Count == 0) {
                SumBinding.Create(new[] { MemoryOffset, BaseAddress }, entry.MemoryAddress);
            }
            else {
                var previous = Entries.Last();
                SumBinding.Create(new[] { previous.MemoryAddress, previous.Size }, entry.MemoryAddress);
            }
            Entries.Add(entry);
            SumBinding.Create(Entries.Select(i => i.Size).ToArray(), Size);
        }

        public IEnumerator<IDataEntry> GetEnumerator() => throw new System.NotImplementedException();
        IEnumerator IEnumerable.GetEnumerator() => throw new System.NotImplementedException();
    }
}
