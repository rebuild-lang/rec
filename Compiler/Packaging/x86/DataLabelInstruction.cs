using REC.Packaging.Code;
using REC.Packaging.Data;
using REC.Packaging.Image;
using System;
using System.IO;

namespace REC.Packaging.x86
{
    internal enum DataLabelInstructionType
    {
        PushAddress,
    }

    internal interface IDataLabelInstruction : IInstruction
    {
        DataLabelInstructionType Type { get; }
        IDataLabel Label { get; }
    }

    internal class DataLabelInstruction : AbstractInstruction, IDataLabelInstruction
    {
        private IDataLabel _label;
        public DataLabelInstructionType Type { get; }

        public IDataLabel Label {
            get { return _label; }
            set {
                if (_label == value) return;
                _label = value;
                FuncBinding.Create(_label?.MemoryAddress, MemoryAddress, AddressToSize, Size);
            }
        }

        public DataLabelInstruction(DataLabelInstructionType type) {
            Type = type;
        }

        private ulong? AddressToSize(ulong? targetAddress, ulong? sourceAddress) {
            if (targetAddress.HasValue && sourceAddress.HasValue) {
                var relative = (long)targetAddress - (long)sourceAddress;
                switch (Type) {
                case DataLabelInstructionType.PushAddress: return 5;
                }
            }
            return null;
        }

        public override InstructionFlags Flags => 0;

        public override void Write(BinaryWriter binaryWriter) {
            var target = _label.MemoryAddress.Value.Value;
            switch (Type) {
            case DataLabelInstructionType.PushAddress:
                var addressData = BitConverter.GetBytes(target);
                binaryWriter.Write(new byte[] { 0x68, addressData[0], addressData[1], addressData[2], addressData[3] });
                return;
            }
        }

        public override ulong? RelocationAddress {
            get {
                if (Type == DataLabelInstructionType.PushAddress)
                    return MemoryAddress.Value + 1;
                return null;
            }
        }
    }
}
