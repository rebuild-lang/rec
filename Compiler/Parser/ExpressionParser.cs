using System.Collections.Generic;
using System.Linq;
using REC.AST;
using REC.Execution;
using REC.Scanner;
using REC.Scope;
using REC.Tools;

namespace REC.Parser
{
    static class ExpressionParser
    {
        internal static INamedExpressionTuple Parse(IEnumerator<TokenData> tokens, IScope scope, ref bool done) {
            var result = new NamedExpressionTuple();
            while (!done) {
                var token = tokens.Current;
                switch (token.Type) {
                case Token.OperatorLiteral:
                    var operatorLiteral = (IIdentifierLiteral) token.Data;
                    // TODO: split operators with scope
                    if (operatorLiteral.Content == "&") {
                        if (!tokens.MoveNext()) done = true;
                        if (done) return result; // TODO: report error: missing tokens

                        var inner = Parse(tokens, scope, ref done);
                        var execResult = CompileTime.Execute(inner, scope);
                        if (execResult != null) {
                            if (execResult is INamedExpressionTuple) {
                                result.Tuple.AddRange((execResult as INamedExpressionTuple).Tuple);
                            }
                            else
                                result.Tuple.Add(new NamedExpression { Expression = execResult });
                        }
                        break;
                    }
                    goto case Token.IdentifierLiteral;
                case Token.IdentifierLiteral:
                    var identifierLiteral = (IIdentifierLiteral) token.Data;
                    var resolved = scope.Identifiers[identifierLiteral.Content];
                    if (null != resolved) {
                        var subexpression = ParseResolved((dynamic) resolved, result, tokens, scope, ref done);
                        result.Tuple.Add(new NamedExpression {Expression = subexpression ?? identifierLiteral});
                        continue;
                    }
                    result.Tuple.Add(new NamedExpression {Expression = identifierLiteral});
                    break;
                case Token.StringLiteral:
                    var stringLiteral = (IStringLiteral) token.Data;
                    result.Tuple.Add(new NamedExpression {Expression = stringLiteral});
                    break;
                case Token.NumberLiteral:
                    var numberLiteral = (INumberLiteral) token.Data;
                    result.Tuple.Add(new NamedExpression {Expression = numberLiteral});
                    break;
                case Token.BlockStartIndentation:
                    var tokenBlock = (IBlockLiteral) token.Data;
                    result.Tuple.Add(new NamedExpression {Expression = tokenBlock});
                    break;
                }
                if (done) break;
                if (!tokens.MoveNext()) done = true;
            }
            return result;
        }

        static IExpression ParseResolved(
            ITypedConstruct typed,
            INamedExpressionTuple leftArguments,
            IEnumerator<TokenData> tokens,
            IScope scope,
            ref bool done) {
            var result = new TypedReference {
                Range = tokens.Current.Range,
                Type = typed.Type,
                Declaration = typed.TypedDeclaration
            };

            if (!tokens.MoveNext()) done = true;
            return result;
        }

        static IExpression ParseResolved(
            IFunctionEntry function,
            INamedExpressionTuple leftArguments,
            IEnumerator<TokenData> tokens,
            IScope scope,
            ref bool done) {
            var range = tokens.Current.Range;
            INamedExpressionTuple rightArguments = new NamedExpressionTuple();
            var pool = function.FunctionDeclarations;

            if (!tokens.MoveNext()) {
                done = true;
                return CreateFunctionInvocation(pool, range, leftArguments, rightArguments);
            }

            var filteredPool = FilterFunctionLeftArguments(pool, leftArguments);
            filteredPool = ParseAndFilterFunctionRightArguments(filteredPool, ref rightArguments, tokens, scope, ref done);
            if (filteredPool.Count != 1) return null;
            // TODO: proper error handling
            return CreateFunctionInvocation(ListExtensionMethods.First<IFunctionDeclaration>(filteredPool), range, leftArguments, rightArguments);
        }

        static IList<IFunctionDeclaration> ParseAndFilterFunctionRightArguments(
            IList<IFunctionDeclaration> pool,
            ref INamedExpressionTuple rightArguments,
            IEnumerator<TokenData> tokens,
            IScope scope,
            ref bool done) {
            if (!pool.IsEmpty()) {
                rightArguments = Parse(tokens, scope, ref done);
                return FilterFunctionRightArguments(pool, rightArguments);
            }
            return pool;
        }

        static IList<IFunctionDeclaration> FilterFunctionLeftArguments(IList<IFunctionDeclaration> pool, INamedExpressionTuple leftArguments) {
            return pool.Where(
                f => {
                    if (f.LeftArguments == null) return true;
                    if (f.LeftArguments.Count > leftArguments.Tuple.Count) return false;
                    var o = leftArguments.Tuple.Count - f.LeftArguments.Count;
                    foreach (var fArg in f.LeftArguments) {
                        var givenArg = leftArguments.Tuple[o];
                        if (!CanImplicitConvertExpressionTo(givenArg.Expression, fArg.Type)) return false;
                        o++;
                    }
                    return f.LeftArguments.Count <= leftArguments.Tuple.Count;
                }).ToList();
        }

        static bool CanImplicitConvertExpressionTo(IExpression givenArgExpression, IModuleDeclaration fArgType) {
            return true;
        }

        static IList<IFunctionDeclaration> FilterFunctionRightArguments(IList<IFunctionDeclaration> pool, INamedExpressionTuple rightArguments) {
            return pool.Where(
                f => {
                    if (f.RightArguments == null) return true;
                    if (f.MandatoryRightArgumentCount() > rightArguments.Tuple.Count
                        || f.MaxRightArgumentCount() != null && f.MaxRightArgumentCount() < rightArguments.Tuple.Count)
                        return false;
                    return true;
                }).ToList();
        }

        static IExpression CreateFunctionInvocation(
            IList<IFunctionDeclaration> pool,
            TextFileRange range,
            INamedExpressionTuple leftArguments,
            INamedExpressionTuple rightArguments) {
            var filteredPool = FilterFunctionLeftArguments(pool, leftArguments);
            filteredPool = FilterFunctionRightArguments(filteredPool, rightArguments);
            if (filteredPool.Count != 1) return null;
            // TODO: proper error handling
            return CreateFunctionInvocation(filteredPool.First(), range, leftArguments, rightArguments);
        }

        static IExpression CreateFunctionInvocation(
            IFunctionDeclaration function,
            TextFileRange range,
            INamedExpressionTuple leftArguments,
            INamedExpressionTuple rightArguments) {
            var invocation = new FunctionInvocation {
                Function = function,
                Range = range,
                Left = AssignArguments(function.LeftArguments, leftArguments),
                Right = AssignArguments(function.RightArguments, rightArguments)
            };
            return invocation;
        }

        static INamedExpressionTuple AssignArguments(NamedCollection<IArgumentDeclaration> argumentDeclarations, INamedExpressionTuple arguments) {
            var result = new NamedExpressionTuple();
            var o = arguments.Tuple.Count - argumentDeclarations.Count;
            foreach (var argumentDeclaration in argumentDeclarations) {
                // TODO: check that conversion exists
                result.Tuple.Add(arguments.Tuple[o]);
                arguments.Tuple.RemoveAt(o);
            }
            // TODO: check that enough arguments are given
            return result;
        }
    }
}
