using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;

namespace REC.Tests.Experiments
{
    [TestFixture]
    public class GeneratorTests
    {
        // This test models an explicit generator
        interface IGenerator<out T>
        {
            IReadOnlyCollection<T> Build();
        }

        class GeneratorEnumerator<T> : IEnumerator<T>
        {
            IEnumerator<T> _enumerator;

            public GeneratorEnumerator(IGenerator<T> generator) {
                Generator = generator;
            }

            IGenerator<T> Generator { get; }
            public T Current => _enumerator.Current;

            object IEnumerator.Current => Current;

            public void Dispose() {}

            public bool MoveNext() {
                if (_enumerator != null) {
                    var hasNext = _enumerator.MoveNext();
                    if (hasNext) return true;
                }
                var next = Generator.Build() ?? new List<T>();
                _enumerator = next.GetEnumerator();
                return _enumerator.MoveNext();
            }

            public void Reset() {
                throw new NotImplementedException();
            }
        }

        abstract class AEnumerableGenerator<T> : IEnumerable<T>, IGenerator<T>
        {
            public IEnumerator<T> GetEnumerator() {
                return new GeneratorEnumerator<T>(this);
            }

            IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
            public abstract IReadOnlyCollection<T> Build();
        }

        enum Token
        {
            VarDecl,
            Call,
            Identifier
        }

        class ParserMoc : AEnumerableGenerator<Token>
        {
            int State { get; set; }


            public override IReadOnlyCollection<Token> Build() {
                switch (State) {
                    case 0:
                        State = 1;
                        return new[] {Token.VarDecl, Token.Identifier};
                    case 1:
                        State = 2;
                        return new[] {Token.Call};
                    default:
                        return null;
                }
            }
        }


        [Test]
        public void TestParserMoc() {
            var p = new ParserMoc();
            Assert.AreEqual(new List<Token> {Token.VarDecl, Token.Identifier, Token.Call}, p.ToList());
        }
    }
}
