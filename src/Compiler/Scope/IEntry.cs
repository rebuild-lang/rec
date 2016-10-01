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
        public virtual IDeclaration Declaration { get; }
    }

    public interface IModuleEntry : IDeclaredEntry
    {
        IModuleDeclaration ModuleDeclaration { get; }
    }

    class ModuleEntry : DeclaredEntry, IModuleEntry
    {
        public override IDeclaration Declaration => ModuleDeclaration;

        public IModuleDeclaration ModuleDeclaration { get; set; }
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
        ITypedDeclaration TypedDeclaration { get; }
        IModuleDeclaration Type { get; } // TODO: Type is not always statically evaluated
    }

    class DefineEntry : DeclaredEntry, ITypedConstruct
    {
        public DefineDeclaration Define { get; set; }
        public override IDeclaration Declaration => Define;
        public ITypedDeclaration TypedDeclaration => Define;
        public IModuleDeclaration Type => Define.Type;
    }
}