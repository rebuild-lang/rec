using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using REC.AST;
using REC.Tools;

namespace REC.Scanner
{
    using EntryDict = Dictionary<char, Entry>;
    using BracketEntry = KeyValuePair<char, TextPosition>;
    using BracketStack = Stack<KeyValuePair<char, TextPosition>>;

    public interface IIdentifier
    {
        string Label { get; }
    }

    internal class Entry
    {
        private int _depth;
        private readonly EntryDict _options = new EntryDict(); // decision options after last streak char
        private IIdentifier _identifier; // Identifier if nothing matches

        public IIdentifier Scan(TextInputRange input) {
            if (_options.IsEmpty() || !input.IsEndValid)
                return _identifier;
            Entry next;
            if (!_options.TryGetValue(input.EndChar, out next))
                return _identifier;
            input.Extend();
            var result = next.Scan(input);
            if (result != null) return result;
            input.Extend(-1);
            return _identifier;
        }

        public void Add(IIdentifier identifier) {
            var label = identifier.Label;
            if (label.Length == _depth) {
                _identifier = identifier;
                return;
            }
            Debug.Assert(label.Length > _depth);
            var option = label[_depth];
            var next = _options.GetOrAdd(option, () => new Entry { _depth = _depth + 1 });
            next.Add(identifier);
        }
    }

    public class IdentifierScanner
    {
        private readonly Entry _root = new Entry();

        public IIdentifier ScanExisting(TextInputRange input) => _root.Scan(input);

        public static IIdentifierLiteral ScanNew(TextInputRange input) {
            var brackets = new BracketStack();
            var chr = input.EndChar;

            if (!IsIdentifierStart(input, brackets, chr)) return null;

            do {
                input.Extend();
                chr = input.EndChar;
            } while (IsIdentifier(chr, brackets, input));

            while (!brackets.IsEmpty()) {
                input.End = brackets.Pop().Value;
                if (input.Length == 0) return null;
            }

            return new IdentifierLiteral { Content = input.Text, Range = input };
        }

        public void Add(IIdentifier identifier) => _root.Add(identifier);

        private static bool IsIdentifierStart(TextInputRange input, BracketStack brackets, char chr) {
            if (chr == '.') return true; // dot is allowed at start but not later
            if (chr == '$') return false; // $ is not allowed at start but later
            if ('0' <= chr && '9' >= chr) return false; // digits are not allowed at start
            return IsIdentifier(chr, brackets, input); // no more special rules
        }

        internal static readonly Dictionary<char, char> OpenCloseDict = new Dictionary<char, char> {
            ['('] = ')',
            ['['] = ']',
            ['{'] = '}',
            ['⟨'] = '⟩',
            ['“'] = '”',
            ['‘'] = '’',
            ['«'] = '»',
        };

        private static bool IsIdentifier(char chr, BracketStack brackets, TextInputRange input) {
            switch (CharUnicodeInfo.GetUnicodeCategory(chr)) {
                case UnicodeCategory.LowercaseLetter: // lower
                case UnicodeCategory.UppercaseLetter: // UPPER
                //case UnicodeCategory.TitlecaseLetter: // Ligatures ǅ, ǈ, ǋ and ǲ
                //case UnicodeCategory.OtherLetter: // Hebrew etc. א
                case UnicodeCategory.MathSymbol: // +=-
                case UnicodeCategory.OtherSymbol: // ©®⌛⌚
                //case UnicodeCategory.DashPunctuation: // all kinds of hyphens
                case UnicodeCategory.ConnectorPunctuation: // various underscores _‿⁀⁔＿
                case UnicodeCategory.DecimalDigitNumber: // 0-9
                // case UnicodeCategory.LetterNumber: // Roman Numbers (look like normal letters)
                case UnicodeCategory.OtherNumber: // ½²
                case UnicodeCategory.PrivateUse: // emojis etc.
                case UnicodeCategory.CurrencySymbol: // ¢¥$€
                    return true;
                case UnicodeCategory.OtherPunctuation: // like !?#.
                    return chr != '#' && chr != '.' && chr != ',';
                case UnicodeCategory.OpenPunctuation: // [{(
                case UnicodeCategory.InitialQuotePunctuation: // “«
                    if (!OpenCloseDict.ContainsKey(chr)) return false;
                    brackets.Push(new BracketEntry(OpenCloseDict[chr], input.End.Clone()));
                    return true;
                case UnicodeCategory.ClosePunctuation: // ]})
                case UnicodeCategory.FinalQuotePunctuation: // ”»
                    if (brackets.IsEmpty() || brackets.Peek().Key != chr) return false;
                    brackets.Pop();
                    return true;
            }
            return false;
        }
    }
}