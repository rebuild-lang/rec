using System;
using System.Collections;
using System.Collections.Generic;
using REC.Scanner;
using REC.Tools;

namespace REC
{
    using Dict = Dictionary<string, IIdentifier>;
    using DictKeyValue = KeyValuePair<string, IIdentifier>;

    public delegate void ScopedIdentifierAdded(IIdentifierScope scope, IIdentifier identifier);

    public delegate void ScopeDisposed(IIdentifierScope scope);

    public interface IIdentifierScope : IDisposable
    {
        IIdentifierScope Parent { get; }
        IIdentifier this[string key] { get; }

        event ScopedIdentifierAdded IdentifierAdded;
        event ScopeDisposed Disposed;

        bool Add(IIdentifier identifier);
    }

    internal class IdentifierScope : IIdentifierScope, IEnumerable<IIdentifier>
    {
        private readonly Dict _identifiers = new Dict();
        private readonly IIdentifierScope _parent;

        public IIdentifierScope Parent => _parent;

        public event ScopedIdentifierAdded IdentifierAdded;
        public event ScopeDisposed Disposed;

        public IIdentifier this[string key] => _identifiers.GetOr(key, () => _parent?[key]);

        public IdentifierScope(IIdentifierScope parent = null) {
            _parent = parent;
            if (_parent != null)
            {
                _parent.Disposed += OnParentDisposed;
                _parent.IdentifierAdded += OnParentIdentifierAdded;
            }
        }

        private void OnParentDisposed(IIdentifierScope _) {
            Dispose();
        }

        private void OnParentIdentifierAdded(IIdentifierScope scope, IIdentifier identifier)
        {
            if (_identifiers.ContainsKey(identifier.Label)) return;
            IdentifierAdded?.Invoke(scope, identifier);
        }

        public void Dispose() {
            if (_parent != null) _parent.Disposed -= OnParentDisposed;
            Disposed?.Invoke(this);
        }

        public bool Add(IIdentifier identifier) {
            var label = identifier.Label;
            if (_identifiers.ContainsKey(label))
                return false;
            _identifiers.Add(label, identifier);
            IdentifierAdded?.Invoke(this, identifier);
            return true;
        }

        public IEnumerator<IIdentifier> GetEnumerator() {
            return new Enumerator(this);
        }

        IEnumerator IEnumerable.GetEnumerator() {
            return GetEnumerator();
        }

        [Serializable]
        public struct Enumerator : IEnumerator<IIdentifier>
        {
            readonly IdentifierScope _startScope;
            IdentifierScope _scope;
            Dict.ValueCollection.Enumerator _enumerator;

            internal Enumerator(IdentifierScope scope) : this() {
                _startScope = scope;
                Reset();
            }

            public bool MoveNext() {
                while (!_enumerator.MoveNext()) {
                    _scope = (IdentifierScope)_scope?._parent;
                    if (null == _scope) return false;
                    _enumerator = _scope._identifiers.Values.GetEnumerator();
                }
                return true;
            }

            public void Reset() {
                _scope = _startScope;
                _enumerator = _scope._identifiers.Values.GetEnumerator();
            }

            public IIdentifier Current => _enumerator.Current;

            object IEnumerator.Current => Current;

            public void Dispose() {}
        }
    }
}
