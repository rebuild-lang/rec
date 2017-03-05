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

        public void Assign(TextPosition position) {
            Line = position.Line;
            Column = position.Column;
            Index = position.Index;
        }

        public TextPosition Clone() {
            return new TextPosition {
                Line = Line,
                Column = Column,
                Index = Index
            };
        }

        public TextPosition ColumnMoved(int columnOffset) {
            return new TextPosition {
                Line = Line,
                Column = Column + columnOffset,
                Index = Index + columnOffset
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
        public string Text => Length > 0 ? File.Content.Substring(Start.Index, Length) : string.Empty;

        public void Assign(TextFileRange range) {
            File = range.File;
            Start.Assign(range.Start);
            End.Assign(range.End);
        }

        public TextFileRange Clone() {
            return new TextFileRange {
                File = File,
                Start = Start.Clone(),
                End = End.Clone()
            };
        }

        public TextFileRange SubRange(int left, int count) {
            return new TextFileRange {
                File = File,
                Start = Start.ColumnMoved(left),
                End = Start.ColumnMoved(left + count)
            };
        }
    }

    public class TextInputRange : TextFileRange
    {
        public char EndChar => End.Index == File.Content.Length ? '\0' : File.Content[End.Index];
        public bool IsEndWhitespace => IsWhiteSpace(EndChar);
        public bool IsEndNewline => IsNewline(EndChar);

        public char PeekChar(int endOffset = 1) => End.Index + endOffset >= File.Content.Length ? '\0' : File.Content[End.Index + endOffset];

        public string EndString(int nChars) => File.Content.Substring(End.Index, Math.Min(nChars, File.Content.Length - End.Index));

        public static bool IsWhiteSpace(char chr) => chr == ' ' || chr == '\t';

        public static bool IsNewline(char chr) => chr == '\n' || chr == '\r';

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
                Extend(nChars: 1, columns: chr == '\t' ? 8 : 1);
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

        public void Backtrack() {
            End.Assign(Start);
        }

        public void Collapse() {
            Start.Assign(End);
        }
    }
}
