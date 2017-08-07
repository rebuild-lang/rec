namespace REC.Packaging.x86
{
    internal enum NativeTypes : byte
    {
        Byte, Word, DWord, QWord,
        Byte2nd, // 2nd lowest byte (only R0...R3)

        // SIMD Vectors
        Byte8, Byte16, Byte32,
        Word4, Word8, Word16, Word32,
        DWord4, DWord8, DWord16,
        QWord2, QWord4, QWord8,
        Float, Float2, Float4, Float8, Float16,
        Double, Double2, Double4, Double8,
    }

    internal static class NativeTypeExtensions
    {
        internal static int ByteCount(this NativeTypes type) {
            switch (type) {
            case NativeTypes.Byte: return 1;
            case NativeTypes.Word: return 2;
            case NativeTypes.DWord: return 4;
            case NativeTypes.QWord: return 8;
            case NativeTypes.Byte2nd: return 1;
            case NativeTypes.Byte8: return 8;
            case NativeTypes.Byte16: return 16;
            case NativeTypes.Byte32: return 32;
            case NativeTypes.Word4: return 2 * 4;
            case NativeTypes.Word8: return 2 * 8;
            case NativeTypes.Word16: return 2 * 16;
            case NativeTypes.Word32: return 2 * 32;
            case NativeTypes.DWord4: return 4 * 4;
            case NativeTypes.DWord8: return 4 * 8;
            case NativeTypes.DWord16: return 4 * 16;
            case NativeTypes.QWord2: return 8 * 2;
            case NativeTypes.QWord4: return 8 * 4;
            case NativeTypes.QWord8: return 8 * 8;
            case NativeTypes.Float: return 4;
            case NativeTypes.Float2: return 4 * 2;
            case NativeTypes.Float4: return 4 * 4;
            case NativeTypes.Float8: return 4 * 8;
            case NativeTypes.Float16: return 4 * 16;
            case NativeTypes.Double: return 8;
            case NativeTypes.Double2: return 8 * 2;
            case NativeTypes.Double4: return 8 * 4;
            case NativeTypes.Double8: return 8 * 8;
            default: return 0;
            }
        }
    }
}
