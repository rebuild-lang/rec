namespace REC
{
    internal class IndentationParser
    {
        public enum Reason
        {
            Block,
            Arguments,
        }

        public interface ILevel
        {
            int Column { get; }
            int Depth { get; }
            Reason Reason { get; set; }
            ILevel Parent { get; }
        }

        public enum IndentStyle
        {
            Undefined,
            Whitespace,
            Tabulator,
        }

        public ILevel LastLevel { get; private set; }
        public ILevel CurrentLevel { get; private set; }
        public IndentStyle Style { get; private set; }

        public bool ParseNewline(TextInputRange range) {
            if (!range.IsEndNewline) return false;
            do {
                range.NewLine();
                ExtractStyle(range);
                ExtendWhitespaces(range);
            } while (range.IsEndNewline); // skip empty lines
            UpdateLevel(range.End.Column);
            range.Collapse();
            return true;
        }

        private void ExtractStyle(TextInputRange range) {
            if (Style == IndentStyle.Undefined) {
                var chr = range.EndChar;
                if (IsTab(chr)) Style = IndentStyle.Tabulator;
                if (IsWhitespace(chr)) Style = IndentStyle.Whitespace;
            }
        }

        private static void ExtendWhitespaces(TextInputRange range) {
            while (true) {
                var chr = range.EndChar;
                if (IsTab(chr)) {
                    // TODO: warn for mismatching style
                    range.Extend(8);
                }
                else if (IsWhitespace(chr)) {
                    // TODO: warn for mismatching style
                    range.Extend();
                }
                else break;
            }
        }

        private class Level : ILevel
        {
            public int Column { get; set; }
            public int Depth { get; set; }
            public Reason Reason { get; set; }
            public ILevel Parent { get; set; }
        }

        private void UpdateLevel(int column) {
            LastLevel = CurrentLevel;
            while (CurrentLevel != null && CurrentLevel.Column > column) {
                CurrentLevel = CurrentLevel.Parent;
            }
            if (column > (CurrentLevel?.Column ?? 1)) {
                CurrentLevel = new Level {
                    Column = column,
                    Depth = (CurrentLevel?.Depth ?? 0) + 1,
                    Parent = CurrentLevel
                };
            }
        }

        private static bool IsWhitespace(char chr) => chr == ' ';
        private static bool IsTab(char chr) => chr == '\t';
    }
}