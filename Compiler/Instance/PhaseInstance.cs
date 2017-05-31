using System.Collections.Generic;
using REC.AST;
using REC.Scope;
using REC.Tools;

namespace REC.Instance
{
    public interface IPhaseInstance : IInstance
    {
        IList<IPhaseDeclaration> Declarations { get; }
        ILocalIdentifierScope Identifiers { get; }
        IList<IFunctionDeclaration> Functions { get; }
    }

    class PhaseInstance : AbstractInstance, IPhaseInstance
    {
        public override string Name => Declarations.IsEmpty() ? "" : Declarations.First().Name;
        public IList<IPhaseDeclaration> Declarations { get; } = new List<IPhaseDeclaration>();
        public ILocalIdentifierScope Identifiers { get; } = new LocalIdentifierScope();
        public IList<IFunctionDeclaration> Functions { get; } = new List<IFunctionDeclaration>();
    }
}
