using REC.Packaging.Code;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace REC.Packaging.Data
{
    internal interface IInitializedDataEntry : IAddressProvider
    {
        ulong Size { get; }
        byte[] Bytes { get; }
        void SetAddress(ulong address);
    }

    internal interface IInitializedData
    {
        ulong BaseAddress { get; }
        ulong Size { get; }
        IEnumerable<IInitializedDataEntry> Entries { get; }

        void Write(BinaryWriter bw);
    }

    internal class InitializedDataEntry : AbstractAddressProvider, IInitializedDataEntry
    {
        public ulong Size => (ulong?)Bytes?.Length ?? 0;
        public byte[] Bytes { get; set; }
        public void SetAddress(ulong address) { Address = address; }
    }

    internal class InitializedData : IInitializedData
    {
        public ulong BaseAddress { get; private set; }
        public ulong Size { get; private set; }
        public IEnumerable<IInitializedDataEntry> Entries { get; private set; }

        public InitializedData(IEnumerable<IInitializedDataEntry> entries, ulong baseAddress) {
            Entries = entries.ToArray();
            BaseAddress = baseAddress;
            Linearize();
        }

        private void Linearize() {
            foreach (var entry in Entries) {
                entry.SetAddress(BaseAddress + Size);
                Size += entry.Size;
            }
        }

        public void Write(BinaryWriter bw) {
            foreach (var entry in Entries) {
                bw.Write(entry.Bytes);
            }
        }
    }
}
