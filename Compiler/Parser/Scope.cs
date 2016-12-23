using REC.Execution;
using REC.Scope;

namespace REC.Parser
{
    public interface IScope
    {
        IScope Parent { get; }
        IIdentifierScope Identifiers { get; }
        IValueScope Values { get; }
    }
    class Scope : IScope
    {
        IScope _parent;
        public IScope Parent {
            get { return _parent; }
            set {
                _parent = value;
                Identifiers = new IdentifierScope { Parent = value?.Identifiers };
                Values = new ValueScope { Parent = value?.Values };
            }
        }

        public IIdentifierScope Identifiers { get; private set; } = new IdentifierScope();
        public IValueScope Values { get; private set; } = new ValueScope();
    }
}