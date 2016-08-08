using System.Collections.Generic;
using NUnit.Framework;
using REC.Tools;

namespace REC.Tests
{
    [TestFixture()]
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

        private class ParserMoc
        {
            public int State { get; private set; }

            public IEnumerable<Token> Parse() {
                return ParseNested().FlattenTo<Token>();
            }

            private IEnumerable<dynamic> ParseNested() {
                State = 1;
                yield return Token.VarDecl;
                var x = ParseVarDecl().GetFlatEnumerator<Token>();
                if (x.MoveNext()) {
                    yield return x.Current;
                    yield return x;
                }
                State = 4;
            }

            private IEnumerable<dynamic> ParseVarDecl() {
                State = 2;
                yield return Token.Identifier;
                yield return ParseCall();
            }

            private IEnumerable<dynamic> ParseCall() {
                State = 3;
                yield return Token.Call;
            }
        }

        [Test()]
        public void PeekCharStartTest() {
            var p = new ParserMoc();
            //var x = p.Parse().ToList();
            //Assert.AreEqual(new Expressions<Token> {Token.VarDecl, Token.entry}, x);
            var i = p.Parse().GetEnumerator();
            Assert.AreEqual(0, p.State);

            Assert.IsTrue(i.MoveNext());
            Assert.AreEqual(1, p.State);
            Assert.AreEqual(Token.VarDecl, i.Current);

            Assert.IsTrue(i.MoveNext());
            Assert.AreEqual(2, p.State);
            Assert.AreEqual(Token.Identifier, i.Current);

            Assert.IsTrue(i.MoveNext());
            Assert.AreEqual(3, p.State);
            Assert.AreEqual(Token.Call, i.Current);

            Assert.IsFalse(i.MoveNext());
            Assert.AreEqual(4, p.State);
        }
    }
}