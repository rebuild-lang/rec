using REC.AST;
using REC.Instance;
using REC.Scanner;
using REC.Tools;
using System.Collections.Generic;
using System.Linq;

namespace REC.Parser
{
    static class ExpressionParser
    {
        internal static IExpression Parse(ITokenIterator tokens, IContext context) {
            IExpression result = null;
            while (tokens.Active) {
                if (null != tokens.CurrentInstance) {
                    var parsed = ParseInstance(result, tokens, context);
                    if (parsed == result) return result;
                    result = parsed;
                    continue; // Parse function moved the token!
                }
                var token = tokens.Current;
                // ReSharper disable once SwitchStatementMissingSomeCases
                switch (token.Type) {
                case Token.BracketClose:
                case Token.SemicolonSeparator:
                    return result; // outer expression done

                case Token.CommaSeparator:
                    // TODO: consider parsing bracket less tuple
                    return result;

                case Token.BracketOpen:
                    if (null != result) return result;
                    result = ParseTuple(tokens, context);
                    continue; // Parse function moved the token!

                case Token.IdentifierLiteral:
                    var identifierLiteral = (IIdentifierLiteral) token.Data;
                    var instance = context.Identifiers[identifierLiteral.Content];
                    if (null == instance) {
                        if (null != result) return result;
                        result = identifierLiteral;
                        break;
                    }
                    tokens.CurrentInstance = instance;
                    continue; // trigger instance cache handling

                case Token.StringLiteral:
                case Token.NumberLiteral:
                case Token.BlockStartIndentation:
                    if (null != result) return result;
                    result = (ILiteral) token.Data;
                    break;
                }
                tokens.MoveNext();
            }
            return result;
        }

        internal static INamedExpressionTuple ParseTuple(ITokenIterator tokens, IContext context) {
            if (tokens.Done) return null;
            var withBracket = tokens.Current.Type == Token.BracketOpen;
            if (withBracket) tokens.MoveNext(); // move beyond opening bracket
            try {
                return ParseTupleInto(new NamedExpressionTuple(), tokens, context);
            }
            finally {
                if (withBracket) {
                    if (tokens.Done) {
                        // error
                    }
                    else if (tokens.Current.Type != Token.BracketClose) {
                        // missing closing bracket
                    }
                    else {
                        tokens.MoveNext(); // move beyond closing bracket
                    }
                }
            }
        }

        static INamedExpressionTuple ParseTupleInto(INamedExpressionTuple result, ITokenIterator tokens, IContext context) {
            while (tokens.Active) {
                var expressionName = ParseExpressionName(tokens);
                var expression = Parse(tokens, context);
                if (null != expression) {
                    result.Tuple.Add(new NamedExpression {Expression = expression, Name = expressionName});
                }

                if (tokens.Done) break;
                // ReSharper disable once SwitchStatementMissingSomeCases
                switch (tokens.Current.Type) {
                case Token.CommaSeparator:
                    tokens.MoveNext();
                    continue;

                case Token.BracketClose:
                case Token.SemicolonSeparator:
                    return result;
                }
            }
            return result;
        }

        static string ParseExpressionName(ITokenIterator tokens) {
            if (!tokens.HasNext) return null;
            var token = tokens.Current;
            if (token.Type != Token.IdentifierLiteral || tokens.Next.Type != Token.ColonSeparator) return null;
            var literal = (IIdentifierLiteral) token.Data;
            if (null != literal.SplittedFrom) return null;
            tokens.MoveNext();
            tokens.MoveNext();
            return literal.Content;
        }

        static IExpression ParseInstance(IExpression left, ITokenIterator tokens, IContext context) {
            var token = tokens.Current;
            var instance = tokens.CurrentInstance;
            var range = token.Range;
            switch (instance) {
            case ITypedInstance typed:
                if (null != left) return left;
                tokens.MoveNext();
                return new TypedReference {Instance = typed, Type = typed.Type, Range = range};

            case IModuleInstance module:
                var result = ParseModuleReference(module, left, tokens, context);
                if (null != result) return result;
                if (null != left) return left;
                tokens.MoveNext();
                return new ModuleReference {Reference = module, Range = range};

            case IFunctionInstance function:
                return ParseFunctionOverloads(new List<IFunctionInstance> {function}, left, tokens, context);

            case IFunctionOverloads overloads:
                return ParseFunctionOverloads(overloads.Overloads, left, tokens, context);

            default:
                return null;
            }
        }

        static IExpression ParseModuleReference(IModuleInstance module, IExpression left, ITokenIterator tokens, IContext context) {
            if (!tokens.HasNext) return null;
            var nextToken = tokens.Next;
            if (nextToken.Type != Token.IdentifierLiteral) return null;
            var nextIdentifier = (IIdentifierLiteral) nextToken.Data;
            if (nextIdentifier.Content.First() != '.') return null;
            var realIdentifier = nextIdentifier.Content;
            var instance = module.Identifiers.LocalIdentifiers[realIdentifier];
            if (instance == null) return null;
            tokens.MoveNext();
            tokens.CurrentInstance = instance;
            return ParseInstance(left, tokens, context);
        }

        static IExpression ParseFunctionOverloads(IList<IFunctionInstance> pool, IExpression left, ITokenIterator tokens, IContext context) {
            var range = tokens.Current.Range;
            var overloadBuilder = pool.Select(f => (IOverloadInvocationBuilder) new OverloadInvocationBuilder(f, range)).ToList();
            foreach (var builder in overloadBuilder) {
                builder.RetireLeftArgument(left);
            }
            if (!overloadBuilder.Any(b => b.IsActive || b.IsCompletable)) return left; // no overload can succeed

            if (!tokens.MoveNext()) {
                return CreateFunctionInvocation(overloadBuilder);
            }

            ParseFunctionRightArguments(overloadBuilder, tokens, context);

            return CreateFunctionInvocation(overloadBuilder);
        }

        static void ParseFunctionRightArguments(IList<IOverloadInvocationBuilder> builders, ITokenIterator tokens, IContext context) {
            while (builders.Any(b => b.IsActive) && tokens.Active) {
                var rightArguments = ParseTuple(tokens, context);
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
