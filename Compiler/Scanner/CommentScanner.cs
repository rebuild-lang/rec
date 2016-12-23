using System;

namespace REC.Scanner
{
    public static class CommentScanner
    {
        /**
         * # single line
         * 
         * #<optional>#
         * block comments
         * #<optional># 
         */

        public static bool Scan(TextInputRange range) {
            if (range.EndChar != '#') return false;
            do {
                range.Extend();
            } while (range.IsEndValid && !range.IsEndNewline && !range.IsEndWhitespace && range.EndChar != '#');

            // single line comment
            if (range.IsEndNewline) return true;

            // is block comment
            if (range.EndChar == '#') {
                range.Extend();
                var commentMarker = range.Text;

                while (range.IsEndValid && range.EndString(commentMarker.Length) != commentMarker) {
                    if (range.IsEndNewline)
                        range.NewLine();
                    else
                        range.Extend();
                }

                if (range.EndString(commentMarker.Length) != commentMarker)
                    throw new Exception(message: "Line Comment not Escaped.");
                range.Extend(commentMarker.Length);
            }
            else {
                do {
                    range.Extend();
                } while (range.IsEndValid && !range.IsEndNewline);
            }
            return true;
        }
    }
}
