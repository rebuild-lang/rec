using REC.Instance;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace REC.Scope
{
    using ReadIdentifierDict = ReadOnlyDictionary<string, IInstance>;

    public interface IParentedIdentifierScope : ILocalIdentifierScope
    {
        IParentedIdentifierScope Parent { get; set; }
    }

    class ParentedIdentifierScope : IParentedIdentifierScope
    {
        readonly ILocalIdentifierScope _locals = new LocalIdentifierScope();

        public IParentedIdentifierScope Parent { get; set; }

        public ReadIdentifierDict LocalIdentifiers => _locals.LocalIdentifiers;

        public IInstance this[string key] => _locals[key] ?? Parent?[key];

        public bool Add(IInstance instance) {
            return _locals.Add(instance);
        }


        public bool Replace(IInstance oldInstance, IInstance newInstance) {
            return _locals.Replace(oldInstance, newInstance);
        }

        internal ParentedIdentifierScope() {}

        internal ParentedIdentifierScope(ILocalIdentifierScope locals) {
            _locals = locals;
        }

        public IEnumerator<IInstance> GetEnumerator()
        {
            return new Enumerator(this);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        struct Enumerator : IEnumerator<IInstance>
        {
            readonly IParentedIdentifierScope _startIdentifierScope;
            ParentedIdentifierScope _currentIdentifierScope;
            IEnumerator<IInstance> _enumerator;

            internal Enumerator(IParentedIdentifierScope scope) : this()
            {
                _startIdentifierScope = scope;
                Reset();
            }

            public bool MoveNext()
            {
                while (!_enumerator.MoveNext())
                {
                    _currentIdentifierScope = (ParentedIdentifierScope)_currentIdentifierScope?.Parent;
                    if (null == _currentIdentifierScope)
                        return false;
                    _enumerator = _currentIdentifierScope._locals.GetEnumerator();
                }
                return true;
            }

            public void Reset()
            {
                _currentIdentifierScope = (ParentedIdentifierScope)_startIdentifierScope;
                _enumerator = _currentIdentifierScope._locals.GetEnumerator();
            }

            public IInstance Current => _enumerator.Current;

            object IEnumerator.Current => Current;

            public void Dispose() { }
        }
    }
}