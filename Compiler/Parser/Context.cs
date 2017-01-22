using System.Collections.ObjectModel;
using REC.AST;
using REC.Execution;
using REC.Instance;
using REC.Scope;

namespace REC.Parser
{
    public interface IContext
    {
        IContext Parent { get; }
        ILocalIdentifierScope Identifiers { get; }
        ILocalValueScope Values { get; }
    }

    class Context : IContext
    {
        class ParentValueScope : ILocalValueScope
        {
            readonly ILocalValueScope _local = new LocalValueScope();
            internal ILocalValueScope Parent;

            public ReadOnlyDictionary<ITypedInstance, ITypedValue> Locals => _local.Locals;

            ITypedValue ILocalValueScope.this[ITypedInstance key] => _local[key] ?? Parent?[key];

            internal ParentValueScope() {}

            internal ParentValueScope(ILocalValueScope local) {
                _local = local;
            }

            public void Add(ITypedInstance instance, ITypedValue value)
            {
                _local.Add(instance, value);
            }
        }

        IContext _parent;
        readonly ParentedIdentifierScope _identifiers = new ParentedIdentifierScope();
        readonly ParentValueScope _values = new ParentValueScope();

        internal Context() {}

        internal Context(ILocalIdentifierScope idScope, ILocalValueScope valueScope) {
            _identifiers = new ParentedIdentifierScope(idScope);
            _values = new ParentValueScope(valueScope);
        }

        public IContext Parent {
            get { return _parent; }
            set {
                _parent = value;
                _identifiers.Parent = value?.Identifiers as IParentedIdentifierScope;
                _values.Parent = value?.Values;
            }
        }

        public ILocalIdentifierScope Identifiers => _identifiers;
        public ILocalValueScope Values => _values;
    }
}
