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
        internal readonly ILocalIdentifierScope Locals = new LocalIdentifierScope();

        public IParentedIdentifierScope Parent { get; set; }

        public ReadIdentifierDict LocalIdentifiers => Locals.LocalIdentifiers;

        public IInstance this[string key] => Locals[key] ?? Parent?[key];

        public bool Add(IInstance instance) {
            return Locals.Add(instance);
        }


        public bool Replace(IInstance oldInstance, IInstance newInstance) {
            return Locals.Replace(oldInstance, newInstance);
        }

        internal ParentedIdentifierScope() {}

        internal ParentedIdentifierScope(ILocalIdentifierScope locals) {
            Locals = locals;
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
                    _enumerator = _currentIdentifierScope.Locals.GetEnumerator();
                }
                return true;
            }

            public void Reset()
            {
                _currentIdentifierScope = (ParentedIdentifierScope)_startIdentifierScope;
                _enumerator = _currentIdentifierScope.Locals.GetEnumerator();
            }

            public IInstance Current => _enumerator.Current;

            object IEnumerator.Current => Current;

            public void Dispose() { }
        }
    }
}