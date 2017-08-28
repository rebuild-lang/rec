using REC.Packaging.Code;
using REC.Packaging.Image;
using System;
using System.IO;

namespace REC.Packaging.x86
{
    internal enum DllEntryInstructionType
    {
        Call,
    }
    internal interface IDllEntryInstruction : IInstruction
    {
        IImportDllEntry DllEntry { get; }
    }

    internal class DllEntryInstruction : AbstractInstruction, IDllEntryInstruction
    {
        public IImportDllEntry DllEntry { get; set; }

        public override ulong? RelocationAddress => MemoryAddress.Value + 2;

        public override InstructionFlags Flags => InstructionFlags.Fixed;

        public DllEntryInstruction() {
            Size.SetValue(6);
        }

        public override void Write(BinaryWriter binaryWriter) {
            var dllEntryAddress = DllEntry.MemoryAddress.Value.Value;
            var addressBytes = BitConverter.GetBytes(dllEntryAddress);
            binaryWriter.Write(new byte[] { 0xff, 0x15, addressBytes[0], addressBytes[1], addressBytes[2], addressBytes[3] });
        }
    }
}
