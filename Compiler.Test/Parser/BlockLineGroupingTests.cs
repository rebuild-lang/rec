using System.Collections.Generic;
using NUnit.Framework;
using REC.AST;
using REC.Parser;
using REC.Scanner;

namespace REC.Tests.Parser
{
    using static TokenHelpers;

    [TestFixture]
    public class BlockLineGroupingTests
    {
        public struct TestData
        {
            public string Name;
            public IEnumerable<TokenData> Input;
            public IBlockLiteral Output;
            public override string ToString() => Name;
        }

        static readonly TestData[] GroupTests = {
            new TestData {
                Name = "Example",
                Input = new[] {
                    Id(text: "if"), Id(text: "a"), NewLineIndentation(column: 4),
                    Op(text: "&&"), Id(text: "b"), BlockStart(column: 4),
                    Id(text: "print"), StringLiteral(text: "a"), NewLineIndentation(column: 4),
                    Id(text: "print"), StringLiteral(text: "b"), NewLineIndentation(column: 1),
                    Id(text: "else"), BlockStart(column: 4),
                    Id(text: "print"), StringLiteral(text: "b"), BlockEnd(column: 1)
                },
                Output = new BlockLiteral {
                    Lines = {
                        new TokenLine {
                            Tokens = {
                                Id(text: "if"),
                                Id(text: "a"),
                                Op(text: "&&"),
                                Id(text: "b"),
                                BlockStart(
                                    column: 4,
                                    block: new BlockLiteral {
                                        Lines = {
                                            new TokenLine {
                                                Tokens = {Id(text: "print"), StringLiteral(text: "a")}
                                            },
                                            new TokenLine {
                                                Tokens = {Id(text: "print"), StringLiteral(text: "b")}
                                            }
                                        }
                                    }),
                                Id(text: "else"),
                                BlockStart(
                                    column: 4,
                                    block: new BlockLiteral {
                                        Lines = {
                                            new TokenLine {
                                                Tokens = {Id(text: "print"), StringLiteral(text: "b")}
                                            }
                                        }
                                    })
                            }
                        }
                    }
                }
            }
        };

        [TestCaseSource(nameof(GroupTests))]
        public void Group(TestData data) {
            var grouping = new BlockLineGrouping();
            var result = grouping.Group(data.Input);
            AssertBlock(data.Output, result, label: "result");
        }

        void AssertBlock(IBlockLiteral expected, IBlockLiteral actual, string label) {
            Assert.That(actual.Lines.Count, Is.EqualTo(expected.Lines.Count), $"{label}.Lines.Count");
            for (var i = 0; i < actual.Lines.Count; i++) AssertLine(expected.Lines[i], actual.Lines[i], $"{label}.Lines[{i}]");
        }

        void AssertLine(ITokenLine expected, ITokenLine actual, string label) {
            Assert.That(actual.Tokens.Count, Is.EqualTo(expected.Tokens.Count), $"{label}.Tokens.Count");
            for (var i = 0; i < actual.Tokens.Count; i++) AssertToken(expected.Tokens[i], actual.Tokens[i], $"{label}.Tokens[{i}]");
        }

        void AssertToken(TokenData expected, TokenData actual, string label) {
            Assert.That(actual.Type, Is.EqualTo(expected.Type), $"{label}.Type");
            switch (actual.Type) {
            case Token.BlockStartIndentation:
                Assert.That(actual.Range.End.Column, Is.EqualTo(expected.Range.End.Column));
                var actualBlock = actual.Data as IBlockLiteral;
                var expectedBlock = expected.Data as IBlockLiteral;
                AssertBlock(expectedBlock, actualBlock, $"{label}.Data");
                break;

            case Token.StringLiteral:
            case Token.NumberLiteral:
            case Token.IdentifierLiteral:
            case Token.OperatorLiteral:
                Assert.That(actual.Range.Text, Is.EqualTo(expected.Range.Text));
                break;
            }
        }
    }
}
