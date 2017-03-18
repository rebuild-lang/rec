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
            var overloadBuilder = pool.Select(
                    f => (IOverloadInvocationBuilder)new OverloadInvocationBuilder(f, range))
                .ToList();

            foreach (var builder in overloadBuilder) {
                builder.RetireLeftArguments(leftArguments);
            }

            if (!tokens.MoveNext()) {
                done = true;
                return CreateFunctionInvocation(overloadBuilder);
            }

            ParseFunctionRightArguments(overloadBuilder, tokens, context, ref done);

            return CreateFunctionInvocation(overloadBuilder);
        }

        static void ParseFunctionRightArguments(
            IList<IOverloadInvocationBuilder> builders,
            IEnumerator<TokenData> tokens,
            IContext context,
            ref bool done) {
            while (builders.Any(b => b.IsActive)) {
                var rightArguments = Parse(tokens, context, ref done);
                foreach (var builder in builders) {
                    if (!builder.IsActive) continue;
                    foreach (var rightArgument in rightArguments.Tuple)
                    {
                        if (!builder.IsActive) continue;
                        builder.RetireRightArgument(rightArgument);
                    }
                }
            }
        }

        static IExpression CreateFunctionInvocation(IEnumerable<IOverloadInvocationBuilder> builders) {
            var completable = builders.Where(b => b.IsCompletable).ToList();
            if (completable.Count != 1) {
                // TODO: proper error handling
                return null;
            }
            return completable.First().Build();
        }
    }
}
