using System;
using NUnit.Framework;
using REC.AST;
using REC.Instance;
using REC.Intrinsic;
using REC.Intrinsic.IO;
using REC.Intrinsic.Types;
using REC.Intrinsic.Types.API;
using REC.Parser;
using REC.Scanner;
using System.Collections.Generic;
using REC.Scope;
using TypedReference = REC.AST.TypedReference;

namespace REC.Tests.Parser
{
    [TestFixture]
    public class ExpressionParserTests : TokenHelpers
    {
        static readonly IContext TestContext = new Func<IContext>(
            () => {
                var context = new Context();
                DeclarationConverter.BuildContext(
                    context,
                    new IntrinsicDict {
                        U64Type.Get(),
                        NumberLiteralType.Get(),
                        SimpleMathIntrinsic<ulong, UlongMath>.Get()
                    });
                return context;
            })();

        static readonly IContext TestVariableResContext = new Context {
            Parent = TestContext,
            Identifiers = {
                new VariableInstance {
                    Variable = new VariableDeclaration {
                        Name = "res",
                        Type = (TestContext.Identifiers[key: "u64"] as IModuleInstance),
                        IsAssignable = true,
                    }
                }
            }
        };

        static IFunctionOverloads ToFunctionInstance(params IFunctionDeclaration[] declarations) {
            var result = new FunctionOverloads();
            foreach (var declaration in declarations) {
                var instance = new FunctionInstance(declaration);
                ToArgumentInstances(instance.LeftArguments, instance.ArgumentIdentifiers, declaration.LeftArguments, ArgumentSide.Left);
                ToArgumentInstances(instance.RightArguments, instance.ArgumentIdentifiers, declaration.RightArguments, ArgumentSide.Right);
                ToArgumentInstances(instance.Results, instance.ArgumentIdentifiers, declaration.Results, ArgumentSide.Result);
                result.Overloads.Add(instance);
            }
            return result;
        }

        static void ToArgumentInstances(
            ICollection<IArgumentInstance> instances,
            ILocalIdentifierScope scope,
            IEnumerable<IArgumentDeclaration> declarations,
            ArgumentSide side) {
            foreach (var declaration in declarations) {
                var instance = new ArgumentInstance {Argument = declaration, Side = side};
                scope.Add(instance);
                instances.Add(instance);
            }
        }

        static readonly IContext TestOperatorContext = new Context {
            Parent = TestContext,
            Identifiers = {
                new VariableInstance {
                    Variable = new VariableDeclaration {
                        Name = "i",
                        Type = (TestContext.Identifiers[key: "u64"] as IModuleInstance),
                        IsAssignable = true,
                    }
                },
                ToFunctionInstance(
                    new FunctionDeclaration {
                        LeftArguments = {
                            new ArgumentDeclaration {Type = (TestContext.Identifiers[key: "u64"] as IModuleInstance)},
                        },
                        Name = "++"
                    }),
                ToFunctionInstance(
                    new FunctionDeclaration {
                        LeftArguments = {
                            new ArgumentDeclaration {Type = (TestContext.Identifiers[key: "u64"] as IModuleInstance)},
                        },
                        RightArguments = {
                            new ArgumentDeclaration {Type = (TestContext.Identifiers[key: "u64"] as IModuleInstance)},
                        },
                        Name = "-"
                    })
            }
        };

        public struct ParseTestData
        {
            public string Name;
            public IContext Context;
            public IEnumerable<TokenData> Input;
            public INamedExpressionTuple Output;

            public override string ToString() => Name;
        }

        static readonly ParseTestData[] ParseExpressionTestData = {
            new ParseTestData {
                Name = "Compile time Add",
                Context = new Context {Parent = TestContext},
                Input = new[] {
                    Op(text: "&"),
                    Id(text: "Add"), NumberLit(text: "3"), NumberLit(text: "20")
                },
                Output = new NamedExpressionTuple(
                    name: "Value",
                    expression: new TypedValue {
                        Type = (TestContext.Identifiers[key: "u64"] as IModuleInstance),
                        Data = new NumberLiteral {IntegerPart = "23"}.ToUnsigned(byteCount: 8)
                    }
                )
            },
            new ParseTestData {
                Name = "split multiple operators ''i++-1''",
                Context = new Context {Parent = TestOperatorContext},
                Input = new[] {
                    Id(text: "i"), Op(text: "++-"), NumberLit(text: "1")
                },
                Output = new NamedExpressionTuple(
                    new FunctionInvocation {
                        Function = (TestOperatorContext.Identifiers[key: "-"] as IFunctionOverloads)?.Overloads[index: 0],
                        Left = new NamedExpressionTuple(
                            new FunctionInvocation {
                                Function = (TestOperatorContext.Identifiers[key: "++"] as IFunctionOverloads)?.Overloads[index: 0],
                                Left = new NamedExpressionTuple(
                                    new TypedReference {
                                        Type = (TestContext.Identifiers[key: "u64"] as IModuleInstance),
                                        Instance = (TestOperatorContext.Identifiers[key: "i"] as IVariableInstance)
                                    }
                                ),
                                Right = new NamedExpressionTuple(),
                            }
                        ),
                        Right = new NamedExpressionTuple(
                            new NumberLiteral {
                                IntegerPart = "1"
                            }
                        ),
                    }
                )
            },
            new ParseTestData {
                Name = "Assign and Add",
                Context = new Context {
                    Parent = TestVariableResContext,
                },
                Input = new[] {
                    Id(text: "Assign"),
                    Id(text: "res"),
                    Id(text: "Add"), NumberLit(text: "3"), NumberLit(text: "20")
                },
                Output = new NamedExpressionTuple(
                    new FunctionInvocation {
                        Function = TestContext.Identifiers[key: "Assign"] as IFunctionInstance,
                        Left = new NamedExpressionTuple(),
                        Right = new NamedExpressionTuple(
                            new TypedReference {
                                Instance = (TestVariableResContext.Identifiers[key: "res"] as IVariableInstance),
                                Type = TestContext.Identifiers[key: "u64"] as IModuleInstance,
                            },
                            new FunctionInvocation {
                                Function = TestContext.Identifiers[key: "Add"] as IFunctionInstance,
                                Left = new NamedExpressionTuple(),
                                Right = new NamedExpressionTuple(
                                    new NumberLiteral {IntegerPart = "3"},
                                    new NumberLiteral {IntegerPart = "20"}
                                )
                            })
                    }
                )
            },
        };

        [TestCaseSource(nameof(ParseExpressionTestData))]
        public void ParseExpressionTest(ParseTestData data) {
            using (var it = data.Input.GetEnumerator()) {
                it.MoveNext();
                var done = false;
                var result = ExpressionParser.Parse(it, data.Context, ref done);
                Assert.IsTrue(done);
                AssertNamedExpressionTuple(data.Output, result);
            }
        }

        void AssertNamedExpressionTuple(INamedExpressionTuple expected, INamedExpressionTuple actual, string label = "") {
            Assert.AreEqual(expected.Tuple.Count, actual.Tuple.Count, $"{label}.Tuple.Count");
            for (var i = 0; i < expected.Tuple.Count; i++) {
                AssertNamedExpression(expected.Tuple[i], actual.Tuple[i], $"{label}.Tuple[{i}]");
            }
        }

        void AssertNamedExpression(INamedExpression expected, INamedExpression actual, string label) {
            Assert.AreEqual(expected.Name, actual.Name, $"{label}.Name");
            Assert.AreEqual(expected.Expression?.GetType(), actual.Expression?.GetType(), $"{label}.Expression.Type");
            AssertExpression((dynamic) expected.Expression, (dynamic) actual.Expression, $"{label}.Expression");
        }

        void AssertExpression(ITypedValue expected, ITypedValue actual, string label) {
            Assert.AreEqual(expected.Type, actual.Type, $"{label}.Type");
            Assert.AreEqual(expected.Data, actual.Data, $"{label}.Data");
        }

        void AssertExpression(IFunctionInvocation expected, IFunctionInvocation actual, string label) {
            Assert.AreEqual(expected.Function, actual.Function, $"{label}.Function");
            AssertNamedExpressionTuple(expected.Left, actual.Left, $"{label}.Left");
            AssertNamedExpressionTuple(expected.Right, actual.Right, $"{label}.Right");
        }

        void AssertExpression(ITypedReference expected, ITypedReference actual, string label) {
            Assert.AreEqual(expected.Type, actual.Type, $"{label}.Type");
            Assert.AreEqual(expected.Instance, actual.Instance, $"{label}.Instance");
        }

        void AssertExpression(INumberLiteral expected, INumberLiteral actual, string label) {
            Assert.AreEqual(expected.IntegerPart, actual.IntegerPart, $"{label}.IntegerPart");
        }
    }
}
