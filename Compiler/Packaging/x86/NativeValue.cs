using System;

namespace REC.Packaging.x86
{
    internal interface INativeValue
    {
        NativeTypes Type { get; }
        byte[] Data { get; }
        bool IsValid { get; }
    }

    internal class NativeValue : INativeValue
    {
        public NativeTypes Type { get; set; }
        public byte[] Data { get; set; }

        public bool IsValid {
            get {
                return Data != null && Data.Length == Type.ByteCount();
            }
        }

        public static NativeValue Create(byte data) => new NativeValue { Type = NativeTypes.Byte, Data = new[] { data } };
        public static NativeValue Create(sbyte data) => new NativeValue { Type = NativeTypes.Byte, Data = new[] { (byte)data } };
        public static NativeValue Create(ushort data) => new NativeValue { Type = NativeTypes.Word, Data = BitConverter.GetBytes(data) };
        public static NativeValue Create(short data) => new NativeValue { Type = NativeTypes.Word, Data = BitConverter.GetBytes(data) };
        public static NativeValue Create(uint data) => new NativeValue { Type = NativeTypes.DWord, Data = BitConverter.GetBytes(data) };
        public static NativeValue Create(int data) => new NativeValue { Type = NativeTypes.DWord, Data = BitConverter.GetBytes(data) };
        public static NativeValue Create(ulong data) => new NativeValue { Type = NativeTypes.QWord, Data = BitConverter.GetBytes(data) };
        public static NativeValue Create(long data) => new NativeValue { Type = NativeTypes.QWord, Data = BitConverter.GetBytes(data) };
    }
}
