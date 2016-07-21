using System;
using System.Collections.Generic;
using System.Text;

namespace REC
{
    using Pair = KeyValuePair<int, int>; // bits, radix
    using Dict = Dictionary<KeyValuePair<int, int>, string>;

    public class NumberLiteral
    {
        public int BaseRadix; // 1, 8, 10, 16
        public string IntegerPart;
        public string FractionalPart;
        public bool IsExponentPositive = true;
        public string ExponentPart;

        public bool IsValid => IntegerPart != null || FractionalPart != null || ExponentPart != null;
        public bool IsFloat => FractionalPart != null || ExponentPart != null;
        public bool IsInteger => IntegerPart != null && FractionalPart == null && ExponentPart == null;

        public bool FitsUnsigned(int byteCount) => IsInteger && IsFitsBits(IntegerPart, BaseRadix, byteCount*8);
        public bool FitsPositive(int byteCount) => IsInteger && IsFitsBits(IntegerPart, BaseRadix, byteCount * 8 - 1);
        public bool FitsNegative(int byteCount) => IsInteger && IsFitsNegativeBits(IntegerPart, BaseRadix, byteCount * 8 - 1);
        public bool FitsFloat(int byteCount) => false;

        public byte[] ToUnsigned(int byteCount) => new byte[byteCount];
        public byte[] ToSigned(int byteCount) => new byte[byteCount];
        public byte[] ToFloat(int byteCount) => new byte[byteCount];

        private static bool IsFitsBits(string number, int radix, int bits)
        {
            var maxString = MaxBitsString(bits, radix);
            return number.Length < maxString.Length 
                || (number.Length == maxString.Length && string.Compare(number, maxString, StringComparison.InvariantCultureIgnoreCase) < 0);
        }
        private static bool IsFitsNegativeBits(string number, int radix, int bits) {
            var maxString = MaxBitsString(bits, radix);
            return number.Length < maxString.Length
                || (number.Length == maxString.Length && string.Compare(number, maxString, StringComparison.InvariantCultureIgnoreCase) <= 0);
        }

        private static readonly Dict _dict = createDict(128);

        private static string MaxBitsString(int bits, int radix)
        {
            return _dict[new Pair(bits, radix)];
        }

        private static Dict createDict(int maxBits)
        {
            var result = new Dict();
            FillHexStrings(result, maxBits);
            FillDecimalStrings(result, maxBits);
            return result;
        }

        private static void FillHexStrings(Dict dict, int maxBits) {
            var now = new StringBuilder(maxBits / 4);
            var flow = new char[] { '2', '4', '8', '0' };
            for (var g = 0; g < maxBits; g += 4) {
                now.Insert(0, '1');
                for (var i = 0; i < 4 && g + i < maxBits; i++) {
                    dict[new Pair(g + i, 16)] = now.ToString();
                    now[0] = flow[i];
                }
            }
        }

        private static void FillDecimalStrings(Dict dict, int maxBits)
        {
            var decimalStr = "1";
            for (var i = 0; i < maxBits; i++)
            {
                dict[new Pair(i, 10)] = decimalStr;
                decimalStr = DecimalAdd(decimalStr, decimalStr);
            }
        }

        private static string DecimalAdd(string a, string b) {
            var result = new StringBuilder(Math.Max(a.Length + 1, b.Length + 1));
            var aI = a.Length - 1;
            var bI = a.Length - 1;
            var overflow = 0;
            while (aI >= 0 || bI >= 0) {
                var aC = aI >= 0 ? a[aI] : 0;
                var bC = bI >= 0 ? b[bI] : 0;
                var rC = aC + bC - '0' + overflow;
                if (rC > '9') {
                    overflow = 1;
                    rC -= 10;
                }
                else overflow = 0;
                result.Insert(0, (char)rC);
                aI--;
                bI--;
            }
            if (1 == overflow) result.Insert(0, '1');
            return result.ToString();
        }

        private static string DecimalDecrement(string a) {
            var result = new StringBuilder(a);
            int aI = a.Length - 1;
            int underflow = 1;
            while (aI >= 0 && underflow == 1) {
                var aC = aI >= 0 ? a[aI] : 0;
                var rC = aC - underflow;
                if (rC < '0') {
                    rC += 10;
                    result[aI] = (char)rC;
                    aI--;
                    continue;
                }
                result[aI] = (char)rC;
                break;
            }
            if (aI < 0) throw new System.IndexOutOfRangeException("below 0");
            if (aI == 0) {
                while (result[0] == '0') result.Remove(0, 1);
            }
            return result.ToString();
        }
    }
}