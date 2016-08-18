using System.Collections.Generic;
using REC.Scanner;

namespace REC.Parser
{
    public static class CommentIndentationFilter
    {
        // filters out 
        // * all comments 
        // * newline + indentations preceding comments
        // * two adjacent newline + indentations (basically multiple newlines)
        // this simplifies the evaluation of the token stream
        public static IEnumerable<TokenData> Filter(IEnumerable<TokenData> input) {
            using (var it = input.GetEnumerator()) {
                do {
                    if (!it.MoveNext()) yield break;
                } while (it.Current.Type == Token.Comment); // skip initial comments
                var previous = it.Current;
                while (it.MoveNext()) {
                    switch (it.Current.Type) {
                        case Token.Comment: // skip any comment
                            if (previous.Type != Token.NewLineIndentation) yield return previous; // skip indentation before comment
                            if (!it.MoveNext()) yield break; // skip final comment / indentation
                            break;
                        case Token.NewLineIndentation:
                            if (previous.Type != Token.NewLineIndentation) yield return previous; // skip double newlines
                            break;
                        default:
                            yield return previous; // keep everything else
                            break;
                    }
                    previous = it.Current;
                }
                if (previous.Type != Token.NewLineIndentation) yield return previous;
            }
        }
    }
}