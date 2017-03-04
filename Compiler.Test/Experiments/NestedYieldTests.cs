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
                using (var x = ParseVarDecl().FlattenTo<Token>().GetEnumerator()) {
                    if (x.MoveNext()) {
                        yield return x.Current;
                        yield return x;
                    }
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
            //Assert.That(x, Is.EqualTo(new Expressions<Token> {Token.VarDecl, Token.entry}));
            using (var i = p.Parse().GetEnumerator()) {
                Assert.That(p.State, Is.Zero);

                Assert.That(i.MoveNext(), Is.True);
                Assert.That(p.State, Is.EqualTo(expected: 1));
                Assert.That(i.Current, Is.EqualTo(Token.VarDecl));

                Assert.That(i.MoveNext(), Is.True);
                Assert.That(p.State, Is.EqualTo(expected: 2));
                Assert.That(i.Current, Is.EqualTo(Token.Identifier));

                Assert.That(i.MoveNext(), Is.True);
                Assert.That(p.State, Is.EqualTo(expected: 3));
                Assert.That(i.Current, Is.EqualTo(Token.Call));

                Assert.That(i.MoveNext(), Is.False);
                Assert.That(p.State, Is.EqualTo(expected: 4));
            }
        }
    }
}
