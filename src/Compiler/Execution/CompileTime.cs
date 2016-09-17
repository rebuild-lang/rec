using System.Collections.Generic;
using REC.AST;
using REC.Intrinsic;
using REC.Scope;

namespace REC.Execution
{


    class ValueScope
    {
        ValueScope Parent { get; set; }

        Dictionary<IDeclaration, ITypedValue> Values { get; } = new Dictionary<IDeclaration, ITypedValue>();
    }

    public static class CompileTime
    {
        public static IExpression Execute(IExpression expression, IScope scope) {
            return Dynamic((dynamic) expression);
        }

        static IExpression Dynamic(INamedExpressionTuple expressionTuple) {
            IExpression result = null;
            foreach (var sub in expressionTuple.tuple) {
                result = Dynamic((dynamic)sub.Expression);
            }
            return result;
        }

        static IExpression Dynamic(IExpressionBlock expressionBlock) {
            IExpression result = null;
            foreach (var sub in expressionBlock.Expressions) {
                result = Dynamic((dynamic) sub);
            }
            return result;
        }

        static IExpression Dynamic(IFunctionInvocation functionInvocation) {
            Dynamic(functionInvocation.Function.Implementation);
            return null;
        }

        static IExpression Dynamic(IIntrinsicExpression intrinsicExpression) {
            var result = new ArgumentValues();
            var rightArguments = new ArgumentValues();
            intrinsicExpression.Intrinsic.InvokeCompileTime(result, rightArguments);
            return null;
        }
    }
}