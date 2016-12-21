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
                        return ScanNumber(input, radix: 16, isDigit: IsHexDigit);
                    case 'o':
                    case 'O':
                        input.Extend(2);
                        return ScanNumber(input, radix: 8, isDigit: IsOctalDigit);
                    case 'b':
                    case 'B':
                        input.Extend(2);
                        return ScanNumber(input, radix: 2, isDigit: IsBinaryDigit);
                }
            }
            return ScanDecimalNumber(input);
        }

        static NumberLiteral ScanDecimalNumber(TextInputRange input) {
            var result = new NumberLiteral {Radix = 10};
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

        static NumberLiteral ScanNumber(TextInputRange input, int radix, System.Func<char,bool> isDigit) {
            var result = new NumberLiteral {Radix = radix};
            var chr = input.EndChar;
            while (isDigit(chr)) {
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
                while (isDigit(chr)) {
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

        static bool IsIgnored(char chr) => chr == '\'';

        static bool IsPlus(char chr) => (chr == '+');

        static bool IsSign(char chr) => chr == '+' || chr == '-';

        static bool IsP(char chr) => chr == 'p' || chr == 'P';

        static bool IsE(char chr) => chr == 'e' || chr == 'E';

        static bool IsDot(char chr) => chr == '.';

        static bool IsZero(char chr) => chr == '0';

        static bool IsBinaryDigit(char chr) => (chr == '0' || chr == '1');

        static bool IsOctalDigit(char chr) => (chr >= '0' && chr <= '7');

        static bool IsDecimalDigit(char chr) => (chr >= '0' && chr <= '9');

        static bool IsHexDigit(char chr) => (chr >= '0' && chr <= '9') || (chr >= 'a' && chr <= 'f') || (chr >= 'A' && chr <= 'F');
    }
}