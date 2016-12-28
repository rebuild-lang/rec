using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
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
            foreach (var sub in expressionTuple.Tuple) result = Dynamic((dynamic) sub.Expression, scope);
            return result;
        }

        static IExpression Dynamic(IExpressionBlock expressionBlock, IScope scope) {
            IExpression result = null;
            foreach (var sub in expressionBlock.Expressions) result = Dynamic((dynamic) sub, scope);
            return result;
        }

        [SuppressMessage(category: "ReSharper", checkId: "UnusedParameter.Local")]
        static IExpression Dynamic(ILiteral literal, IScope scope) {
            return literal;
        }

        [SuppressMessage(category: "ReSharper", checkId: "UnusedParameter.Local")]
        static IExpression Dynamic(ITypedValue value, IScope scope) {
            return value;
        }

        static IExpression Dynamic(ITypedReference reference, IScope scope) {
            return scope.Values[reference.Declaration];
        }

        static IExpression Dynamic(IFunctionInvocation functionInvocation, IScope scope) {
            var dynamicScope = new Parser.Scope {Parent = functionInvocation.Function.StaticScope};
            BuildArgumentValues(dynamicScope, functionInvocation.Left, functionInvocation.Function.LeftArguments, scope);
            BuildArgumentValues(dynamicScope, functionInvocation.Right, functionInvocation.Function.RightArguments, scope);
            BuildResultValues(dynamicScope, functionInvocation.Function.Results);
            var result = Dynamic(functionInvocation.Function.Implementation, dynamicScope);
            ExtractArgumentReferenceValues(dynamicScope, functionInvocation.Left, functionInvocation.Function.LeftArguments, scope);
            ExtractArgumentReferenceValues(dynamicScope, functionInvocation.Right, functionInvocation.Function.RightArguments, scope);
            return ExtractResultValues(dynamicScope, result, functionInvocation.Function.Results);
        }

        static void BuildArgumentValues(
            IScope innerScope,
            INamedExpressionTuple expressions,
            IReadOnlyList<IArgumentDeclaration> arguments,
            IScope argumentScope) {
            var argN = 0;
            foreach (var expression in expressions.Tuple) {
                var argument = arguments[argN];
                argN++;
                var value = Dynamic((dynamic) expression.Expression, argumentScope);
                var casted = ImplicitCast(value, argument.Type);
                innerScope.Values.Add(argument, casted);
            }
            for (; argN < arguments.Count; argN++) {
                var argument = arguments[argN];
                var value = Dynamic((dynamic) argument.Value, innerScope.Parent);
                innerScope.Values.Add(argument, value);
            }
        }

        static void BuildResultValues(IScope innerScope, IEnumerable<IArgumentDeclaration> results) {
            foreach (var result in results) {
                var value = result.Value != null ? Dynamic((dynamic) result.Value, innerScope.Parent) : CreateValue(result.Type);
                innerScope.Values.Add(result, value);
            }
        }

        static void ExtractArgumentReferenceValues(
            IScope innerScope,
            INamedExpressionTuple expressions,
            IReadOnlyList<IArgumentDeclaration> arguments,
            IScope argumentScope)
        {
            var argN = 0;
            foreach (var expression in expressions.Tuple)
            {
                var argument = arguments[argN];
                argN++;
                if (argument.IsAssignable && expression.Expression is ITypedReference reference) {
                    var referenceDecl = argumentScope.Values[reference.Declaration];
                    var value = innerScope.Values[argument.Name];
                    var casted = ImplicitCast(value, referenceDecl.Type);
                    Array.Copy(casted.Data, referenceDecl.Data, referenceDecl.Data.Length);
                }
            }
        }

        static IExpression ExtractResultValues(IScope scope, IExpression result, NamedCollection<IArgumentDeclaration> results) {
            if (results.IsEmpty()) return null;
            if (results.Count == 1 && results.First().Name == null) return result;
            var expressionTuple = new NamedExpressionTuple();
            foreach (var namedResult in results) {
                var value = scope.Values[namedResult.Name];
                expressionTuple.Tuple.Add(
                    new NamedExpression {
                        Name = namedResult.Name,
                        Expression = value
                    });
            }
            return expressionTuple;
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

        static ITypedValue ImplicitCast(INamedExpressionTuple tuple, IModuleDeclaration type) {
            if (tuple.Tuple.Count == 1) return ImplicitCast((dynamic) tuple.Tuple.First().Expression, type);
            throw new ArgumentException(message: "Invalid argument tuple");
        }

        static IExpression Dynamic(IIntrinsicExpression intrinsicExpression, IScope scope) {
            if (intrinsicExpression.Intrinsic is IFunctionIntrinsic func) {
                var leftArguments = (ILeftArguments) (func.LeftArgumentsType != null ? Activator.CreateInstance(func.LeftArgumentsType) : null);
                var rightArguments = (IRightArguments) (func.RightArgumentsType != null ? Activator.CreateInstance(func.RightArgumentsType) : null);
                var results = (IResultArguments) (func.ResultType != null ? Activator.CreateInstance(func.ResultType) : null);

                ScopeToNetTypes(scope.Values, leftArguments, func.LeftArgumentsType);
                ScopeToNetTypes(scope.Values, rightArguments, func.RightArgumentsType);
                ScopeToNetTypes(scope.Values, results, func.ResultType);

                func.CompileTime(leftArguments, rightArguments, results);

                NetTypesToScope(leftArguments, func.LeftArgumentsType, scope.Values);
                NetTypesToScope(rightArguments, func.RightArgumentsType, scope.Values);
                NetTypesToScope(results, func.ResultType, scope.Values);
            }
            return null;
        }

        static void NetTypesToScope(object netValues, Type type, IValueScope scope) {
            if (netValues == null) return;
            foreach (var fieldInfo in type.GetRuntimeFields()) {
                if (fieldInfo.GetCustomAttributes(typeof(ArgumentAssignable)).Any()) {
                    var netValue = fieldInfo.GetValue(netValues);
                    var fieldValue = scope[fieldInfo.Name];
                    var converter = fieldValue.Type.GetFromNetType();
                    converter(netValue, fieldValue.Data);
                }
            }
        }

        static void ScopeToNetTypes(IValueScope scope, object netValues, Type type) {
            if (netValues == null) return;
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
