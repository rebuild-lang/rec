using REC.Packaging.Code;
using System.Collections.Generic;
using System.Linq;

namespace REC.Packaging.Data
{
    internal interface IUninitializedDataEntry : IAddressProvider
    {
        ulong Size { get; }
        void SetAddress(ulong address);
    }
    internal interface IUninitializedData
    {
        ulong BaseAddress { get; }
        ulong Size { get; }
        IEnumerable<IUninitializedDataEntry> Entries { get; }
    }

    internal class UninitializedDataEntry : AbstractAddressProvider, IUninitializedDataEntry
    {
        public ulong Size { get; set; }
        public void SetAddress(ulong address) { Address = address; }
    }

    internal class UninitializedData : IUninitializedData
    {
        public ulong BaseAddress { get; private set; }
        public ulong Size { get; private set; }
        public IEnumerable<IUninitializedDataEntry> Entries { get; private set; }

        public UninitializedData(IEnumerable<IUninitializedDataEntry> entries, ulong baseAddress) {
            Entries = entries.ToArray();
            BaseAddress = baseAddress;
            Linearize();
        }

        private void Linearize() {
            foreach(var entry in Entries) {
                entry.SetAddress(BaseAddress + Size);
                Size += entry.Size;
            }
        }
    }
}
