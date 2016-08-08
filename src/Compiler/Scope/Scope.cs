using System;
using System.Collections;
using System.Collections.Generic;
using REC.Tools;

namespace REC.Scope
{
    using Dict = Dictionary<string, IEntry>;

    public delegate void ScopedIdentifierAdded(IScope scope, IEntry entry);
    public delegate void ScopeDisposed(IScope scope);

    // The scope is used for name lookups during parsing
    public interface IScope : IDisposable, IEnumerable<IEntry>
    {
        IScope Parent { get; }
        IEntry this[string key] { get; }

        event ScopedIdentifierAdded IdentifierAdded;
        event ScopeDisposed Disposed;

        bool Add(IEntry entry);
    }

    internal class Scope : IScope
    {
        private readonly Dict _identifiers = new Dict();
        private readonly IScope _parent;

        public IScope Parent => _parent;

        public event ScopedIdentifierAdded IdentifierAdded;
        public event ScopeDisposed Disposed;

        public IEntry this[string key] => _identifiers.GetOr(key, () => _parent?[key]);

        public Scope(IScope parent = null) {
            _parent = parent;
            if (_parent != null)
            {
                _parent.Disposed += OnParentDisposed;
                _parent.IdentifierAdded += OnParentIdentifierAdded;
            }
        }

        private void OnParentDisposed(IScope _) {
            Dispose();
        }

        private void OnParentIdentifierAdded(IScope scope, IEntry entry)
        {
            if (_identifiers.ContainsKey(entry.Label)) return;
            IdentifierAdded?.Invoke(scope, entry);
        }

        public void Dispose() {
            if (_parent != null) _parent.Disposed -= OnParentDisposed;
            Disposed?.Invoke(this);
        }

        public bool Add(IEntry entry) {
            var label = entry.Label;
            if (_identifiers.ContainsKey(label))
                return false;
            _identifiers.Add(label, entry);
            IdentifierAdded?.Invoke(this, entry);
            return true;
        }

        public IEnumerator<IEntry> GetEnumerator() {
            return new Enumerator(this);
        }

        IEnumerator IEnumerable.GetEnumerator() {
            return GetEnumerator();
        }

        public struct Enumerator : IEnumerator<IEntry>
        {
            readonly Scope _startScope;
            Scope _scope;
            Dict.ValueCollection.Enumerator _enumerator;

            internal Enumerator(Scope scope) : this() {
                _startScope = scope;
                Reset();
            }

            public bool MoveNext() {
                while (!_enumerator.MoveNext()) {
                    _scope = (Scope)_scope?._parent;
                    if (null == _scope) return false;
                    _enumerator = _scope._identifiers.Values.GetEnumerator();
                }
                return true;
            }

            public void Reset() {
                _scope = _startScope;
                _enumerator = _scope._identifiers.Values.GetEnumerator();
            }

            public IEntry Current => _enumerator.Current;

            object IEnumerator.Current => Current;

            public void Dispose() {}
        }
    }
}
