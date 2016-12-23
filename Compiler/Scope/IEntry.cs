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
        public abstract IDeclaration Declaration { get; }
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

    public interface IFunctionEntry : IEntry
    {
        IList<IFunctionDeclaration> FunctionDeclarations { get; }
    }

    class FunctionEntry : IFunctionEntry
    {
        public string Name => FunctionDeclarations?.First()?.Name;
        public IList<IFunctionDeclaration> FunctionDeclarations { get; } = new List<IFunctionDeclaration>();
    }

    public interface ITypedConstruct : IDeclaredEntry
    {
        ITypedDeclaration TypedDeclaration { get; }
        IModuleDeclaration Type { get; } // TODO: Type is not always statically evaluated
    }

    public interface IVariableEntry : ITypedConstruct
    {
        IVariableDeclaration Variable { get; }
    }

    class VariableEntry : DeclaredEntry, IVariableEntry
    {
        public IVariableDeclaration Variable { get; set; }
        public override IDeclaration Declaration => Variable;
        public ITypedDeclaration TypedDeclaration => Variable;
        public IModuleDeclaration Type => Variable.Type;
    }

    public interface IArgumentEntry : ITypedConstruct
    {
        IArgumentDeclaration Argument { get; }
    }

    class ArgumentEntry : DeclaredEntry, IArgumentEntry
    {
        public IArgumentDeclaration Argument { get; set; }
        public override IDeclaration Declaration => Argument;
        public ITypedDeclaration TypedDeclaration => Argument;
        public IModuleDeclaration Type => Argument.Type;
    }
}
