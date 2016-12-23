using System.Collections.Generic;
using NUnit.Framework;
using REC.AST;
using REC.Intrinsic;
using REC.Intrinsic.IO;
using REC.Intrinsic.Types;
using REC.Intrinsic.Types.API;
using REC.Parser;
using REC.Scanner;
using REC.Scope;
using REC.Tools;

namespace REC.Tests.Parser
{
    [TestFixture]
    public class ExpressionParserTests
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

        static TokenData NumberLit(string text) {
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


        static IScope BuildTestScope() {
            var scope = new REC.Parser.Scope();
            DeclarationConverter.BuildScope(
                scope,
                new IntrinsicDict {
                    U64Type.Get(),
                    NumberLiteralType.Get(),
                    SimpleMathIntrinsic<ulong, UlongMath>.Get()
                });
            return scope;
        }

        static readonly IScope TestScope = BuildTestScope();

        static readonly IScope TestVariableResScope = new REC.Parser.Scope {
            Parent = TestScope,
            Identifiers = {
                new VariableEntry {
                    Variable = new VariableDeclaration {
                        Name = "res",
                        Type = (TestScope.Identifiers[key: "u64"] as IModuleEntry)?.ModuleDeclaration,
                        IsAssignable = true,
                    }
                }
            }
        };

        public struct ParseTestData
        {
            public string Name;
            public IScope Scope;
            public IEnumerable<TokenData> Input;
            public INamedExpressionTuple Output;

            public override string ToString() => Name;
        }

        static readonly ParseTestData[] ParseExpressionTestData = {
            new ParseTestData {
                Name = "Compile time Add",
                Scope = new REC.Parser.Scope {Parent = TestScope},
                Input = new[] {
                    Op(text: "&"),
                    Id(text: "Add"), NumberLit(text: "3"), NumberLit(text: "20")
                },
                Output = new NamedExpressionTuple {
                    Tuple = {
                        new NamedExpression {
                            Name = "Value",
                            Expression = new TypedValue {
                                Type = (TestScope.Identifiers[key: "u64"] as IModuleEntry)?.ModuleDeclaration,
                                Data = new NumberLiteral {IntegerPart = "23"}.ToUnsigned(byteCount: 8)
                            }
                        }
                    }
                }
            },
            new ParseTestData {
                Name = "Assign and Add",
                Scope = new REC.Parser.Scope {
                    Parent = TestVariableResScope,
                },
                Input = new[] {
                    Id(text: "Assign"),
                    Id(text: "res"),
                    Id(text: "Add"), NumberLit(text: "3"), NumberLit(text: "20")
                },
                Output = new NamedExpressionTuple {
                    Tuple = {
                        new NamedExpression {
                            Expression = new FunctionInvocation {
                                Function = (TestScope.Identifiers[key: "Assign"] as IFunctionEntry)?.FunctionDeclarations?.First(),
                                Left = new NamedExpressionTuple(),
                                Right = new NamedExpressionTuple {
                                    Tuple = {
                                        new NamedExpression {
                                            Expression = new TypedReference {
                                                Declaration = (TestVariableResScope.Identifiers[key: "res"] as IVariableEntry)?.Variable,
                                                Type = (TestScope.Identifiers[key: "u64"] as IModuleEntry)?.ModuleDeclaration,
                                            }
                                        },
                                        new NamedExpression {
                                            Expression = new FunctionInvocation {
                                                Function = (TestScope.Identifiers[key: "Add"] as IFunctionEntry)?.FunctionDeclarations?.First(),
                                                Left = new NamedExpressionTuple(),
                                                Right = new NamedExpressionTuple {
                                                    Tuple = {
                                                        new NamedExpression {
                                                            Expression = new NumberLiteral {
                                                                IntegerPart = "3"
                                                            }
                                                        },
                                                        new NamedExpression {
                                                            Expression = new NumberLiteral {
                                                                IntegerPart = "20",
                                                            }
                                                        },
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            },
        };

        [TestCaseSource(nameof(ParseExpressionTestData))]
        public void ParseExpressionTest(ParseTestData data) {
            using (var it = data.Input.GetEnumerator()) {
                it.MoveNext();
                var done = false;
                var result = ExpressionParser.Parse(it, data.Scope, done: ref done);
                Assert.IsTrue(done);
                AssertNamedExpressionTuple(data.Output, result);
            }
        }

        void AssertNamedExpressionTuple(INamedExpressionTuple expected, INamedExpressionTuple actual, string label = "") {
            Assert.AreEqual(expected.Tuple.Count, actual.Tuple.Count, $"{label}.Tuple.Count");
            for (var i = 0; i < expected.Tuple.Count; i++) {
                AssertNamedExpression(expected.Tuple[i], actual.Tuple[i], label: $"{label}.Tuple[{i}]");
            }
        }

        void AssertNamedExpression(INamedExpression expected, INamedExpression actual, string label) {
            Assert.AreEqual(expected.Name, actual.Name, $"{label}.Name");
            Assert.AreEqual(expected.Expression?.GetType(), actual.Expression?.GetType(), $"{label}.Expression.Type");
            AssertExpression((dynamic) expected.Expression, (dynamic) actual.Expression, label: $"{label}.Expression");
        }

        void AssertExpression(ITypedValue expected, ITypedValue actual, string label) {
            Assert.AreEqual(expected.Type, actual.Type, $"{label}.Type");
            Assert.AreEqual(expected.Data, actual.Data, $"{label}.Data");
        }

        void AssertExpression(IFunctionInvocation expected, IFunctionInvocation actual, string label) {
            Assert.AreEqual(expected.Function, actual.Function, $"{label}.Function");
            AssertNamedExpressionTuple(expected.Left, actual.Left, label: $"{label}.Left");
            AssertNamedExpressionTuple(expected.Right, actual.Right, label: $"{label}.Right");
        }

        void AssertExpression(ITypedReference expected, ITypedReference actual, string label) {
            Assert.AreEqual(expected.Type, actual.Type, $"{label}.Type");
            Assert.AreEqual(expected.Declaration, actual.Declaration, $"{label}.Declaration");
        }

        void AssertExpression(INumberLiteral expected, INumberLiteral actual, string label)
        {
            Assert.AreEqual(expected.IntegerPart, actual.IntegerPart, $"{label}.IntegerPart");
        }
    }
}
