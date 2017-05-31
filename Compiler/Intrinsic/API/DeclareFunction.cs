using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using REC.AST;
using REC.Execution;
using REC.Instance;
using REC.Parser;

namespace REC.Intrinsic.API
{

    class DeclareFunction
    {

        public static IFunctionIntrinsic Get()
        {
            return new FunctionIntrinsic
            {
                Name = ".DeclareFunction",
                RightArgumentsType = typeof(DeclareFunction.RightArguments),
                ResultType = typeof(DeclareFunction.ResultArguments),
                CompileTimeApi = (left, right, result, context) => EvalCompileTime((DeclareFunction.RightArguments)right, (DeclareFunction.ResultArguments)result, context),
                IsCompileTimeOnly = true
            };
        }

        static void EvalCompileTime(DeclareFunction.RightArguments right, DeclareFunction.ResultArguments result, IContext context)
        {
            // TODO
        }

        class RightArguments : IRightArguments
        {
            public IIdentifierLiteral Name;
            public IList<IArgumentDeclaration> Left;
            public IList<IArgumentDeclaration> Right;
            public IList<IArgumentDeclaration> Result;
            public IBlockLiteral Block;
            public bool IsCompileTimeOnly = false;
        }

        [SuppressMessage(category: "ReSharper", checkId: "NotAccessedField.Local")]
        class ResultArguments : IResultArguments
        {
            [ArgumentAssignable] public IFunctionInstance Instance;
            [ArgumentAssignable] public IFunctionDeclaration Declaration;
        }
    }

}
