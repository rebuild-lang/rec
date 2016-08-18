using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;
using REC.Parser;
using REC.Scanner;

namespace REC.Tests.Parser
{
    [TestFixture()]
    public class CommentIndentationFilterTests
    {
        [TestCase(arg1: new[] {
            Token.Comment,
            Token.NewLineIndentation,
        }, arg2: new[] {
            Token.NewLineIndentation
        })]
        [TestCase(arg1: new[] {
            Token.NewLineIndentation, Token.Comment,
            Token.NewLineIndentation, Token.IdentifierLiteral, Token.Comment,
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral,
        })]
        public void Filter(IEnumerable<Token> inputTokens, IEnumerable<Token> expectedTokens) {
            var input = inputTokens.Select(t => new TokenData {Type = t});
            var expected = expectedTokens.Select(t => new TokenData {Type = t});

            var filtered = CommentIndentationFilter.Filter(input);
            Assert.AreEqual(expected, filtered);
        }
    }
}