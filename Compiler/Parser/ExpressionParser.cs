using REC.AST;
using REC.Instance;
using REC.Scanner;
using REC.Tools;
using System.Collections.Generic;
using System.Linq;
using REC.Execution;
using TypedReference = REC.AST.TypedReference;

namespace REC.Parser
{
    internal interface IExpressionParser
    {
        bool AllowNamed { get; }
        INamedExpression Parse(ITokenIterator tokens, IContext context, string name = null);
    }

    class DefaultExpressionParser : IExpressionParser
    {
        public bool AllowNamed => true;

        public INamedExpression Parse(ITokenIterator tokens, IContext context, string name) {
            var expression = ExpressionParser.Parse(tokens, context);
            return new NamedExpression {Expression = expression, Name = name};
        }
    }

    class EpressionLiteralParser : IExpressionParser
    {
        public bool AllowNamed => true;

        public INamedExpression Parse(ITokenIterator tokens, IContext context, string name) {
            var expression = ExpressionParser.Parse(tokens, context);
            var literal = new ExpressionLiteral {Expression = expression};
            return new NamedExpression {Expression = literal, Name = name};
        }
    }

    class VariableDeclExpressionParser : IExpressionParser
    {
        public bool AllowNamed => false;

        public INamedExpression Parse(ITokenIterator tokens, IContext context, string name) {
            var variable = VariableDeclParser.Parse(tokens, context);
            return new NamedExpression {Expression = variable, Name = name};
        }
    }


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

        internal static string ParseExpressionName(ITokenIterator tokens) {
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

            if (tokens.MoveNext()) {
                ParseFunctionRightArguments(overloadBuilder, tokens, context);
            }

            var invocation = CreateFunctionInvocation(overloadBuilder);
            return RunCompileTime(invocation, context);
        }

        static IExpression RunCompileTime(IFunctionInvocation invocation, IContext context) {
            if (null == invocation || !invocation.Function.IsCompileTimeOnly) return invocation;
            var result = CompileTime.Execute(invocation, context);
            if (result is INamedExpressionTuple tuple 
                && tuple.Tuple.Count == 1
                && tuple.Tuple.First().Expression is ITypedValue value
                && value.Type.Name == "Expr") {
                return value.Type.GetToNetType()(value.Data);
            }
            return result;
        }

        static IExpressionParser ParserForType(IModuleInstance type, IContext context) {
            if (null == type) return null;
            if (type.Name == "Expression") return new EpressionLiteralParser();
            return new DefaultExpressionParser();
        }

        static void ParseFunctionRightArguments(IList<IOverloadInvocationBuilder> builders, ITokenIterator tokens, IContext context) {
            var withBracket = tokens.Current.Type == Token.BracketOpen;
            if (withBracket) tokens.MoveNext(); // move beyond opening bracket
            try {
                var tuples = builders.Where(b => b.IsActive).Select(b => (b, tokens.Backup())).ToList();
                while (builders.Any(b => b.IsActive) && tokens.Active) {
                    var nextTuples = new List<(IOverloadInvocationBuilder, ITokenIteratorBackup)>();
                    foreach (var (builder, backup) in tuples) {
                        if (!builder.IsActive) continue;
                        var retired = false;
                        var nextType = builder.NextRightArgumentType();
                        var parser = ParserForType(nextType, context);
                        if (null != parser && !parser.AllowNamed) {
                            tokens.Restore(backup);
                            var argument = parser.Parse(tokens, context);
                            if (null != argument) {
                                builder.RetireRightArgument(argument);
                                retired = true;
                            }
                        }
                        else {
                            var argumentName = ParseExpressionName(tokens);
                            if (null != argumentName) {
                                nextType = builder.RightArgumentTypeByName(argumentName);
                                parser = ParserForType(nextType, context);
                            }
                            if (null != parser) {
                                tokens.Restore(backup);
                                var argument = parser.Parse(tokens, context, argumentName);
                                if (null != argument) {
                                    builder.RetireRightArgument(argument);
                                    retired = true;
                                }
                            }
                        }
                        if (!retired) continue;
                        if (!tokens.Active) continue;

                        // ReSharper disable once SwitchStatementMissingSomeCases
                        switch (tokens.Current.Type) {
                        case Token.CommaSeparator:
                            tokens.MoveNext();
                            break;

                        case Token.BracketClose:
                        case Token.SemicolonSeparator:
                            retired = false;
                            break;
                        }

                        if (retired && tokens.Active && builder.IsActive) {
                            nextTuples.Add((builder, tokens.Backup()));
                        }
                    }
                    tuples = nextTuples;
                }
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

        static IFunctionInvocation CreateFunctionInvocation(IEnumerable<IOverloadInvocationBuilder> builders) {
            var completable = builders.Where(b => b.IsCompletable).ToList();
            if (completable.Count != 1) {
                // TODO: proper error handling
                return null;
            }
            return completable.First().Build();
        }
    }
}
