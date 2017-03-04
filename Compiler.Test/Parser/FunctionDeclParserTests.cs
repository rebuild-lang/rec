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
    using static TokenHelpers;

    [TestFixture]
    public class FunctionDeclParserTests
    {
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
                Assert.That(done, Is.True);
                AssertFunctionDecl(data.Output, (dynamic) functionDecl);
            }
        }

        void AssertFunctionDecl(IFunctionDeclaration expected, IFunctionDeclaration actual) {
            Assert.That(actual.Name, Is.EqualTo(expected.Name));
            //Assert.That(actual.IsRuntimeUsable, Is.EqualTo(expected.IsRuntimeUsable));
            //Assert.That(actual.IsCompileTimeUsable, Is.EqualTo(expected.IsCompileTimeUsable));
            AssertArguments(expected.LeftArguments, actual.LeftArguments, name: "Left");
            AssertArguments(expected.RightArguments, actual.RightArguments, name: "Right");
            AssertArguments(expected.Results, actual.Results, name: "Results");
        }

        void AssertArguments(NamedCollection<IArgumentDeclaration> expected, NamedCollection<IArgumentDeclaration> actual, string name) {
            Assert.That(actual.Count, Is.EqualTo(expected.Count), $"{name}Arguments.Count");
            for (var i = 0; i < expected.Count; i++) AssertArgument(expected[i], actual[i], name: $"{name}Arguments[{i}]");
        }

        void AssertArgument(IArgumentDeclaration expected, IArgumentDeclaration actual, string name) {
            Assert.That(actual.Name, Is.EqualTo(expected.Name), $"{name}.Name");
            Assert.That(actual.IsAssignable, Is.EqualTo(expected.IsAssignable), $"{name}.IsAssignable");
            Assert.That(actual.IsUnrolled, Is.EqualTo(expected.IsUnrolled), $"{name}.IsUnrolled");
            Assert.That(actual.Type.Name, Is.EqualTo(expected.Type.Name), $"{name}.Type.Name");
        }
    }
}
