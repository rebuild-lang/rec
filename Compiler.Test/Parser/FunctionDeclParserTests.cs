using NUnit.Framework;
using REC.AST;
using REC.Instance;
using REC.Intrinsic;
using REC.Intrinsic.Types;
using REC.Parser;
using REC.Scanner;
using REC.Tools;
using System.Collections.Generic;

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
            public IContext Context;
            public IEnumerable<TokenData> Input;
            public IFunctionDeclaration Output;

            public override string ToString() => Name;
        }

        static IContext BuildTestContext() {
            var context = new Context();
            DeclarationConverter.BuildContext(
                context,
                new IntrinsicDict {
                    U64Type.Get(),
                });
            return context;
        }

        static readonly IContext TestContext = BuildTestContext();

        static readonly ParseFunctionDeclTestData[] ParseFunctionDeclTests = {
            new ParseFunctionDeclTestData {
                Name = "no args",
                Context = new Context(),
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
                Context = new Context(),
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
                Context = new Context {Parent = TestContext},
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
                            Type = TestContext.Identifiers[key: "u64"] as IModuleInstance
                        }
                    },
                    Results = new NamedCollection<IArgumentDeclaration>(),
                    Implementation = new ExpressionBlock()
                }
            },
            new ParseFunctionDeclTestData {
                Name = "bracket args",
                Context = new Context {Parent = TestContext},
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
                            Type = TestContext.Identifiers[key: "u64"] as IModuleInstance
                        }
                    },
                    RightArguments = new NamedCollection<IArgumentDeclaration> {
                        new ArgumentDeclaration {
                            Name = "rightarg",
                            Type = TestContext.Identifiers[key: "u64"] as IModuleInstance
                        }
                    },
                    Results = new NamedCollection<IArgumentDeclaration>(),
                    Implementation = new ExpressionBlock()
                }
            },
            new ParseFunctionDeclTestData {
                Name = "operator with args",
                Context = new Context {Parent = TestContext},
                Input = new[] {
                    Id(text: "fn"),
                    BracketOpen(),
                    Id(text: "leftarg"), Op(text: ":"), Id(text: "u64"),
                    BracketClose(),
                    Op(text: "+"),
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
                    Name = "+",
                    LeftArguments = new NamedCollection<IArgumentDeclaration> {
                        new ArgumentDeclaration {
                            Name = "leftarg",
                            Type = TestContext.Identifiers[key: "u64"] as IModuleInstance
                        }
                    },
                    RightArguments = new NamedCollection<IArgumentDeclaration> {
                        new ArgumentDeclaration {
                            Name = "rightarg",
                            Type = TestContext.Identifiers[key: "u64"] as IModuleInstance
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
                var functionDecl = FunctionDeclParser.Parse(it, data.Context, done: ref done);
                Assert.IsTrue(done);
                AssertFunctionDecl(data.Output, (dynamic) functionDecl);
            }
        }

        void AssertFunctionDecl(IFunctionDeclaration expected, IFunctionDeclaration actual) {
            Assert.AreEqual(expected.Name, actual.Name);
            //Assert.AreEqual(expected.IsRuntimeUsable, actual.IsRuntimeUsable);
            //Assert.AreEqual(expected.IsCompileTimeUsable, actual.IsCompileTimeUsable);
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
