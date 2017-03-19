using System;
using System.Collections.Generic;
using REC.Instance;
using REC.Scanner;

namespace REC.Parser
{
    public interface ITokenIteratorBackup
    {}

    public interface ITokenIterator : IDisposable
    {
        IInstance CurrentInstance { get; set; }
        TokenData Current { get; }
        TokenData Next { get; }
        bool Active { get; }
        bool Done { get; }
        bool HasNext { get; }

        bool MoveNext();

        ITokenIteratorBackup Backup();
        void Restore(ITokenIteratorBackup iteratorBackup);
    }

    class TokenIterator : ITokenIterator
    {
        IList<TokenData> List { get; }
        int Index { get; set; }

        public IInstance CurrentInstance { get; set; }
        public TokenData Current => List[Index];
        public TokenData Next => List[Index + 1];
        public bool Active => Index < List.Count;
        public bool Done => !Active;
        public bool HasNext => Index + 1 < List.Count;

        public TokenIterator(IList<TokenData> list) {
            List = list;
        }

        public void Dispose() { }

        public bool MoveNext() {
            CurrentInstance = null;
            Index++;
            return Active;
        }

        public ITokenIteratorBackup Backup() {
            return new IteratorIteratorBackup { Index = Index };
        }

        public void Restore(ITokenIteratorBackup iteratorBackup) {
            Index = ((IteratorIteratorBackup)iteratorBackup).Index;
        }

        class IteratorIteratorBackup : ITokenIteratorBackup
        {
            internal int Index { get; set; }
        }
    }

    static class EnumerableTokenDataExt
    {
        public static TokenIterator GetIterator(this IList<TokenData> enumerable) {
            return new TokenIterator(enumerable);
        }
    }
}
