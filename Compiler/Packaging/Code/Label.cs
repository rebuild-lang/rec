using System;
using System.IO;

namespace REC.Packaging.Code
{
    internal interface ILabel : IInstruction
    {
        string Name { get; }
    }

    internal class Label : AbstractAddressProvider, ILabel
    {
        public string Name { get; set; }

        public bool IsValid => true;
        public ulong Size => 0;
        public ulong? RelocationAddress => null;

#pragma warning disable CS0067
        public event SizeChangedHandler SizeChanged;
#pragma warning restore
        public void SetAddress(ulong address) { Address = address; }
        public void Write(BinaryWriter bw) { }
    }
}
