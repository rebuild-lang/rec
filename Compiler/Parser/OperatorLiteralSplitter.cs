using System.Collections.Generic;
using System.Linq;
using REC.AST;
using REC.Scanner;
using REC.Tools;

namespace REC.Parser
{
    class OperatorLiteralSplitter
    {
        internal static IList<TokenData> SplitAll(IEnumerable<TokenData> tokens, IContext context) {
            return tokens.SelectMany(
                token => {
                    if (token.Type != Token.OperatorLiteral) return new[] {token};
                    var operatorLiteral = (IIdentifierLiteral) token.Data;
                    return Split(operatorLiteral, context);
                }).ToList();
        }

        internal static IEnumerable<TokenData> Split(IIdentifierLiteral operatorLiteral, IContext context) {
            var result = new List<TokenData>();
            var content = operatorLiteral.Content;
            var left = 0;
            var leftover = 0;
            var right = content.Length;
            while (left != right) {
                var resolve = context.Identifiers[content.Substring(left, right - left)];
                if (resolve == null) {
                    right -= 1; // try shorter part
                    if (left != right) continue; // tried all
                    left += 1; // move one right
                    right = content.Length; // try again
                    continue;
                }

                if (leftover != left) {
                    // we have some not identified parts
                    result.Add(MakeSubIdentifier(operatorLiteral, leftover, left));
                }
                result.Add(MakeSubIdentifier(operatorLiteral, left, right));
                left = right; // start from right
                leftover = left;
                right = content.Length;
            }
            if (leftover != left) {
                // some not identified parts
                result.Add(MakeSubIdentifier(operatorLiteral, leftover, left));
            }

            // transfer separates flags
            ((IdentifierLiteral) result.First().Data).LeftSeparates = operatorLiteral.LeftSeparates;
            ((IdentifierLiteral) result.Last().Data).RightSeparates = operatorLiteral.RightSeparates;
            return result;
        }

        static TokenData MakeSubIdentifier(IIdentifierLiteral identifier, int left, int right) {
            var range = identifier.Range?.SubRange(left, right - left);
            var content = identifier.Content.Substring(left, right - left);
            return new TokenData {
                Type = Token.IdentifierLiteral,
                Range = range,
                Data = new IdentifierLiteral {
                    Content = content,
                    Range = range,
                    SplittedFrom = identifier,
                }
            };
        }
    }
}
