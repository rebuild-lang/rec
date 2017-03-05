using REC.AST;
using REC.Execution;
using REC.Instance;
using REC.Scanner;
using REC.Tools;
using System.Collections.Generic;
using System.Linq;

namespace REC.Parser
{
    static class ExpressionParser
    {
        internal static INamedExpressionTuple Parse(IEnumerator<TokenData> tokens, IContext context, ref bool done) {
            return ParseResult(new NamedExpressionTuple(), tokens, context, ref done);
        }

        static INamedExpressionTuple ParseResult(INamedExpressionTuple result, IEnumerator<TokenData> tokens, IContext context, ref bool done) {
            while (!done) {
                var token = tokens.Current;
                switch (token.Type) {
                case Token.OperatorLiteral:
                    var operatorLiteral = (IIdentifierLiteral) token.Data;
                    if (operatorLiteral.Content == "&") {
                        if (!tokens.MoveNext()) done = true;
                        if (done) return result; // TODO: report error: missing tokens

                        var inner = Parse(tokens, context, ref done);
                        var execResult = CompileTime.Execute(inner, context);
                        if (execResult != null) {
                            if (execResult is INamedExpressionTuple execNamedTuple)
                                result.Tuple.AddRange(execNamedTuple.Tuple);
                            else
                                result.Tuple.Add(new NamedExpression {Expression = execResult});
                        }
                        continue;
                    }
                    using (var newTokens = OperatorLiteralSplitter.Split(operatorLiteral, context, ref done).Before(tokens).GetEnumerator()) {
                        return ParseResult(result, newTokens, context, ref done);
                    }
                case Token.IdentifierLiteral:
                    var identifierLiteral = (IIdentifierLiteral) token.Data;
                    var resolved = context.Identifiers[identifierLiteral.Content];
                    if (null != resolved) {
                        var subexpression = ParseResolved((dynamic) resolved, result, tokens, context, ref done);
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
            ITypedInstance typed,
            INamedExpressionTuple leftArguments,
            IEnumerator<TokenData> tokens,
            IContext context,
            ref bool done) {
            var result = new TypedReference {
                Range = tokens.Current.Range,
                Type = typed.Type,
                Instance = typed
            };

            if (!tokens.MoveNext()) done = true;
            return result;
        }

        static IExpression ParseResolved(
            IModuleInstance module,
            INamedExpressionTuple leftArguments,
            IEnumerator<TokenData> tokens,
            IContext context,
            ref bool done) {
            if (!tokens.MoveNext()) done = true;
            if (done) return new ModuleReference {Reference = module};

            if (tokens.Current.Type != Token.IdentifierLiteral) return new ModuleReference {Reference = module};
            var nextIdentifier = (IIdentifierLiteral) tokens.Current.Data;
            if (nextIdentifier.Content.First() != '.') return new ModuleReference {Reference = module};
            var realIdentifier = nextIdentifier.Content; // .Substring(startIndex: 1);
            var resolved = module.Identifiers.LocalIdentifiers[realIdentifier];
            return ParseResolved((dynamic) resolved, leftArguments, tokens, context, ref done);
        }

        static IExpression ParseResolved(
            IFunctionInstance function,
            INamedExpressionTuple leftArguments,
            IEnumerator<TokenData> tokens,
            IContext context,
            ref bool done) {
            var pool = new List<IFunctionInstance> {function};

            return ParseFunctionOverloads(pool, leftArguments, tokens, context, ref done);
        }

        static IExpression ParseResolved(
            IFunctionOverloads function,
            INamedExpressionTuple leftArguments,
            IEnumerator<TokenData> tokens,
            IContext context,
            ref bool done) {
            var pool = function.Overloads;
            return ParseFunctionOverloads(pool, leftArguments, tokens, context, ref done);
        }

        static IExpression ParseFunctionOverloads(
            IList<IFunctionInstance> pool,
            INamedExpressionTuple leftArguments,
            IEnumerator<TokenData> tokens,
            IContext context,
            ref bool done) {
            var range = tokens.Current.Range;
            INamedExpressionTuple rightArguments = new NamedExpressionTuple();

            if (!tokens.MoveNext()) {
                done = true;
                return CreateFunctionInvocation(pool, range, leftArguments, rightArguments);
            }

            var filteredPool = FilterFunctionLeftArguments(pool, leftArguments);
            filteredPool = ParseAndFilterFunctionRightArguments(filteredPool, ref rightArguments, tokens, context, ref done);
            if (filteredPool.Count != 1)
                return null;
            // TODO: proper error handling
            return CreateFunctionInvocation(filteredPool.First(), range, leftArguments, rightArguments);
        }

        static IList<IFunctionInstance> ParseAndFilterFunctionRightArguments(
            IList<IFunctionInstance> pool,
            ref INamedExpressionTuple rightArguments,
            IEnumerator<TokenData> tokens,
            IContext context,
            ref bool done) {
            if (!pool.IsEmpty() && (pool.Count != 1 || pool[0].RightArguments.Count != 0)) {
                rightArguments = Parse(tokens, context, ref done);
                return FilterFunctionRightArguments(pool, rightArguments);
            }
            return pool;
        }

        static IList<IFunctionInstance> FilterFunctionLeftArguments(IList<IFunctionInstance> pool, INamedExpressionTuple leftArguments) {
            return pool.Where(
                    f => {
                        if (f.Declaration.LeftArguments == null) return true;
                        if (f.Declaration.LeftArguments.Count > leftArguments.Tuple.Count) return false;
                        var o = leftArguments.Tuple.Count - f.Declaration.LeftArguments.Count;
                        foreach (var fArg in f.Declaration.LeftArguments) {
                            var givenArg = leftArguments.Tuple[o];
                            if (!CanImplicitConvertExpressionTo(givenArg.Expression, fArg.Type)) return false;
                            o++;
                        }
                        return f.Declaration.LeftArguments.Count <= leftArguments.Tuple.Count;
                    })
                .ToList();
        }

        static bool CanImplicitConvertExpressionTo(IExpression givenArgExpression, IModuleInstance fArgType) {
            return true;
        }

        static IList<IFunctionInstance> FilterFunctionRightArguments(IList<IFunctionInstance> pool, INamedExpressionTuple rightArguments) {
            return pool.Where(
                    f => f.Declaration.RightArguments == null ||
                        f.Declaration.MandatoryRightArgumentCount() <= rightArguments.Tuple.Count &&
                        (f.Declaration.MaxRightArgumentCount() == null || !(f.Declaration.MaxRightArgumentCount() < rightArguments.Tuple.Count)))
                .ToList();
        }

        static IExpression CreateFunctionInvocation(
            IList<IFunctionInstance> pool,
            TextFileRange range,
            INamedExpressionTuple leftArguments,
            INamedExpressionTuple rightArguments) {
            var filteredPool = FilterFunctionLeftArguments(pool, leftArguments);
            filteredPool = FilterFunctionRightArguments(filteredPool, rightArguments);
            return filteredPool.Count != 1 ? null : CreateFunctionInvocation(filteredPool.First(), range, leftArguments, rightArguments);
            // TODO: proper error handling
        }

        static IExpression CreateFunctionInvocation(
            IFunctionInstance function,
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

        static INamedExpressionTuple AssignArguments(NamedCollection<IArgumentInstance> instances, INamedExpressionTuple arguments) {
            var result = new NamedExpressionTuple();
            var o = arguments.Tuple.Count - instances.Count;
            foreach (var instance in instances) {
                // TODO: check that conversion exists
                result.Tuple.Add(arguments.Tuple[o]);
                arguments.Tuple.RemoveAt(o);
            }
            // TODO: check that enough arguments are given
            return result;
        }
    }
}
