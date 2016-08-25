using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using NUnit.Framework;
using REC.Parser;
using REC.Scanner;

namespace REC.Tests.Parser
{
    [TestFixture()]
    public class TokenPreparationTests
    {
        [TestCase(arg1: new object[] {
            Token.Comment,
            Token.NewLineIndentation, "",
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral
        }, TestName = "Apply starting comment")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation, Token.Comment,
            Token.NewLineIndentation, ""
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral,
        }, TestName = "Apply indented comment")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation, Token.Comment, Token.WhiteSpaceSeperator, Token.Comment,
            Token.NewLineIndentation, ""
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral,
        }, TestName = "Apply comment whitespace comment")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation, "", Token.Comment,
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral,
        }, TestName = "Apply final comment")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation, "", Token.WhiteSpaceSeperator,
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral,
        }, TestName = "Apply final whitespace")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation,
            Token.NewLineIndentation, "",
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral,
        }, TestName = "Apply newline sequence")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation,
            Token.NewLineIndentation,
            Token.NewLineIndentation, "",
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral,
        }, TestName = "Apply long newline sequence")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation, "", Token.NewLineIndentation,
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral,
        }, TestName = "Apply final newline")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation, "end", Token.NewLineIndentation,
        }, arg2: new[] {
            Token.BlockEndIndentation,
        }, TestName = "Mutate block end")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation, "begin", ":", Token.NewLineIndentation,
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral, Token.BlockStartIndentation,
        }, TestName = "Mutate block start")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation, "begin", ":", Token.WhiteSpaceSeperator, Token.Comment, Token.NewLineIndentation,
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral, Token.BlockStartIndentation,
        }, TestName = "Mutate block start with comment")]
        public void Filter(IEnumerable<dynamic> inputTokens, IEnumerable<Token> expectedTokens) {
            var input = inputTokens.Select(
                t => {
                    var s = t as string;
                    if (s != null) {
                        return new TokenData { Type = Token.IdentifierLiteral, Range = new TextFileRange {File = new TextFile {Content = s}, End = new TextPosition {Index = s.Length} } };
                    }
                    return new TokenData {Type = t};
                });

            var filtered = TokenPreparation.Apply(input).ToArray();

            Assert.AreEqual(expected: expectedTokens, actual: filtered.Select(t => t.Type));
        }
    }
}