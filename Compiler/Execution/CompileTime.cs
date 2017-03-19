using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using REC.AST;
using REC.Instance;
using REC.Intrinsic;
using REC.Parser;
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
                return Invoke(invocation, context);

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

        static IExpression Invoke(IFunctionInvocation invocation, IContext callerContext) {
            var function = invocation.Function;
            var localValues = new LocalValueScope();
            var localContext = new Context(function.ArgumentIdentifiers, localValues);
            BuildArgumentValues(invocation.Left, function.LeftArguments, callerContext, localContext);
            BuildArgumentValues(invocation.Right, function.RightArguments, callerContext, localContext);
            BuildResultValues(function.Results, localContext);
            var result = ExecuteBlock(function.Declaration.Implementation, localContext);
            ExtractArgumentReferenceValues(localValues, invocation.Left, function.LeftArguments, callerContext);
            ExtractArgumentReferenceValues(localValues, invocation.Right, function.RightArguments, callerContext);
            return ExtractResultValues(localValues, result, function.Results);
        }

        static void BuildArgumentValues(
            INamedExpressionTuple expressions,
            ArgumentInstanceCollection arguments,
            IContext argumentContext,
            IContext functionContext) {
            var values = functionContext.Values;
            var argumentScope = functionContext.LocalIdentifiers;
            var argN = 0;
            foreach (var expression in expressions.Tuple) {
                var argumentName = expression.Name;
                if (string.IsNullOrEmpty(argumentName)) {
                    argumentName = arguments[argN].Name;
                    argN++;
                }
                var argument = (IArgumentInstance) argumentScope[argumentName];
                var value = ExecuteArgumentValue(argument, expression.Expression, argumentContext);
                var casted = ImplicitCast(value, argument.Type);
                values.Add(argument, casted);
            }
            for (; argN < arguments.Count; argN++) {
                var argument = arguments[argN];
                var value = Execute(argument.Argument.Value, functionContext) as ITypedValue;
                values.Add(argument, value);
            }
        }

        static IExpression ExecuteArgumentValue(IArgumentInstance argument, IExpression expression, IContext context) {
            if (argument.Type is IIntrinsicTypeModuleInstance intrinsicType
                && intrinsicType.NetType.IsInstanceOfType(expression)) {
                return expression; // allow passing unevaluated literals
            }
            return Execute(expression, context);
        }


        static void BuildResultValues(ArgumentInstanceCollection results, IContext functionContext) {
            var values = functionContext.Values;
            foreach (var result in results) {
                var value = result.Argument.Value != null ? Execute(result.Argument.Value, functionContext) as ITypedValue : CreateValue(result.Type);
                values.Add(result, value);
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
                    
            case INamedExpressionTuple tuple:
                if (tuple.Tuple.Count == 1) return ImplicitCast(tuple.Tuple.First().Expression, type);
                throw new ArgumentException(message: "Invalid argument tuple");

            default:
                var fromExpression = type.GetFromExpression();
                var expressionValue = CreateValue(type);
                fromExpression(expressionValue.Data, expression);
                return expressionValue;
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
