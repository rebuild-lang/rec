using NUnit.Framework;
using REC.AST;
using REC.Instance;
using REC.Intrinsic;
using REC.Intrinsic.Types;
using REC.Parser;
using REC.Scanner;
using System;
using System.Collections.Generic;

namespace REC.Tests.Parser
{
    using static TokenHelpers;

    [TestFixture]
    public class VariableDeclParserTests
    {
        public struct ParseVariableDeclTestData
        {
            public string Name;
            public IContext Context;
            public IEnumerable<TokenData> Input;
            public IVariableDeclaration Output;

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

        static readonly ParseVariableDeclTestData[] ParseVariableDeclTests = {
            new ParseVariableDeclTestData {
                Name = "uninitialized simple type",
                Context = new Context {Parent = TestContext},
                Input = new[] {
                    Id(text: "let"), Id(text: "x"), Op(text: ":"), Id(text: "u64")
                },
                Output = new VariableDeclaration {
                    Name = "x",
                    Type = TestContext.Identifiers[key: "u64"] as IModuleInstance
                }
            },
        };

        [TestCaseSource(nameof(ParseVariableDeclTests))]
        public void ParseFunctionDeclTest(ParseVariableDeclTestData data) {
            using (var it = data.Input.GetIterator()) {
                var variableDecl = VariableDeclParser.Parse(it, data.Context);
                Assert.That(it.Done, Is.True);
                AssertVariableDecl(data.Output, (dynamic) variableDecl);
                AssertVariableInstance(data.Output, (dynamic) data.Context.Identifiers[data.Output.Name]);
            }
        }

        static void AssertVariableDecl(IVariableDeclaration expected, INamedExpressionTuple actual) {
            Assert.That(actual.Tuple, Has.Count.EqualTo(expected: 1));
            foreach (var namedExpression in actual.Tuple) {
                AssertVariableDecl(expected, namedExpression.Expression as IVariableDeclaration);
            }
        }

        static void AssertVariableInstance(IVariableDeclaration expected, IVariableInstance actual) {
            Assert.That(actual.Name, Is.EqualTo(expected.Name));
            Assert.That(actual.Type, Is.SameAs(expected.Type));
            AssertVariableDecl(expected, actual.Variable);
        }

        static void AssertVariableDecl(IVariableDeclaration expected, IVariableDeclaration actual) {
            Assert.That(actual.Name, Is.EqualTo(expected.Name));
            Assert.That(actual.IsAssignable, Is.EqualTo(expected.IsAssignable));
            Assert.That(actual.Type, Is.SameAs(expected.Type));
        }
    }
}
