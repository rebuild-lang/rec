namespace REC.Scanner
{
    static class CommentScanner
    {
        public static bool Scan(TextInputRange range) {
            if (range.EndChar != '#') return false;
            // TODO: allow for #(...) block comments
            do {
                range.Extend();
            } while (range.IsEndValid && !range.IsEndNewline);
            return true;
        }
    }
}