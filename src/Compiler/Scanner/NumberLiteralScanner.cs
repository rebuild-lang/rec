using REC.Tools;

namespace REC.Scanner
{
    using INumberLiteral = AST.INumberLiteral;
    using NumberLiteral = AST.NumberLiteral;

    public static class NumberLiteralScanner
    {
        public static INumberLiteral Scan(TextInputRange input) {
            var chr = input.EndChar;
            if (!IsDecimalDigit(chr)) return null;

            if (chr == '0') {
                var next = input.PeekChar();
                switch (next) {
                    case 'x':
                    case 'X':
                        input.Extend(2);
                        return ScanNumber(input,16,IsHexDigit);
                    case 'o':
                    case 'O':
                        input.Extend(2);
                        return ScanNumber(input,8,IsOctalDigit);
                    case 'b':
                    case 'B':
                        input.Extend(2);
                        return ScanNumber(input,2,IsBinaryDigit);
                }
            }
            return ScanDecimalNumber(input);
        }

        private static NumberLiteral ScanDecimalNumber(TextInputRange input) {
            var result = new NumberLiteral {BaseRadix = 10};
            var chr = input.EndChar;
            while (IsDecimalDigit(chr)) {
                if (result.IntegerPart == null) result.IntegerPart = "";
                if (!IsZero(chr) || !result.IntegerPart.IsEmpty()) {
                    result.IntegerPart += chr;
                }
                do {
                    input.Extend();
                    chr = input.EndChar;
                } while (IsIgnored(chr));
            }
            if (IsDot(chr)) {
                result.FractionalPart = "";
                input.Extend();
                chr = input.EndChar;
                while (IsDecimalDigit(chr)) {
                    result.FractionalPart += chr;
                    do {
                        input.Extend();
                        chr = input.EndChar;
                    } while (IsIgnored(chr));
                }
            }
            if (IsE(chr)) {
                input.Extend();
                chr = input.EndChar;
                if (IsSign(chr)) {
                    result.IsExponentPositive = IsPlus(chr);
                    input.Extend();
                    chr = input.EndChar;
                }
                while (IsDecimalDigit(chr)) {
                    if (result.ExponentPart == null) result.ExponentPart = "";
                    if (!IsZero(chr) || !result.ExponentPart.IsEmpty()) {
                        result.ExponentPart += chr;
                    }
                    do {
                        input.Extend();
                        chr = input.EndChar;
                    } while (IsIgnored(chr));
                }
                if (result.ExponentPart == null) return null; // if exponent started it has to contain value
            }
            return result.IsValid ? result : null;
        }

        private static NumberLiteral ScanNumber(TextInputRange input, int radix, System.Func<char,bool> isBase) {
            var result = new NumberLiteral {BaseRadix = radix};
            var chr = input.EndChar;
            while (isBase(chr)) {
                if (result.IntegerPart == null) result.IntegerPart = "";
                if (!IsZero(chr) || !result.IntegerPart.IsEmpty()) {
                    result.IntegerPart += chr;
                }
                do {
                    input.Extend();
                    chr = input.EndChar;
                } while (IsIgnored(chr));
            }
            if (IsDot(chr)) {
                result.FractionalPart = "";
                input.Extend();
                chr = input.EndChar;
                while (isBase(chr)) {
                    result.FractionalPart += chr;
                    do {
                        input.Extend();
                        chr = input.EndChar;
                    } while (IsIgnored(chr));
                }
            }
            if (IsP(chr)) {
                input.Extend();
                chr = input.EndChar;
                if (IsSign(chr)) {
                    result.IsExponentPositive = IsPlus(chr);
                    input.Extend();
                    chr = input.EndChar;
                }
                while (IsDecimalDigit(chr)) {
                    if (result.ExponentPart == null) result.ExponentPart = "";
                    if (!IsZero(chr) || !result.ExponentPart.IsEmpty()) {
                        result.ExponentPart += chr;
                    }
                    do {
                        input.Extend();
                        chr = input.EndChar;
                    } while (IsIgnored(chr));
                }
                if (result.ExponentPart == null) return null; // if exponent started it has to contain value
            }
            return result.IsValid ? result : null;
        }

        private static bool IsIgnored(char chr) => chr == '\'';

        private static bool IsPlus(char chr) => (chr == '+');

        private static bool IsSign(char chr) => chr == '+' || chr == '-';

        private static bool IsP(char chr) => chr == 'p' || chr == 'P';

        private static bool IsE(char chr) => chr == 'e' || chr == 'E';

        private static bool IsDot(char chr) => chr == '.';

        private static bool IsZero(char chr) => chr == '0';

        private static bool IsBinaryDigit(char chr) => (chr == '0' || chr == '1');

        private static bool IsOctalDigit(char chr) => (chr >= '0' && chr <= '7');

        private static bool IsDecimalDigit(char chr) => (chr >= '0' && chr <= '9');

        private static bool IsHexDigit(char chr) => (chr >= '0' && chr <= '9') || (chr >= 'a' && chr <= 'f') || (chr >= 'A' && chr <= 'F');
    }
}