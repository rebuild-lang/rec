using REC.AST;
using REC.Function;
using REC.Scope;
using REC.Tools;

namespace REC.Instance
{
    using ArgumentInstanceCollection = NamedCollection<IArgumentInstance>;

    public interface IFunctionInstance : IInstance
    {
        IBindingLevel BindingLevel { get; set; }
        IPhaseInstance Phase { get; set; }

        ArgumentInstanceCollection LeftArguments { get; }
        ArgumentInstanceCollection RightArguments { get; }
        ArgumentInstanceCollection Results { get; }


        // TODO: how are external functions handled? - another instance type?
        // For now use the simple form
        IFunctionDeclaration Declaration { get; }

        ILocalIdentifierScope ArgumentIdentifiers { get; }
    }

    class FunctionInstance : AbstractInstance, IFunctionInstance
    {
        public override string Name { get; }
        public IBindingLevel BindingLevel { get; set; }
        public IPhaseInstance Phase { get; set; }

        public ArgumentInstanceCollection LeftArguments { get; } = new ArgumentInstanceCollection();
        public ArgumentInstanceCollection RightArguments { get; } = new ArgumentInstanceCollection();
        public ArgumentInstanceCollection Results { get; } = new ArgumentInstanceCollection();

        public IFunctionDeclaration Declaration { get; protected set; }

        public ILocalIdentifierScope ArgumentIdentifiers { get; set; } = new LocalIdentifierScope();

        internal FunctionInstance(string name) {
            Name = name;
        }

        internal FunctionInstance(IFunctionDeclaration decl) {
            Name = decl.Name;
            Declaration = decl;
        }
    }
}
