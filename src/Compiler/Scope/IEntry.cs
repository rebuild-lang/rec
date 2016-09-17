using System.Collections.Generic;
using REC.AST;
using REC.Tools;

namespace REC.Scope
{
    using ArgumentDeclarationCollection = NamedCollection<IArgumentDeclaration>;

    public interface IEntry : INamed
    {
        // TODO: add access to shadowed names to process better error messages
    }

    abstract class Entry : IEntry
    {
        public string Name { get; set; }
    }

    public interface IDeclaredEntry : IEntry
    {
        IDeclaration Declaration { get; }
    }

    abstract class DeclaredEntry : IDeclaredEntry
    {
        public string Name => Declaration.Name;
        public IDeclaration Declaration { get; }
    }

    public interface IFuntionEntry : IEntry
    {
        IList<IFunctionDeclaration> FunctionDeclarations { get; }
    }

    class FunctionEnty : IFuntionEntry
    {
        public string Name => FunctionDeclarations?.First()?.Name;
        public IList<IFunctionDeclaration> FunctionDeclarations { get; } = new List<IFunctionDeclaration>();
    }

    public interface ITypedConstruct : IDeclaredEntry
    {
        IModuleDeclaration Type { get; } // TODO: Type is not always statically evaluated
    }

    class TypeConstruct : DeclaredEntry, ITypedConstruct
    {
        public IModuleDeclaration Type { get; set; }
    }
}