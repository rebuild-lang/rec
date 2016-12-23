using System.Collections.Generic;
using NUnit.Framework;
using REC.AST;
using REC.Intrinsic;
using REC.Intrinsic.Types;
using REC.Scanner;
using REC.Parser;
using REC.Scope;
using REC.Tools;

namespace REC.Tests.Parser
{
    [TestFixture]
    public class FunctionDeclParserTests
    {
        static TokenData Id(string text) {
            return new TokenData {
                Type = Token.IdentifierLiteral,
                Range = new TextFileRange {
                    File = new TextFile {Content = text},
                    End = new TextPosition {Column = text.Length, Index = text.Length}
                },
                Data = new IdentifierLiteral {
                    Content = text
                }
            };
        }

        static TokenData Op(string text) {
            return new TokenData {
                Type = Token.OperatorLiteral,
                Range = new TextFileRange {
                    File = new TextFile {Content = text},
                    End = new TextPosition {Column = text.Length, Index = text.Length}
                },
                Data = new IdentifierLiteral {
                    Content = text
                }
            };
        }

        static TokenData BracketOpen() {
            const string text = "(";
            return new TokenData {
                Type = Token.BracketOpen,
                Range = new TextFileRange {
                    File = new TextFile {Content = text},
                    End = new TextPosition {Column = text.Length, Index = text.Length}
                }
            };
        }

        static TokenData BracketClose() {
            const string text = ")";
            return new TokenData {
                Type = Token.BracketClose,
                Range = new TextFileRange {
                    File = new TextFile {Content = text},
                    End = new TextPosition {Column = text.Length, Index = text.Length}
                }
            };
        }

        static TokenData NumberLiteral(string text) {
            return new TokenData {
                Type = Token.NumberLiteral,
                Range = new TextFileRange {
                    File = new TextFile {Content = text},
                    End = new TextPosition {Column = text.Length, Index = text.Length}
                },
                Data = new NumberLiteral {
                    Radix = 10,
                    IntegerPart = text
                }
            };
        }

        static TokenData BlockStart(int column, IBlockLiteral block = null) {
            return new TokenData {
                Type = Token.BlockStartIndentation,
                Range = new TextFileRange {
                    End = new TextPosition {Column = column}
                },
                Data = block
            };
        }

        public struct ParseFunctionDeclTestData
        {
            public string Name;
            public IScope Scope;
            public IEnumerable<TokenData> Input;
            public IFunctionDeclaration Output;

            public override string ToString() => Name;
        }

        static IScope BuildTestScope() {
            var scope = new REC.Parser.Scope();
            DeclarationConverter.BuildScope(
                scope,
                new IntrinsicDict {
                    U64Type.Get(),
                });
            return scope;
        }

        static readonly IScope TestScope = BuildTestScope();

        static readonly ParseFunctionDeclTestData[] ParseFunctionDeclTests = {
            new ParseFunctionDeclTestData {
                Name = "no args",
                Scope = new REC.Parser.Scope(),
                Input = new[] {
                    Id(text: "fn"), Id(text: "a"),
                    BlockStart(
                        column: 4,
                        block: new BlockLiteral {
                            Lines = {new TokenLine()}
                        })
                },
                Output = new FunctionDeclaration {
                    Name = "a",
                    LeftArguments = new NamedCollection<IArgumentDeclaration>(),
                    RightArguments = new NamedCollection<IArgumentDeclaration>(),
                    Results = new NamedCollection<IArgumentDeclaration>(),
                    Implementation = new ExpressionBlock()
                }
            },
            new ParseFunctionDeclTestData {
                Name = "empty args",
                Scope = new REC.Parser.Scope(),
                Input = new[] {
                    Id(text: "fn"), Id(text: "a"), BracketOpen(), BracketClose(),
                    BlockStart(
                        column: 4,
                        block: new BlockLiteral {
                            Lines = {new TokenLine()}
                        })
                },
                Output = new FunctionDeclaration {
                    Name = "a",
                    LeftArguments = new NamedCollection<IArgumentDeclaration>(),
                    RightArguments = new NamedCollection<IArgumentDeclaration>(),
                    Results = new NamedCollection<IArgumentDeclaration>(),
                    Implementation = new ExpressionBlock()
                }
            },
            new ParseFunctionDeclTestData {
                Name = "no bracket args",
                Scope = new REC.Parser.Scope {Parent = TestScope},
                Input = new[] {
                    Id(text: "fn"), Id(text: "a"),
                    Id(text: "arg"), Op(text: ":"), Id(text: "u64"),
                    BlockStart(
                        column: 4,
                        block: new BlockLiteral {
                            Lines = {new TokenLine()}
                        })
                },
                Output = new FunctionDeclaration {
                    Name = "a",
                    LeftArguments = new NamedCollection<IArgumentDeclaration>(),
                    RightArguments = new NamedCollection<IArgumentDeclaration> {
                        new ArgumentDeclaration {
                            Name = "arg",
                            Type = (TestScope.Identifiers[key: "u64"] as IModuleEntry)?.ModuleDeclaration
                        }
                    },
                    Results = new NamedCollection<IArgumentDeclaration>(),
                    Implementation = new ExpressionBlock()
                }
            },
            new ParseFunctionDeclTestData {
                Name = "bracket args",
                Scope = new REC.Parser.Scope {Parent = TestScope},
                Input = new[] {
                    Id(text: "fn"),
                    BracketOpen(),
                    Id(text: "leftarg"), Op(text: ":"), Id(text: "u64"),
                    BracketClose(),
                    Id(text: "a"),
                    BracketOpen(),
                    Id(text: "rightarg"), Op(text: ":"), Id(text: "u64"),
                    BracketClose(),
                    BlockStart(
                        column: 4,
                        block: new BlockLiteral {
                            Lines = {new TokenLine()}
                        })
                },
                Output = new FunctionDeclaration {
                    Name = "a",
                    LeftArguments = new NamedCollection<IArgumentDeclaration> {
                        new ArgumentDeclaration {
                            Name = "leftarg",
                            Type = (TestScope.Identifiers[key: "u64"] as IModuleEntry)?.ModuleDeclaration
                        }
                    },
                    RightArguments = new NamedCollection<IArgumentDeclaration> {
                        new ArgumentDeclaration {
                            Name = "rightarg",
                            Type = (TestScope.Identifiers[key: "u64"] as IModuleEntry)?.ModuleDeclaration
                        }
                    },
                    Results = new NamedCollection<IArgumentDeclaration>(),
                    Implementation = new ExpressionBlock()
                }
            },
        };

        [TestCaseSource(nameof(ParseFunctionDeclTests))]
        public void ParseFunctionDeclTest(ParseFunctionDeclTestData data) {
            using (var it = data.Input.GetEnumerator()) {
                it.MoveNext();
                var done = false;
                var functionDecl = FunctionDeclParser.Parse(it, data.Scope, done: ref done);
                Assert.IsTrue(done);
                AssertFunctionDecl(data.Output, (dynamic) functionDecl);
            }
        }

        void AssertFunctionDecl(IFunctionDeclaration expected, IFunctionDeclaration actual) {
            Assert.AreEqual(expected.Name, actual.Name);
            Assert.AreEqual(expected.IsRuntimeUsable, actual.IsRuntimeUsable);
            Assert.AreEqual(expected.IsCompileTimeUsable, actual.IsCompileTimeUsable);
            AssertArguments(expected.LeftArguments, actual.LeftArguments, name: "Left");
            AssertArguments(expected.RightArguments, actual.RightArguments, name: "Right");
            AssertArguments(expected.Results, actual.Results, name: "Results");
        }

        void AssertArguments(NamedCollection<IArgumentDeclaration> expected, NamedCollection<IArgumentDeclaration> actual, string name) {
            Assert.AreEqual(expected.Count, actual.Count, $"{name}Arguments.Count");
            for (var i = 0; i < expected.Count; i++) AssertArgument(expected[i], actual[i], name: $"{name}Arguments[{i}]");
        }

        void AssertArgument(IArgumentDeclaration expected, IArgumentDeclaration actual, string name) {
            Assert.AreEqual(expected.Name, actual.Name, $"{name}.Name");
            Assert.AreEqual(expected.IsAssignable, actual.IsAssignable, $"{name}.IsAssignable");
            Assert.AreEqual(expected.IsUnrolled, actual.IsUnrolled, $"{name}.IsUnrolled");
            Assert.AreEqual(expected.Type.Name, actual.Type.Name, $"{name}.Type.Name");
        }
    }
}
