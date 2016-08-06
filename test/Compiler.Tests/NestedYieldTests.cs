using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;

namespace REC.Tests
{
    [TestFixture()]
    public class NestedYieldTests
    {
        private enum Token
        {
            VarDecl,
            Call,
            Identifier
        }

        private class ParserMoc
        {
            public int State { get; private set; }

            public IEnumerable<Token> Parse() {
                return Flatten(ParseNested());
            }

            private static IEnumerable<Token> Flatten(Token t) {
                yield return t;
            }
            private static IEnumerable<Token> Flatten(IEnumerable<dynamic> d) {
                return d.SelectMany<dynamic, Token>(i => Flatten(i));
            }

            private static IEnumerable<dynamic> ToEnumerable(IEnumerator<Token> d) {
                while (d.MoveNext()) yield return d.Current;
            }

            private IEnumerable<dynamic> ParseNested() {
                State = 1;
                yield return Token.VarDecl;
                var x = Flatten(ParseVarDecl()).GetEnumerator();
                if (x.MoveNext()) {
                    yield return x.Current;
                    yield return ToEnumerable(x);
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
            //Assert.AreEqual(new List<Token> {Token.VarDecl, Token.Identifier}, x);
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