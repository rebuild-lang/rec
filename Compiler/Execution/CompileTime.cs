using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Reflection;
using REC.AST;
using REC.Instance;
using REC.Intrinsic;
using REC.Parser;
using REC.Scope;
using REC.Tools;

namespace REC.Execution
{
    using FieldToDecl = Dictionary<FieldInfo, ITypedInstance>;
    using ArgumentInstanceCollection = NamedCollection<IArgumentInstance>;

    public static class CompileTime
    {
        public static IExpression Execute(IExpression expression, IContext context) {
            return Dynamic((dynamic) expression, context);
        }

        static IExpression Dynamic(INamedExpressionTuple expressionTuple, IContext context) {
            IExpression result = null;
            foreach (var sub in expressionTuple.Tuple) result = Dynamic((dynamic) sub.Expression, context);
            return result;
        }

        static IExpression Dynamic(IExpressionBlock expressionBlock, IContext context) {
            IExpression result = null;
            foreach (var sub in expressionBlock.Expressions) result = Dynamic((dynamic) sub, context);
            return result;
        }

        [SuppressMessage(category: "ReSharper", checkId: "UnusedParameter.Local")]
        static IExpression Dynamic(ILiteral literal, IContext context) {
            return literal;
        }

        [SuppressMessage(category: "ReSharper", checkId: "UnusedParameter.Local")]
        static IExpression Dynamic(ITypedValue value, IContext context) {
            return value;
        }

        static IExpression Dynamic(ITypedReference reference, IContext context) {
            return context.Values[reference.Instance];
        }

        static IExpression Dynamic(IVariableDeclaration variableDecl, IContext context) {
            var variable = context.Identifiers[variableDecl.Name] as IVariableInstance;
            var value = variableDecl.Value != null ? Dynamic((dynamic)variableDecl.Value, context) : CreateValue(variableDecl.Type);
            context.Values.Add(variable, value);
            return null;
        }

        static IExpression Dynamic(IFunctionInvocation functionInvocation, IContext callerContext) {
            var localValues = new LocalValueScope();
            var argumentIdentifiers = functionInvocation.Function.ArgumentIdentifiers;
            var localContext = new Context(argumentIdentifiers, localValues);
            BuildArgumentValues(localValues, functionInvocation.Left, argumentIdentifiers, functionInvocation.Function.LeftArguments, callerContext, localContext);
            BuildArgumentValues(localValues, functionInvocation.Right, argumentIdentifiers, functionInvocation.Function.RightArguments, callerContext, localContext);
            BuildResultValues(localValues, functionInvocation.Function.Results, localContext);
            var result = Dynamic(functionInvocation.Function.Declaration.Implementation, localContext);
            ExtractArgumentReferenceValues(localValues, functionInvocation.Left, functionInvocation.Function.LeftArguments, callerContext);
            ExtractArgumentReferenceValues(localValues, functionInvocation.Right, functionInvocation.Function.RightArguments, callerContext);
            return ExtractResultValues(localValues, result, functionInvocation.Function.Results);
        }

        static void BuildArgumentValues(
            ILocalValueScope localValues,
            INamedExpressionTuple expressions,
            ILocalIdentifierScope argumentScope,
            ArgumentInstanceCollection arguments,
            IContext argumentContext,
            IContext functionContext) {
            var argN = 0;
            foreach (var expression in expressions.Tuple) {
                var argumentName = expression.Name;
                if (string.IsNullOrEmpty(argumentName)) {
                    argumentName = arguments[argN].Name;
                    argN++;
                }
                var argument = (IArgumentInstance)argumentScope[argumentName];
                var value = Dynamic((dynamic)expression.Expression, argumentContext);
                var casted = ImplicitCast(value, argument.Type);
                localValues.Add(argument, casted);
            }
            for (; argN < arguments.Count; argN++) {
                var argument = arguments[argN];
                var value = Dynamic((dynamic) argument.Argument.Value, functionContext);
                localValues.Add(argument, value);
            }
        }

        static void BuildResultValues(LocalValueScope localValues, ArgumentInstanceCollection results, IContext functionContext) {
            foreach (var result in results) {
                var value = result.Argument.Value != null ? Dynamic((dynamic) result.Argument.Value, functionContext) : CreateValue(result.Type);
                localValues.Add(result, value);
            }
        }

        static void ExtractArgumentReferenceValues(
            ILocalValueScope localValues,
            INamedExpressionTuple expressions,
            ArgumentInstanceCollection arguments,
            IContext callerContext)
        {
            var argN = 0;
            foreach (var expression in expressions.Tuple)
            {
                var argument = arguments[argN];

                argN++;
                if (argument.Argument.IsAssignable && expression.Expression is ITypedReference reference) {
                    var referenceDecl = callerContext.Values[reference.Instance];
                    var value = localValues[argument];
                    var casted = ImplicitCast(value, referenceDecl.Type);
                    Array.Copy(casted.Data, referenceDecl.Data, referenceDecl.Data.Length);
                }
            }
        }

        static IExpression ExtractResultValues(ILocalValueScope localValues, IExpression result, ArgumentInstanceCollection results) {
            if (results.IsEmpty()) return null;
            if (results.Count == 1 && results.First().Name == null) return result;
            var expressionTuple = new NamedExpressionTuple();
            foreach (var namedResult in results) {
                var value = localValues[namedResult];
                expressionTuple.Tuple.Add(
                    new NamedExpression {
                        Name = namedResult.Name,
                        Expression = value
                    });
            }
            return expressionTuple;
        }

        static ITypedValue ImplicitCast(ITypedValue value, IModuleInstance type) {
            if (null == value) return null;
            if (value.Type == type) return value;
            // TODO: call the cast
            return null;
        }

        static ITypedValue ImplicitCast(ILiteral literal, IModuleInstance type) {
            // TODO: make fromLiteral implementable in Rebuild
            var fromLiteral = type.GetFromLiteral();
            var value = CreateValue(type);
            fromLiteral(value.Data, literal);
            return value;
        }

        static ITypedValue ImplicitCast(INamedExpressionTuple tuple, IModuleInstance type) {
            if (tuple.Tuple.Count == 1) return ImplicitCast((dynamic) tuple.Tuple.First().Expression, type);
            throw new ArgumentException(message: "Invalid argument tuple");
        }

        static IExpression Dynamic(IIntrinsicExpression intrinsicExpression, IContext context) {
            if (intrinsicExpression.Intrinsic is IFunctionIntrinsic func) {
                var leftArguments = (ILeftArguments) (func.LeftArgumentsType != null ? Activator.CreateInstance(func.LeftArgumentsType) : null);
                var rightArguments = (IRightArguments) (func.RightArgumentsType != null ? Activator.CreateInstance(func.RightArgumentsType) : null);
                var results = (IResultArguments) (func.ResultType != null ? Activator.CreateInstance(func.ResultType) : null);

                RebuildToNetTypes(context.Values, leftArguments, intrinsicExpression.LeftFields);
                RebuildToNetTypes(context.Values, rightArguments, intrinsicExpression.RightFields);
                RebuildToNetTypes(context.Values, results, intrinsicExpression.ResultFields);

                func.CompileTime(leftArguments, rightArguments, results);

                NetTypesToRebuild(leftArguments, intrinsicExpression.LeftFields, context.Values);
                NetTypesToRebuild(rightArguments, intrinsicExpression.RightFields, context.Values);
                NetTypesToRebuild(results, intrinsicExpression.ResultFields, context.Values);
            }
            return null;
        }

        static void NetTypesToRebuild(object netValues, FieldToDecl fields, ILocalValueScope localValues) {
            if (netValues == null) return;
            foreach (var field in fields) {
                if (field.Key.GetCustomAttributes(typeof(ArgumentAssignable)).Any()) {
                    var netValue = field.Key.GetValue(netValues);
                    var fieldValue = localValues[field.Value];
                    var converter = fieldValue.Type.GetFromNetType();
                    converter(netValue, fieldValue.Data);
                }
            }
        }

        static void RebuildToNetTypes(ILocalValueScope localValues, object netValues, FieldToDecl fields) {
            if (netValues == null) return;
            foreach (var field in fields) {
                var fieldValue = localValues[field.Value];
                var converter = fieldValue.Type.GetToNetType();
                var netValue = converter(fieldValue.Data);
                field.Key.SetValue(netValues, netValue);
            }
        }


        static ITypedValue CreateValue(IModuleInstance type) {
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
