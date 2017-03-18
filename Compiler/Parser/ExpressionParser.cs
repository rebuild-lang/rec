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
        internal static INamedExpressionTuple Parse(ITokenIterator tokens, IContext context) {
            return ParseResult(new NamedExpressionTuple(), tokens, context);
        }

        static INamedExpressionTuple ParseResult(INamedExpressionTuple result, ITokenIterator tokens, IContext context) {
            while (tokens.Active) {
                var token = tokens.Current;
                switch (token.Type) {
                case Token.OperatorLiteral:
                    var operatorLiteral = (IIdentifierLiteral) token.Data;
                    if (operatorLiteral.Content == "&") {
                        if (!tokens.MoveNext()) return result; // TODO: report error: missing tokens

                        var inner = Parse(tokens, context);
                        var execResult = CompileTime.Execute(inner, context);
                        if (execResult != null) {
                            if (execResult is INamedExpressionTuple execNamedTuple)
                                result.Tuple.AddRange(execNamedTuple.Tuple);
                            else
                                result.Tuple.Add(new NamedExpression {Expression = execResult});
                        }
                        continue;
                    }
                    using (var newTokens = OperatorLiteralSplitter.Split(operatorLiteral, context).Before(tokens.Enumerator).GetIterator()) {
                        try {
                            return ParseResult(result, newTokens, context);
                        }
                        finally {
                            tokens.Active = newTokens.Active;
                        }
                    }
                case Token.IdentifierLiteral:
                    var identifierLiteral = (IIdentifierLiteral) token.Data;
                    var resolved = context.Identifiers[identifierLiteral.Content];
                    if (null != resolved) {
                        var subexpression = ParseResolved((dynamic) resolved, result, tokens, context);
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
                if (tokens.Done) break;
                tokens.MoveNext();
            }
            return result;
        }

        static IExpression ParseResolved(
            ITypedInstance typed,
            INamedExpressionTuple leftArguments,
            ITokenIterator tokens,
            IContext context) {
            var result = new TypedReference {
                Range = tokens.Current.Range,
                Type = typed.Type,
                Instance = typed
            };

            tokens.MoveNext();
            return result;
        }

        static IExpression ParseResolved(
            IModuleInstance module,
            INamedExpressionTuple leftArguments,
            ITokenIterator tokens,
            IContext context) {
            if (!tokens.MoveNext()) return new ModuleReference {Reference = module};

            if (tokens.Current.Type != Token.IdentifierLiteral) return new ModuleReference {Reference = module};
            var nextIdentifier = (IIdentifierLiteral) tokens.Current.Data;
            if (nextIdentifier.Content.First() != '.') return new ModuleReference {Reference = module};
            var realIdentifier = nextIdentifier.Content; // .Substring(startIndex: 1);
            var resolved = module.Identifiers.LocalIdentifiers[realIdentifier];
            return ParseResolved((dynamic) resolved, leftArguments, tokens, context);
        }

        static IExpression ParseResolved(
            IFunctionInstance function,
            INamedExpressionTuple leftArguments,
            ITokenIterator tokens,
            IContext context) {
            var pool = new List<IFunctionInstance> {function};

            return ParseFunctionOverloads(pool, leftArguments, tokens, context);
        }

        static IExpression ParseResolved(
            IFunctionOverloads function,
            INamedExpressionTuple leftArguments,
            ITokenIterator tokens,
            IContext context) {
            var pool = function.Overloads;
            return ParseFunctionOverloads(pool, leftArguments, tokens, context);
        }

        static IExpression ParseFunctionOverloads(
            IList<IFunctionInstance> pool,
            INamedExpressionTuple leftArguments,
            ITokenIterator tokens,
            IContext context) {
            var range = tokens.Current.Range;
            var overloadBuilder = pool.Select(
                    f => (IOverloadInvocationBuilder) new OverloadInvocationBuilder(f, range))
                .ToList();

            foreach (var builder in overloadBuilder) {
                builder.RetireLeftArguments(leftArguments);
            }

            if (!tokens.MoveNext()) {
                return CreateFunctionInvocation(overloadBuilder);
            }

            ParseFunctionRightArguments(overloadBuilder, tokens, context);

            return CreateFunctionInvocation(overloadBuilder);
        }

        static void ParseFunctionRightArguments(
            IList<IOverloadInvocationBuilder> builders,
            ITokenIterator tokens,
            IContext context) {
            while (builders.Any(b => b.IsActive)) {
                var rightArguments = Parse(tokens, context);
                foreach (var builder in builders) {
                    if (!builder.IsActive) continue;
                    foreach (var rightArgument in rightArguments.Tuple) {
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
