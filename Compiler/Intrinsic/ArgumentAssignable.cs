using System;
using REC.AST;

namespace REC.Intrinsic
{
    [AttributeUsage(AttributeTargets.Field)]
    class ArgumentAssignable : Attribute
    {
        ITypedDeclaration Decl { get; set; }
    }
}