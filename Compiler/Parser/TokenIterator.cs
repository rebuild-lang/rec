using System;
using System.Collections.Generic;
using REC.Scanner;

namespace REC.Parser
{
    public interface ITokenIterator : IDisposable
    {
        IEnumerator<TokenData> Enumerator { get; }
        TokenData Current { get; }
        bool Active { get; set; }
        bool Done { get; }

        bool MoveNext();
    }

    class TokenIterator : ITokenIterator
    {
        public IEnumerator<TokenData> Enumerator { get; }
        public bool Active { get; set; }

        public TokenData Current => Enumerator.Current;
        public bool Done => !Active;

        public TokenIterator(IEnumerable<TokenData> enumerable) {
            Enumerator = enumerable.GetEnumerator();
            MoveNext();
        }

        public void Dispose() {
            Enumerator.Dispose();
        }

        public bool MoveNext() {
            return Active = Enumerator.MoveNext();
        }
    }

    static class EnumerableTokenDataExt
    {
        public static TokenIterator GetIterator(this IEnumerable<TokenData> enumerable)
        {
            return new TokenIterator(enumerable);
        }
    }
}