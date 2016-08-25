using System;
using NUnit.Framework;
using System.Collections.Generic;
using REC.Scanner;

namespace REC.Parser.Tests
{
    [TestFixture()]
    public class BlockLineGroupingTests
    {
        public struct TestData
        {
            public IEnumerable<TokenData> Input;
            public ITokenBlock Output;
        }

        static TokenData Id(string text) {
            return new TokenData {
                Type = Token.IdentifierLiteral,
                Range = new TextFileRange {
                    File = new TextFile { Content = text },
                    End = new TextPosition { Column = text.Length, Index = text.Length }
                }
            };
        }

        static TokenData StringLiteral(string text) {
            return new TokenData {
                Type = Token.StringLiteral,
                Range = new TextFileRange {
                    File = new TextFile { Content = text },
                    End = new TextPosition { Column = text.Length, Index = text.Length }
                }
            };
        }

        static TokenData WS() {
            return new TokenData { Type = Token.WhiteSpaceSeperator };
        }

        static TokenData NewLineIndentation(int column) {
            return new TokenData {
                Type = Token.NewLineIndentation,
                Range = new TextFileRange {
                    End = new TextPosition {Column = column}
                }
            };
        }
        static TokenData BlockStart(int column, ITokenBlock block = null) {
            return new TokenData {
                Type = Token.BlockStartIndentation,
                Range = new TextFileRange {
                    End = new TextPosition { Column = column }
                },
                Data = block
            };
        }
        static TokenData BlockEnd(int column) {
            return new TokenData {
                Type = Token.BlockEndIndentation,
                Range = new TextFileRange {
                    End = new TextPosition { Column = column }
                }
            };
        }

        class Line : ITokenLine
        {
            public IList<TokenData> Tokens { get; } = new List<TokenData>();
        }

        class Block : ITokenBlock
        {
            public IList<ITokenLine> Lines { get; } = new List<ITokenLine>();
        }

        static readonly TestData[] GroupTests = new[] {
            new TestData {
                Input = new [] {
                    Id(text: "if"), WS(), Id(text: "a"), NewLineIndentation(column: 4),
                        Id(text: "&&"), WS(), Id(text: "b"), BlockStart(column: 4),
                        Id(text: "print"), WS(), StringLiteral(text: "a"), NewLineIndentation(column: 1),
                    Id(text: "else"), BlockStart(column: 4),
                        Id(text: "print"), WS(), StringLiteral(text: "b"), BlockEnd(column: 1),
                },
                Output = new Block {
                    Lines = {
                        new Line {
                            Tokens = {
                                Id(text: "if"), WS(), Id(text: "a"), WS(), Id(text: "&&"), WS(), Id(text: "b"),
                                BlockStart(column: 4, block: new Block {
                                    Lines = {
                                        new Line {
                                            Tokens = { Id(text: "print"), WS(), StringLiteral(text: "a") }
                                        }
                                    }
                                }), Id(text: "else"),
                                BlockStart(column: 4, block: new Block {
                                    Lines = {
                                        new Line {
                                            Tokens = { Id(text: "print"), WS(), StringLiteral(text: "b") }
                                        }
                                    }
                                })
                            }
                        }
                    }
                }
            }, 
        };


        [TestCaseSource(nameof(GroupTests))]
        public void Group(TestData data) {
            var grouping = new BlockLineGrouping();
            var result = grouping.Group(data.Input);
            AssertBlock(data.Output, result, label: "result");
        }

        void AssertBlock(ITokenBlock expected, ITokenBlock actual, string label) {
            Assert.That(actual.Lines.Count, Is.EqualTo(expected.Lines.Count), $"{label}.Lines.Count");
            for (var i = 0; i < actual.Lines.Count; i++) {
                AssertLine(expected.Lines[i], actual.Lines[i], $"{label}.Lines[{i}]");
            }
        }

        void AssertLine(ITokenLine expected, ITokenLine actual, string label) {
            Assert.That(actual.Tokens.Count, Is.EqualTo(expected.Tokens.Count), $"{label}.Tokens.Count");
            for (var i = 0; i < actual.Tokens.Count; i++) {
                AssertToken(expected.Tokens[i], actual.Tokens[i], $"{label}.Tokens[{i}]");
            }
        }

        void AssertToken(TokenData expected, TokenData actual, string label) {
            Assert.That(actual.Type, Is.EqualTo(expected.Type), $"{label}.Type");
            switch (actual.Type) {
                case Token.BlockStartIndentation:
                    Assert.That(actual.Range.End.Column, Is.EqualTo(expected.Range.End.Column));
                    var actualBlock = actual.Data as ITokenBlock;
                    var expectedBlock = expected.Data as ITokenBlock;
                    AssertBlock(expectedBlock, actualBlock, $"{label}.Data");
                    break;

                case Token.StringLiteral:
                case Token.NumberLiteral:
                case Token.IdentifierLiteral:
                case Token.OperatorLiteral:
                    Assert.That(actual.Range.Text, Is.EqualTo(expected.Range.Text));
                    break;
                default:
                    break;
            }
        }
    }
}