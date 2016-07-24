using System.Text;
using REC.AST;

namespace REC.Scanner
{
    public class StringLiteralScanner
    {
        public static IStringLiteral Scan(TextInputRange input) {
            var chr = input.EndChar;
            if (!IsDoubleQuote(chr)) return null;
            input.Extend();

            var result = new StringBuilder();

            while (true) {
                chr = input.EndChar;
                if (chr == '\0') return null; // file end or invalid input
                if (IsDoubleQuote(chr)) break;
                if (input.IsEndNewline) input.NewLine();
                else if (!IsTab(chr) && char.IsControl(chr)) HandleControl(input);
                else if (IsBackslash(chr))
                {
                    if (!HandleEscape(input, result)) return null;
                }
                else
                {
                    result.Append(chr);
                    input.Extend();
                }
            }

            input.Extend();
            return new StringLiteral { Content = result.ToString() };
        }

        private static bool HandleEscape(TextInputRange input, StringBuilder result) {
            input.Extend();
            var chr = input.EndChar;
            switch (chr) {
                case '\0':
                    return false;
                case 't':
                    result.Append('\t'); break;
                case 'r':
                    result.Append('\r'); break;
                case 'n':
                    result.Append('\n'); break;
                // TODO: add Unicode escape handling
                default:
                    result.Append(chr); break;
            }
            input.Extend();
            return true;
        }

        private static void HandleControl(TextInputRange input) {
            // do not add arbitrary control characters to internal strings.
            input.Extend();
        }

        private static bool IsDoubleQuote(char chr) => chr == '"';
        private static bool IsBackslash(char chr) => chr == '\\';
        private static bool IsTab(char chr) => chr == '\t';
    }
}
