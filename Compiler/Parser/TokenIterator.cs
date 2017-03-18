using System;
using System.Collections.Generic;
using REC.Instance;
using REC.Scanner;

namespace REC.Parser
{
    public interface ITokenIterator : IDisposable
    {
        TokenData Current { get; }
        IInstance CurrentInstance { get; set; }
        TokenData Next { get; }
        bool Active { get; set; }
        bool Done { get; }
        bool HasNext { get; }

        bool MoveNext();
    }

    class TokenIterator : ITokenIterator
    {
        public IEnumerator<TokenData> Enumerator { get; }
        public bool Active { get; set; }

        public TokenData Current { get; set; }
        public IInstance CurrentInstance { get; set; }
        public bool Done => !Active;
        public bool HasNext { get; set; }
        public TokenData Next => Enumerator.Current;

        public TokenIterator(IEnumerable<TokenData> enumerable) {
            Enumerator = enumerable.GetEnumerator();
            HasNext = Enumerator.MoveNext();
            MoveNext();
        }

        public void Dispose() {
            Enumerator.Dispose();
        }

        public bool MoveNext() {
            CurrentInstance = null;
            Active = HasNext;
            if (Active) {
                Current = Next;
                HasNext = Enumerator.MoveNext();
            }
            else Current = new TokenData();
            return Active;
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