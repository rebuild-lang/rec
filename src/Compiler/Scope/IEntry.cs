using System.Collections.Generic;
using REC.AST;

namespace REC.Scope
{
    public interface IEntry
    {
        string Label { get; }
    }

    public class Entry : IEntry
    {
        public string Label { get; set; }
    }

    public interface IDeclaredEntry : IEntry
    {
        IDeclaration Declaration { get; }
    }

    public class DeclaredEntry : IDeclaredEntry
    {
        public string Label => Declaration.Name;
        public IDeclaration Declaration { get; }
    }

    public enum Association
    {
        Both, // does not matter (might give optimizations)
        Left, // a+b+c evaluates (a+b)+c
        Right, // a+b+c evaluates a+(b+c)
    }

    public interface ICallableEntry : IEntry
    {
        ICollection<IArgumentDeclaration> LeftArguments { get; }
        ICollection<IArgumentDeclaration> RightArguments { get; }
        ICollection<IArgumentDeclaration> Results { get; }

        ISet<IFunction> Preferred { get; }
        Association Associative { get; }
    }

    public interface IFunction : IDeclaredEntry, ICallableEntry
    {
        new IFunctionDeclaration Declaration { get; }
    }

    public class FunctionEnty : IFunction
    {
        public string Label => Declaration.Name;
        IDeclaration IDeclaredEntry.Declaration => Declaration;

        public IFunctionDeclaration Declaration { get; set; }

        public ICollection<IArgumentDeclaration> LeftArguments => Declaration.LeftArguments;
        public ICollection<IArgumentDeclaration> RightArguments => Declaration.RightArguments;
        public ICollection<IArgumentDeclaration> Results => Declaration.Results;
        public ISet<IFunction> Preferred { get; } = new HashSet<IFunction>();
        public Association Associative { get; } = Association.Both;
    }

    public interface ITypedConstruct : IEntry
    {
        IModule Type { get; } // TODO: Type is not always statically evaluated
    }

    public class TypeConstruct : Entry, ITypedConstruct
    {
        public IModule Type { get; set; }
    }

    public interface IArgument : ITypedConstruct
    {}

    public class Argument : TypeConstruct, IArgument
    {}

    public interface IVariable : ITypedConstruct
    {}

    public interface IDefine : ITypedConstruct
    {}
}