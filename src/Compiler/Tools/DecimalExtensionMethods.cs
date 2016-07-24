using System;
using System.Text;

namespace REC.Tools
{
    // Extends string to allow endless decimal number calculations
    public static class DecimalExtensionMethods
    {
        private const char Zero = '0';
        private const char One = '1';

        public static string DecimalAdd(this string a, string b) {
            var result = new StringBuilder(Math.Max(a.Length + 1, b.Length + 1));
            var aI = a.Length - 1;
            var bI = a.Length - 1;
            var overflow = 0;
            while (aI >= 0 || bI >= 0) {
                var aC = aI >= 0 ? a[aI] - Zero : 0;
                var bC = bI >= 0 ? b[bI] - Zero : 0;
                var rC = aC + bC + overflow;
                if (rC > 9) {
                    overflow = 1;
                    rC -= 10;
                }
                else overflow = 0;
                result.Insert(0, (char)(Zero + rC));
                aI--;
                bI--;
            }
            if (1 == overflow) result.Insert(0, One);
            return result.ToString();
        }

        public static string DecimalDecrement(this string str) {
            var result = new StringBuilder(str);
            var index = str.Length - 1;
            while (index >= 0) {
                var aC = index >= 0 ? str[index] : 0;
                var rC = aC - 1;
                if (rC < Zero) {
                    rC += 10;
                    result[index] = (char)rC;
                    index--;
                    continue;
                }
                result[index] = (char)rC;
                break;
            }
            if (index < 0) throw new IndexOutOfRangeException("below 0");
            if (index == 0) {
                while (result[0] == Zero) result.Remove(0, 1);
            }
            return result.ToString();
        }
    }
}