using System;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using REC.AST;
using REC.Intrinsic;
using REC.Parser;
using REC.Tools;

namespace REC.Execution
{
    public static class CompileTime
    {
        public static IExpression Execute(IExpression expression, IScope scope) {
            return Dynamic((dynamic) expression, scope);
        }

        static IExpression Dynamic(INamedExpressionTuple expressionTuple, IScope scope) {
            IExpression result = null;
            foreach (var sub in expressionTuple.Tuple) {
                result = Dynamic((dynamic)sub.Expression, scope);
            }
            return result;
        }

        static IExpression Dynamic(IExpressionBlock expressionBlock, IScope scope) {
            IExpression result = null;
            foreach (var sub in expressionBlock.Expressions) {
                result = Dynamic((dynamic) sub, scope);
            }
            return result;
        }

        [SuppressMessage("ReSharper", "UnusedParameter.Local")]
        static IExpression Dynamic(INumberLiteral numberLiteral, IScope scope) {
            //var type = scope.Identifiers["NumberLiteral"];
            //IExpression result = null;
            return numberLiteral;
        }

        static IExpression Dynamic(IFunctionInvocation functionInvocation, IScope scope) {
            var dynamicScope = new Parser.Scope {Parent = functionInvocation.Function.StaticScope};
            BuildArgumentValues(dynamicScope, functionInvocation.Left, functionInvocation.Function.LeftArguments, scope);
            BuildArgumentValues(dynamicScope, functionInvocation.Right, functionInvocation.Function.RightArguments, scope);
            // TODO: BuildResultValues(innerScope, functionInvocation.Function.Results);
            Dynamic(functionInvocation.Function.Implementation, dynamicScope);
            return null; // TODO: ExtractResultValues(innerScope, functionInvocation.Function.Results);
        }

        static void BuildArgumentValues(IScope innerScope, INamedExpressionTuple expressions, NamedCollection<IArgumentDeclaration> arguments, IScope argumentScope) {
            var argN = 0;
            foreach (var expression in expressions.Tuple) {
                var argument = arguments[argN];
                argN++;
                var value = Dynamic((dynamic)expression.Expression, argumentScope);
                var casted = ImplicitCast(value, argument.Type);
                innerScope.Values.Add(argument, casted);
            }
            for (;argN < arguments.Count; argN++) {
                var argument = arguments[argN];
                var value = Dynamic((dynamic) argument.Value, innerScope.Parent);
                innerScope.Values.Add(argument, value);
            }
        }

        static ITypedValue ImplicitCast(ITypedValue value, IModuleDeclaration type) {
            if (null == value) return null;
            if (value.Type == type) return value;
            // TODO: call the cast
            return null;
        }
        static ITypedValue ImplicitCast(ILiteral literal, IModuleDeclaration type) {
            // TODO: make fromLiteral implementable in Rebuild
            var fromLiteral = type.GetFromLiteral();
            var value = CreateValue(type);
            fromLiteral(value.Data, literal);
            return value;
        }

        static IExpression Dynamic(IIntrinsicExpression intrinsicExpression, IScope scope) {
            var func = intrinsicExpression.Intrinsic as IFunctionIntrinsic;
            if (func != null) {
                var leftArguments = (ILeftArguments)(func.LeftArgumentsType != null ? Activator.CreateInstance(func.LeftArgumentsType) : null);
                var rightArguments = (IRightArguments)(func.RightArgumentsType != null ? Activator.CreateInstance(func.RightArgumentsType) : null);
                var results = (IResultArguments)(func.ResultType != null ? Activator.CreateInstance(func.ResultType) : null);

                ScopeToNetTypes(scope.Values, leftArguments, func.LeftArgumentsType);
                ScopeToNetTypes(scope.Values, rightArguments, func.RightArgumentsType);
                ScopeToNetTypes(scope.Values, results, func.ResultType);

                func.CompileTime(leftArguments, rightArguments, results);
                // TODO: convert results
                // NetTypesToScope(results, func.ResultType, scope);
            }
            return null;
        }

        static void ScopeToNetTypes(IValueScope scope, object netValues, Type type) {
            if (netValues==null) return;
            foreach (var fieldInfo in type.GetRuntimeFields()) {
                var fieldValue = scope[fieldInfo.Name];
                var converter = fieldValue.Type.GetToNetType();
                var netValue = converter(fieldValue.Data);
                fieldInfo.SetValue(netValues, netValue);
            }
        }


        static ITypedValue CreateValue(IModuleDeclaration type) {
            var result = new TypedValue {
                Data = new byte[type.GetTypeSize()],
                Type = type
            };
            //var constructor = type.GetConstructor();
            // TODO: invoke constructor
            return result;
        }
    }
}