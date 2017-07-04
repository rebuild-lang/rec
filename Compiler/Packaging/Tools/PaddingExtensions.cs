using System.IO;

namespace REC.Packaging.Tools
{
    internal static class PaddingExtensions
    {
        public static uint Padding(this uint value, uint alignment)
        {
            return (alignment - (value % alignment)) % alignment;
        }

        public static uint AlignTo(this uint value, uint alignment)
        {
            return value + Padding(value, alignment);
        }

        public static byte[] PaddingBytes(this uint value, uint alignment)
        {
            return new byte[Padding(value, alignment)];
        }

        public static void PadPosition(this BinaryWriter bw, uint alignment)
        {
            var position = (uint)bw.BaseStream.Position;
            var bytes = PaddingBytes(position, alignment);
            bw.Write(bytes);
        }
    }
}
