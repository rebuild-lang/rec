using System;
using System.Collections.Generic;
using System.Globalization;
using REC.AST;
using REC.Tools;

namespace REC.Scanner
{
    using BracketEntry = KeyValuePair<char, TextPosition>;
    using BracketStack = Stack<KeyValuePair<char, TextPosition>>;

    // Operators can contain any symbols and are disambiguated into multiple identifiers the current identifierScope
    // This allows a+b to be parsed as three tokens
    public static class OperatorScanner
    {
        static readonly Dictionary<char, char> OpenCloseDict = new Dictionary<char, char> {
            [key: '{'] = '}',
            [key: '⟨'] = '⟩',
            [key: '“'] = '”',
            [key: '‘'] = '’',
            [key: '«'] = '»'
        };

        // this will scan the longest valid operator entry
        public static IIdentifierLiteral Scan(TextInputRange input) {
            var brackets = new BracketStack();

            if (!IsStart(input, brackets)) return null;

            do {
                input.Extend();
            } while (IsContinue(brackets, input));

            while (!brackets.IsEmpty()) {
                input.End = brackets.Pop().Value;
                if (input.Length == 0) return null;
            }

            return new IdentifierLiteral {Content = input.Text, Range = input.Clone()};
        }

        static bool IsStart(TextInputRange input, BracketStack brackets) {
            return IsContinue(brackets, input); // no more special rules
        }

        static bool IsContinue(BracketStack brackets, TextInputRange input) {
            var chr = input.EndChar;
            switch (CharUnicodeInfo.GetUnicodeCategory(chr)) {
                case UnicodeCategory.LowercaseLetter: // lower
                case UnicodeCategory.UppercaseLetter: // UPPER
                case UnicodeCategory.DecimalDigitNumber: // 0-9
                case UnicodeCategory.ConnectorPunctuation: // various underscores _‿⁀⁔＿
                    return false; // regular identifiers and literals

                case UnicodeCategory.MathSymbol: // +=/
                case UnicodeCategory.OtherSymbol: // ©®⌛⌚
                case UnicodeCategory.OtherNumber: // ½²
                case UnicodeCategory.PrivateUse: // emojis etc.
                    return true;
                case UnicodeCategory.CurrencySymbol: // ¢¥$€
                    return chr != '$'; // $ is used to start pattern literals (we might want to disambiguate this later)
                case UnicodeCategory.OtherPunctuation: // like !?#.
                    return chr != '#' && chr != '.' && chr != ',';
                case UnicodeCategory.OpenPunctuation: // [{(
                case UnicodeCategory.InitialQuotePunctuation: // “«
                    if (!OpenCloseDict.ContainsKey(chr)) return false;
                    brackets.Push(new BracketEntry(OpenCloseDict[chr], input.End.Clone()));
                    // TODO: allow more chars inside brackets
                    return true;
                case UnicodeCategory.ClosePunctuation: // ]})
                case UnicodeCategory.FinalQuotePunctuation: // ”»
                    if (brackets.IsEmpty() || brackets.Peek().Key != chr) return false;
                    brackets.Pop();
                    return true;
                case UnicodeCategory.DashPunctuation: // all kinds of hyphens
                    return chr == '-';
                case UnicodeCategory.TitlecaseLetter: // Ligatures ǅ, ǈ, ǋ and ǲ
                case UnicodeCategory.OtherLetter: // Hebrew etc. א
                case UnicodeCategory.LetterNumber: // Roman Numbers (look like normal letters)
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
