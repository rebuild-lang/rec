using System;
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
            public IList<TokenData> Input;
            public IFunctionDeclaration Output;

            public override string ToString() => Name;
        }

        static readonly IContext TestContext = new Func<IContext>(
            () => {
                var context = new Context();
                DeclarationConverter.BuildContext(
                    context,
                    new IntrinsicDict {
                        U64Type.Get(),
                    });
                return context;
            }
        )();

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
                    Id(text: "arg"), Colon(), Id(text: "u64"),
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
                    Id(text: "leftarg"), Colon(), Id(text: "u64"),
                    BracketClose(),
                    Id(text: "a"),
                    BracketOpen(),
                    Id(text: "rightarg"), Colon(), Id(text: "u64"),
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
                    Id(text: "leftarg"), Colon(), Id(text: "u64"),
                    BracketClose(),
                    Id(text: "+"),
                    BracketOpen(),
                    Id(text: "rightarg"), Colon(), Id(text: "u64"),
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
            using (var it = data.Input.GetIterator()) {
                var functionDecl = FunctionDeclParser.Parse(it, data.Context);
                Assert.That(it.Done, Is.True);
                AssertFunctionDecl(data.Output, (dynamic) functionDecl);
                AssertFunctionInstance(data.Output, (dynamic)data.Context.Identifiers[data.Output.Name]);
            }
        }

        static void AssertFunctionDecl(IFunctionDeclaration expected, IFunctionDeclaration actual) {
            Assert.That(actual.Name, Is.EqualTo(expected.Name));
            //Assert.That(actual.IsRuntimeUsable, Is.EqualTo(expected.IsRuntimeUsable));
            //Assert.That(actual.IsCompileTimeUsable, Is.EqualTo(expected.IsCompileTimeUsable));
            AssertArgumentsDecl(expected.LeftArguments, actual.LeftArguments, name: "Left");
            AssertArgumentsDecl(expected.RightArguments, actual.RightArguments, name: "Right");
            AssertArgumentsDecl(expected.Results, actual.Results, name: "Results");
        }

        static void AssertFunctionInstance(IFunctionDeclaration expected, IFunctionOverloads actual)
        {
            Assert.That(actual.Name, Is.EqualTo(expected.Name));
            Assert.That(actual.Overloads, Has.Count.EqualTo(expected: 1));
            AssertFunctionInstance(expected, actual.Overloads[index: 0]);
        }

        static void AssertFunctionInstance(IFunctionDeclaration expected, IFunctionInstance actual)
        {
            Assert.That(actual.Name, Is.EqualTo(expected.Name));
            AssertArgumentsInstance(expected.LeftArguments, actual.LeftArguments, ArgumentSide.Left, name: "instance.LeftArguments");
            AssertArgumentsInstance(expected.RightArguments, actual.RightArguments, ArgumentSide.Right, name: "instance.RightArguments");
            AssertArgumentsInstance(expected.Results, actual.Results, ArgumentSide.Result, name: "instance.Results");
            AssertFunctionDecl(expected, actual.Declaration);
        }

        static void AssertArgumentsDecl(NamedCollection<IArgumentDeclaration> expected, NamedCollection<IArgumentDeclaration> actual, string name) {
            Assert.That(actual, Has.Count.EqualTo(expected.Count), $"{name}Arguments.Count");
            for (var i = 0; i < expected.Count; i++) AssertArgumentDecl(expected[i], actual[i], $"{name}Arguments[{i}]");
        }

        static void AssertArgumentsInstance(NamedCollection<IArgumentDeclaration> expected, NamedCollection<IArgumentInstance> actual, ArgumentSide side, string name)
        {
            Assert.That(actual, Has.Count.EqualTo(expected.Count), $"{name}Arguments.Count");
            for (var i = 0; i < expected.Count; i++) AssertArgumentInstance(expected[i], actual[i], side, $"{name}Arguments[{i}]");
        }

        static void AssertArgumentDecl(IArgumentDeclaration expected, IArgumentDeclaration actual, string name) {
            Assert.That(actual.Name, Is.EqualTo(expected.Name), $"{name}.Name");
            Assert.That(actual.IsAssignable, Is.EqualTo(expected.IsAssignable), $"{name}.IsAssignable");
            Assert.That(actual.IsUnrolled, Is.EqualTo(expected.IsUnrolled), $"{name}.IsUnrolled");
            Assert.That(actual.Type, Is.SameAs(expected.Type), $"{name}.Type");
        }

        static void AssertArgumentInstance(IArgumentDeclaration expected, IArgumentInstance actual, ArgumentSide side, string name)
        {
            Assert.That(actual.Name, Is.EqualTo(expected.Name), $"{name}.Name");
            Assert.That(actual.Type, Is.SameAs(expected.Type), $"{name}.Type");
            Assert.That(actual.Side, Is.EqualTo(side), $"{name}.SIde");
            AssertArgumentDecl(expected, actual.Argument, $"{name}.Argument");
        }
    }
}
