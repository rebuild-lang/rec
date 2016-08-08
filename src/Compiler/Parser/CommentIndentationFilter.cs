using System.Collections.Generic;
using REC.Scanner;

namespace REC.Parser
{
    public static class CommentIndentationFilter
    {
        // filters out all comments and also indentations preceding comments
        // this is useful to prevent the indentation checker from complaining about comments
        public static IEnumerable<TokenData> Filter(IEnumerable<TokenData> input) {
            using (var it = input.GetEnumerator()) {
                do {
                    if (!it.MoveNext()) yield break;
                } while (it.Current.Type == Token.Comment); // skip initial comments
                var previous = it.Current;
                while (it.MoveNext()) {
                    if (it.Current.Type != Token.Comment) yield return previous;
                    else {
                        if (previous.Type != Token.NewLineIndentation) yield return previous; // skip indentation before comment
                        if (!it.MoveNext()) yield break; // skip comment
                    }
                    previous = it.Current;
                }
                yield return previous;
            }
        }
    }
}