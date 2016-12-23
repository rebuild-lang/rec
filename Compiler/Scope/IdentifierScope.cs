using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using REC.Tools;

namespace REC.Scope
{
    using Dict = Dictionary<string, IEntry>;
    using ReadDict = ReadOnlyDictionary<string, IEntry>;

    // The identifierScope is used for name lookups during parsing
    public interface IIdentifierScope : IEnumerable<IEntry>
    {
        IIdentifierScope Parent { get; }

        ReadDict Locals { get; }

        IEntry this[string key] { get; }

        bool Add(IEntry entry);
    }

    class IdentifierScope : IIdentifierScope
    {
        readonly Dict _locals = new Dict();

        public IIdentifierScope Parent { get; set; }
        public ReadDict Locals => new ReadDict(_locals);

        public IEntry this[string key] => _locals.GetOr(key, () => Parent?[key]);

        public bool Add(IEntry entry) {
            var label = entry.Name;
            if (_locals.ContainsKey(label))
                return false;
            _locals.Add(label, entry);
            return true;
        }

        public IEnumerator<IEntry> GetEnumerator() {
            return new Enumerator(this);
        }

        IEnumerator IEnumerable.GetEnumerator() {
            return GetEnumerator();
        }

        struct Enumerator : IEnumerator<IEntry>
        {
            readonly IdentifierScope _startIdentifierScope;
            IdentifierScope _identifierScope;
            Dict.ValueCollection.Enumerator _enumerator;

            internal Enumerator(IdentifierScope identifierScope) : this() {
                _startIdentifierScope = identifierScope;
                Reset();
            }

            public bool MoveNext() {
                while (!_enumerator.MoveNext()) {
                    _identifierScope = (IdentifierScope) _identifierScope?.Parent;
                    if (null == _identifierScope) return false;
                    _enumerator = _identifierScope._locals.Values.GetEnumerator();
                }
                return true;
            }

            public void Reset() {
                _identifierScope = _startIdentifierScope;
                _enumerator = _identifierScope._locals.Values.GetEnumerator();
            }

            public IEntry Current => _enumerator.Current;

            object IEnumerator.Current => Current;

            public void Dispose() {}
        }
    }
}
