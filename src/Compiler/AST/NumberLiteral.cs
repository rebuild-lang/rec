using System;
using System.Collections.Generic;
using System.Text;
using REC.Tools;

namespace REC.AST
{
    using Pair = KeyValuePair<int, int>; // bits, radix
    using Dict = Dictionary<KeyValuePair<int, int>, string>;

    public interface INumberLiteral : ILiteral
    {
        int BaseRadix { get; } // 1, 8, 10, 16
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
    }

    internal class NumberLiteral : Literal, INumberLiteral
    {
        public int BaseRadix { get; set; } // 1, 8, 10, 16
        public string IntegerPart { get; set; }
        public string FractionalPart { get; set; }
        public bool IsExponentPositive { get; set; } = true;
        public string ExponentPart { get; set; }

        public bool IsValid => IntegerPart != null || FractionalPart != null || ExponentPart != null;
        public bool IsFloat => FractionalPart != null || ExponentPart != null;
        public bool IsInteger => IntegerPart != null && FractionalPart == null && ExponentPart == null;

        public bool FitsUnsigned(int byteCount) => IsInteger && IsFitsBits(IntegerPart, BaseRadix, byteCount * 8);
        public bool FitsPositive(int byteCount) => IsInteger && IsFitsBits(IntegerPart, BaseRadix, byteCount * 8 - 1);
        public bool FitsNegative(int byteCount) => IsInteger && IsFitsNegativeBits(IntegerPart, BaseRadix, byteCount * 8 - 1);
        public bool FitsFloat(int byteCount) => false;

        public byte[] ToUnsigned(int byteCount) => new byte[byteCount];
        public byte[] ToSigned(int byteCount) => new byte[byteCount];
        public byte[] ToFloat(int byteCount) => new byte[byteCount];

        private static bool IsFitsBits(string number, int radix, int bits) {
            var maxString = MaxBitsString(bits, radix);
            return number.Length < maxString.Length
                || (number.Length == maxString.Length && string.Compare(number, maxString, StringComparison.CurrentCultureIgnoreCase) < 0);
        }
        private static bool IsFitsNegativeBits(string number, int radix, int bits) {
            var maxString = MaxBitsString(bits, radix);
            return number.Length < maxString.Length
                || (number.Length == maxString.Length && string.Compare(number, maxString, StringComparison.CurrentCultureIgnoreCase) <= 0);
        }

        internal static readonly Dict MaxStringDict = CreateDict(128);

        private static string MaxBitsString(int bits, int radix) {
            return MaxStringDict[new Pair(bits, radix)];
        }

        private static Dict CreateDict(int maxBits) {
            var result = new Dict();
            FillHexStrings(result, maxBits);
            FillDecimalStrings(result, maxBits);
            return result;
        }

        private static void FillHexStrings(Dict dict, int maxBits) {
            var now = new StringBuilder(maxBits / 4);
            var flow = new[] { '2', '4', '8', '0' };
            for (var g = 0; g < maxBits; g += 4) {
                now.Insert(0, '1');
                for (var i = 0; i < 4 && g + i < maxBits; i++) {
                    dict[new Pair(g + i, 16)] = now.ToString();
                    now[0] = flow[i];
                }
            }
        }

        private static void FillDecimalStrings(Dict dict, int maxBits) {
            var decimalStr = "1";
            for (var i = 0; i < maxBits; i++) {
                dict[new Pair(i, 10)] = decimalStr;
                decimalStr = decimalStr.DecimalAdd(decimalStr);
            }
        }
    }
}