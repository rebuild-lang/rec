using System.Collections.Generic;

namespace REC.Identifier
{
    public interface IIdentifier
    {
        string Label { get; }
        ICallable Callable { get; }
        bool IsCompileTime { get; }
    }

    public interface IModule : IIdentifier
    {
        bool IsType { get; }
        IIdentifierScope Scope { get; }
    }

    public enum Association
    {
        Both, // does not matter (might give optimizations)
        Left, // a+b+c evaluates (a+b)+c
        Right, // a+b+c evaluates a+(b+c)
    }

    public interface ICallable
    {
        IEnumerable<IArgument> LeftArguments { get; }
        IEnumerable<IArgument> RightArguments { get; }
        IEnumerable<IArgument> Results { get; }
        ISet<IFunction> Preferred { get; } // only valid if left and right arguments are expected
        Association Associative { get; } // only valid if no preferred precedence was found
    }

    public interface IFunction : IIdentifier, ICallable
    {
        
    }

    public interface ITypedConstruct : IIdentifier
    {
        IModule Type { get; } // TODO: Type is not always statically evaluated
    }

    public interface IArgument : ITypedConstruct
    {
        
    }

    public interface IVariable : ITypedConstruct
    {
    }

    public interface IDefine : ITypedConstruct
    {
    }
}