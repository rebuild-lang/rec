using System;
using System.Collections.Generic;
using System.Text;
using REC.Tools;

namespace REC.AST
{
    using Pair = KeyValuePair<int, int>; // bits, radix
    using Dict = Dictionary<KeyValuePair<int, int>, string>;

    // collection of data derived from parsing any number literal token
    // the processing of the number value is deferred until we know a format to encode the value to
    public interface INumberLiteral : ILiteral
    {
        int Radix { get; } // 1, 8, 10, 16
        string IntegerPart { get; }
        string FractionalPart { get; }
        bool IsExponentPositive { get; }
        string ExponentPart { get; }

        bool IsValid { get; }
        bool IsFloat { get; }
        bool IsInteger { get; }

        bool FitsUnsigned(int byteCount);
        bool FitsPositive(int byteCount);
        bool FitsNegative(int byteCount);
        bool FitsFloat(int byteCount);

        byte[] ToUnsigned(int byteCount);
        byte[] ToSigned(int byteCount);
        byte[] ToFloat(int byteCount);
    }

    class NumberLiteral : Literal, INumberLiteral
    {
        internal static readonly Dict MaxStringDict = CreateDict(maxBits: 128);
        public int Radix { get; set; } // 1, 8, 10, 16
        public string IntegerPart { get; set; }
        public string FractionalPart { get; set; }
        public bool IsExponentPositive { get; set; } = true;
        public string ExponentPart { get; set; }

        public bool IsValid => IntegerPart != null || FractionalPart != null || ExponentPart != null;
        public bool IsFloat => FractionalPart != null || ExponentPart != null;
        public bool IsInteger => IntegerPart != null && FractionalPart == null && ExponentPart == null;

        public bool FitsUnsigned(int byteCount) => IsInteger && IsFitsBits(IntegerPart, Radix, byteCount * 8);
        public bool FitsPositive(int byteCount) => IsInteger && IsFitsBits(IntegerPart, Radix, byteCount * 8 - 1);
        public bool FitsNegative(int byteCount) => IsInteger && IsFitsNegativeBits(IntegerPart, Radix, byteCount * 8 - 1);
        public bool FitsFloat(int byteCount) => false;

        public byte[] ToUnsigned(int byteCount) {
            return BitConverter.GetBytes(Convert.ToUInt64(IntegerPart));
        }

        public byte[] ToSigned(int byteCount) => new byte[byteCount];
        public byte[] ToFloat(int byteCount) => new byte[byteCount];

        static bool IsFitsBits(string number, int radix, int bits) {
            var maxString = MaxBitsString(bits, radix);
            return number.Length < maxString.Length
                || number.Length == maxString.Length && string.Compare(number, maxString, StringComparison.CurrentCultureIgnoreCase) < 0;
        }

        static bool IsFitsNegativeBits(string number, int radix, int bits) {
            var maxString = MaxBitsString(bits, radix);
            return number.Length < maxString.Length
                || number.Length == maxString.Length && string.Compare(number, maxString, StringComparison.CurrentCultureIgnoreCase) <= 0;
        }

        static string MaxBitsString(int bits, int radix) {
            return MaxStringDict[new Pair(bits, radix)];
        }

        static Dict CreateDict(int maxBits) {
            var result = new Dict();
            FillHexStrings(result, maxBits);
            FillDecimalStrings(result, maxBits);
            return result;
        }

        static void FillHexStrings(Dict dict, int maxBits) {
            var now = new StringBuilder(maxBits / 4);
            var flow = new[] {'2', '4', '8', '0'};
            for (var g = 0; g < maxBits; g += 4) {
                now.Insert(index: 0, value: '1');
                for (var i = 0; i < 4 && g + i < maxBits; i++) {
                    dict[new Pair(g + i, value: 16)] = now.ToString();
                    now[index: 0] = flow[i];
                }
            }
        }

        static void FillDecimalStrings(Dict dict, int maxBits) {
            var decimalStr = "1";
            for (var i = 0; i < maxBits; i++) {
                dict[new Pair(i, value: 10)] = decimalStr;
                decimalStr = decimalStr.DecimalAdd(decimalStr);
            }
        }
    }
}
