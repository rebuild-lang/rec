using System;

namespace REC
{
    public class TextFile
    {
        public string Filename { get; set; }
        public string Content { get; set; }
    }

    public class TextPosition
    {
        public int Line { get; set; } = 1; // 1 for first line
        public int Column { get; set; } = 1; // 1 for first column
        public int Index { get; set; }

        public TextPosition Clone() {
            return new TextPosition {
                Line = Line,
                Column = Column,
                Index = Index,
            };
        }
    }

    public class TextFileRange
    {
        public TextFile File { get; set; }
        public TextPosition Start { get; set; } = new TextPosition();
        public TextPosition End { get; set; } = new TextPosition();

        public bool IsStartValid => Start.Index < File.Content.Length;
        public bool IsEndValid => End.Index < File.Content.Length;
        public int Length => End.Index - Start.Index;
        public string Text => File.Content.Substring(Start.Index, Length);

        public TextFileRange Clone() {
            return new TextFileRange {
                File = File,
                Start = Start.Clone(),
                End = End.Clone(),
            };
        }
    }

    public class TextInputRange : TextFileRange
    {
        public char EndChar => End.Index == File.Content.Length ? '\0' : File.Content[End.Index];

        public char PeekChar(int endOffset = 1) => (End.Index + endOffset) >= File.Content.Length ? '\0' : File.Content[End.Index + endOffset];

        public string EndString(int nChars) => File.Content.Substring(End.Index, Math.Min(nChars, File.Content.Length - End.Index));

        public static bool IsWhiteSpace(char chr) => chr == ' ' || chr == '\t';
        public bool IsEndWhitespace => IsWhiteSpace(EndChar);

        public static bool IsNewline(char chr) => chr == '\n' || chr == '\r';
        public bool IsEndNewline => IsNewline(EndChar);

        public bool IsKeyword(string word) {
            var len = word.Length;
            var match = EndString(len) == word && IsWhiteSpace(PeekChar(len));
            if (match) Extend(len);
            return match;
        }

        public void Skip(int nChars = 1) => End.Index += nChars;

        public void Extend(int nChars = 1) => Extend(nChars, nChars);
        public void Extend(int nChars, int columns) {
            Skip(nChars);
            End.Column += columns;
        }

        public void ExtendWhitespaces() {
            while (true) {
                var chr = EndChar;
                if (!IsWhiteSpace(chr)) break;
                Extend(1, (chr == '\t') ? 8 : 1);
            }
        }

        public void CollapseWhitespaces() {
            ExtendWhitespaces();
            Collapse();
        }

        public void NewLine() {
            var chr = EndChar;
            Skip();
            if (IsEndValid) {
                var nextChr = EndChar;
                if (chr != nextChr && (nextChr == '\n' || nextChr == '\r')) Skip();
            }
            End.Line++;
            End.Column = 1;
        }

        public void Collapse() {
            Start.Index = End.Index;
            Start.Column = End.Column;
            Start.Line = End.Line;
        }
    }
}