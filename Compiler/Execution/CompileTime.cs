using System;
using System.Collections.Generic;
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
            switch (expression) {
            case ILiteral literal:
                return literal;
            case ITypedValue value:
                return value;
            case ITypedReference reference:
                return context.Values[reference.Instance];

            case INamedExpressionTuple tuple:
                return ExecuteTuple(tuple, context);
            case IExpressionBlock block:
                return ExecuteBlock(block, context);

            case IVariableDeclaration variable:
                InitVariable(variable, context);
                return null;

            case IFunctionInvocation invocation:
                return Invoce(invocation, context);

            case IIntrinsicExpression intrinsic:
                return ExecuteIntrinsic(intrinsic, context);

            default:
                throw new ArgumentException(message: "unknown expression type");
            }
        }

        static IExpression ExecuteTuple(INamedExpressionTuple expressionTuple, IContext context) {
            IExpression result = null;
            foreach (var sub in expressionTuple.Tuple) {
                result = Execute(sub.Expression, context);
            }
            return result;
        }

        static IExpression ExecuteBlock(IExpressionBlock expressionBlock, IContext context) {
            IExpression result = null;
            foreach (var sub in expressionBlock.Expressions) {
                result = Execute(sub, context);
            }
            return result;
        }

        static void InitVariable(IVariableDeclaration variableDecl, IContext context) {
            var variable = context.Identifiers[variableDecl.Name] as IVariableInstance;
            var value = variableDecl.Value != null ? Execute(variableDecl.Value, context) as ITypedValue : CreateValue(variableDecl.Type);
            context.Values.Add(variable, value);
        }

        static IExpression Invoce(IFunctionInvocation invocation, IContext callerContext) {
            var localValues = new LocalValueScope();
            var argumentIdentifiers = invocation.Function.ArgumentIdentifiers;
            var localContext = new Context(argumentIdentifiers, localValues);
            BuildArgumentValues(localValues, invocation.Left, argumentIdentifiers, invocation.Function.LeftArguments, callerContext, localContext);
            BuildArgumentValues(localValues, invocation.Right, argumentIdentifiers, invocation.Function.RightArguments, callerContext, localContext);
            BuildResultValues(localValues, invocation.Function.Results, localContext);
            var result = ExecuteBlock(invocation.Function.Declaration.Implementation, localContext);
            ExtractArgumentReferenceValues(localValues, invocation.Left, invocation.Function.LeftArguments, callerContext);
            ExtractArgumentReferenceValues(localValues, invocation.Right, invocation.Function.RightArguments, callerContext);
            return ExtractResultValues(localValues, result, invocation.Function.Results);
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
                var argument = (IArgumentInstance) argumentScope[argumentName];
                var value = Execute(expression.Expression, argumentContext);
                var casted = ImplicitCast(value, argument.Type);
                localValues.Add(argument, casted);
            }
            for (; argN < arguments.Count; argN++) {
                var argument = arguments[argN];
                var value = Execute(argument.Argument.Value, functionContext) as ITypedValue;
                localValues.Add(argument, value);
            }
        }

        static void BuildResultValues(LocalValueScope localValues, ArgumentInstanceCollection results, IContext functionContext) {
            foreach (var result in results) {
                var value = result.Argument.Value != null ? Execute(result.Argument.Value, functionContext) as ITypedValue : CreateValue(result.Type);
                localValues.Add(result, value);
            }
        }

        static void ExtractArgumentReferenceValues(
            ILocalValueScope localValues,
            INamedExpressionTuple expressions,
            ArgumentInstanceCollection arguments,
            IContext callerContext) {
            var argN = 0;
            foreach (var expression in expressions.Tuple) {
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

        static ITypedValue ImplicitCast(IExpression expression, IModuleInstance type) {
            if (null == expression) return null;
            switch (expression) {
            case ITypedValue value:
                if (value.Type == type) return value;
                // TODO: call the cast
                return null;

            case ILiteral literal:
                // TODO: make fromLiteral implementable in Rebuild
                var fromLiteral = type.GetFromLiteral();
                var literalValue = CreateValue(type);
                fromLiteral(literalValue.Data, literal);
                return literalValue;

            case INamedExpressionTuple tuple:
                if (tuple.Tuple.Count == 1) return ImplicitCast(tuple.Tuple.First().Expression, type);
                throw new ArgumentException(message: "Invalid argument tuple");

            default:
                throw new ArgumentException(message: "Unknown Cast Source");
            }
        }

        static IExpression ExecuteIntrinsic(IIntrinsicExpression intrinsicExpression, IContext context) {
            var function = intrinsicExpression.Intrinsic;

            var leftArguments = (ILeftArguments) function.LeftArgumentsType?.CreateInstance();
            var rightArguments = (IRightArguments) function.RightArgumentsType?.CreateInstance();
            var results = (IResultArguments) function.ResultType?.CreateInstance();

            RebuildToNetTypes(context.Values, leftArguments, intrinsicExpression.LeftFields);
            RebuildToNetTypes(context.Values, rightArguments, intrinsicExpression.RightFields);
            RebuildToNetTypes(context.Values, results, intrinsicExpression.ResultFields);

            if (null != function.CompileTimeApi) {
                function.CompileTimeApi(leftArguments, rightArguments, results, context);
            }
            else {
                function.CompileTime(leftArguments, rightArguments, results);
            }

            NetTypesToRebuild(leftArguments, intrinsicExpression.LeftFields, context.Values);
            NetTypesToRebuild(rightArguments, intrinsicExpression.RightFields, context.Values);
            NetTypesToRebuild(results, intrinsicExpression.ResultFields, context.Values);

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
