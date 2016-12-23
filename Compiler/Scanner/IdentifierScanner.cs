using System;
using System.Globalization;
using REC.AST;

namespace REC.Scanner
{
    public static class IdentifierScanner
    {
        // this will scan the longest valid regular entry
        public static IIdentifierLiteral Scan(TextInputRange input) {
            if (!IsStart(input)) {
                input.Backtrack(); // dot might have been skipped
                return null;
            }
            do {
                input.Extend();
            } while (IsContinue(input));

            return new IdentifierLiteral {Content = input.Text, Range = input.Clone()};
        }

        static bool IsStart(TextInputRange input) {
            var chr = input.EndChar;
            if (chr == '.') {
                input.Extend(); // dot is allowed at start but not later
                chr = input.EndChar;
            }
            if ('0' <= chr && '9' >= chr) return false; // digits are not allowed at start
            return IsContinue(input); // no more special rules
        }

        static bool IsContinue(TextInputRange input) {
            var chr = input.EndChar;
            switch (CharUnicodeInfo.GetUnicodeCategory(chr)) {
                case UnicodeCategory.LowercaseLetter: // lower
                case UnicodeCategory.UppercaseLetter: // UPPER
                case UnicodeCategory.DecimalDigitNumber: // 0-9
                case UnicodeCategory.ConnectorPunctuation: // various underscores _‿⁀⁔＿
                    return true;
                case UnicodeCategory.TitlecaseLetter: // Ligatures ǅ, ǈ, ǋ and ǲ
                case UnicodeCategory.OtherLetter: // Hebrew etc. א
                case UnicodeCategory.MathSymbol: // +=-
                case UnicodeCategory.OtherSymbol: // ©®⌛⌚
                case UnicodeCategory.DashPunctuation: // all kinds of hyphens                
                case UnicodeCategory.LetterNumber: // Roman Numbers (look like normal letters)
                case UnicodeCategory.OtherNumber: // ½²
                case UnicodeCategory.PrivateUse: // emojis etc.
                case UnicodeCategory.CurrencySymbol: // ¢¥$€
                case UnicodeCategory.OtherPunctuation: // like !?#.
                case UnicodeCategory.OpenPunctuation: // [{(
                case UnicodeCategory.InitialQuotePunctuation: // “«
                case UnicodeCategory.ClosePunctuation: // ]})
                case UnicodeCategory.FinalQuotePunctuation: // ”»
                case UnicodeCategory.ModifierLetter:
                case UnicodeCategory.NonSpacingMark:
                case UnicodeCategory.SpacingCombiningMark:
                case UnicodeCategory.EnclosingMark:
                case UnicodeCategory.SpaceSeparator:
                case UnicodeCategory.LineSeparator:
                case UnicodeCategory.ParagraphSeparator:
                case UnicodeCategory.Control:
                case UnicodeCategory.Format:
                case UnicodeCategory.Surrogate:
                case UnicodeCategory.ModifierSymbol:
                case UnicodeCategory.OtherNotAssigned:
                    return false;
                default:
                    throw new ArgumentOutOfRangeException();
            }
        }
    }
}
