using System.Text;
using REC.AST;

namespace REC.Scanner
{
    public static class StringLiteralScanner
    {
        // Scan basic double quoted strings
        public static IStringLiteral Scan(TextInputRange input) {
            var chr = input.EndChar;
            if (!IsDoubleQuote(chr)) return null;
            input.Extend();

            var result = new StringBuilder();
            var wsIndex = -1;
            while (true) {
                chr = input.EndChar;
                if (chr == '\0') return null; // file end or invalid input
                if (IsDoubleQuote(chr)) {
                    input.Extend();
                    if (result.Length == 0 && IsDoubleQuote(input.EndChar)) {
                        input.Extend(); // skip 3rd doublequote
                        return ScanUnescaped(result, input);
                    }
                    break;
                }
                if (input.IsEndNewline) {
                    input.NewLine();
                    if (wsIndex >= 0) result.Remove(wsIndex, result.Length - wsIndex);
                }
                else {
                    if (input.IsEndWhitespace) {
                        if (wsIndex < 0) wsIndex = result.Length;
                    }
                    else wsIndex = -1;
                    if (!IsTab(chr) && char.IsControl(chr)) HandleControl(input);
                    else {
                        if (IsBackslash(chr)) {
                            if (!HandleEscape(input, result)) return null;
                        }
                        else {
                            result.Append(chr);
                            input.Extend();
                        }
                    }
                }
            }
            return new StringLiteral {Content = result.ToString(), Range = input.Clone()};
        }

        static IStringLiteral ScanUnescaped(StringBuilder result, TextInputRange input) {
            var skipLastLine = false;
            var wsIndex = -1;
            while (true) {
                var chr = input.EndChar;
                if (chr == '\0') return null; // file end or invalid input
                if (IsDoubleQuote(chr)) {
                    input.Extend();
                    if (!IsDoubleQuote(input.EndChar)) {
                        result.Append(chr);
                        continue;
                    }
                    input.Extend();
                    if (!IsDoubleQuote(input.EndChar)) {
                        result.Append(chr).Append(chr);
                        continue;
                    }
                    input.Extend(); // 3rd quote
                    if (IsDoubleQuote(input.EndChar)) {
                        result.Append(chr);
                        input.Extend(); // 4th quote => " + end
                        if (IsDoubleQuote(input.EndChar)) {
                            result.Append(chr);
                            input.Extend(); // 5th quote => "" + end
                            if (IsDoubleQuote(input.EndChar)) {
                                result.Append(chr);
                                input.Extend(); // 6th quote => """ (continue)
                                continue;
                            }
                        }
                    }
                    if (skipLastLine && result[result.Length - 1] == '\n') result.Remove(result.Length - 1, length: 1);
                    break;
                }
                if (input.IsEndNewline) {
                    if (wsIndex >= 0) result.Remove(wsIndex, result.Length - wsIndex);
                    if (result.Length == 0) {
                        skipLastLine = true;
                    }
                    else result.Append(value: '\n');
                    input.NewLine();
                }
                else {
                    if (input.IsEndWhitespace) {
                        if (wsIndex < 0) wsIndex = result.Length;
                    }
                    else wsIndex = -1;

                    result.Append(chr);
                    input.Extend();
                }
            }
            return new StringLiteral {Content = result.ToString(), Range = input.Clone()};
        }

        static bool HandleEscape(TextInputRange input, StringBuilder result) {
            input.Extend();
            var chr = input.EndChar;
            switch (chr) {
            case '\0':
                return false;
            case 't':
                result.Append(value: '\t');
                break;
            case 'r':
                result.Append(value: '\r');
                break;
            case 'n':
                result.Append(value: '\n');
                break;
            // TODO: add Unicode escape handling
            default:
                result.Append(chr);
                break;
            }
            input.Extend();
            return true;
        }

        static void HandleControl(TextInputRange input) {
            // do not add arbitrary control characters to internal strings.
            input.Extend();
        }

        static bool IsDoubleQuote(char chr) => chr == '"';
        static bool IsBackslash(char chr) => chr == '\\';
        static bool IsTab(char chr) => chr == '\t';
    }
}
