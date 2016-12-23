using System.Collections.Generic;
using NUnit.Framework;
using REC.Tools;

namespace REC.Tests.Experiments
{
    [TestFixture]
    public class NestedYieldTests
    {
        // This test shows how a nested yield generator can be flattened
        // ReSharper disable once MemberCanBePrivate.Global
        public enum Token
        {
            VarDecl,
            Call,
            Identifier
        }

        class ParserMoc
        {
            public int State { get; private set; }

            public IEnumerable<Token> Parse() {
                return ParseNested().FlattenTo<Token>();
            }

            IEnumerable<dynamic> ParseNested() {
                State = 1;
                yield return Token.VarDecl;
                var x = ParseVarDecl().GetFlatEnumerator<Token>();
                if (x.MoveNext()) {
                    yield return x.Current;
                    yield return x;
                }
                State = 4;
            }

            IEnumerable<dynamic> ParseVarDecl() {
                State = 2;
                yield return Token.Identifier;
                yield return ParseCall();
            }

            IEnumerable<dynamic> ParseCall() {
                State = 3;
                yield return Token.Call;
            }
        }

        [Test]
        public void PeekCharStartTest() {
            var p = new ParserMoc();
            //var x = p.Parse().ToList();
            //Assert.AreEqual(new Expressions<Token> {Token.VarDecl, Token.entry}, x);
            using (var i = p.Parse().GetEnumerator()) {
                Assert.AreEqual(expected: 0, actual: p.State);

                Assert.IsTrue(i.MoveNext());
                Assert.AreEqual(expected: 1, actual: p.State);
                Assert.AreEqual(Token.VarDecl, i.Current);

                Assert.IsTrue(i.MoveNext());
                Assert.AreEqual(expected: 2, actual: p.State);
                Assert.AreEqual(Token.Identifier, i.Current);

                Assert.IsTrue(i.MoveNext());
                Assert.AreEqual(expected: 3, actual: p.State);
                Assert.AreEqual(Token.Call, i.Current);

                Assert.IsFalse(i.MoveNext());
                Assert.AreEqual(expected: 4, actual: p.State);
            }
        }
    }
}
