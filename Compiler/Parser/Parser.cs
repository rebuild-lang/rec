using System.Collections.Generic;
using System.Linq;
using REC.AST;
using REC.Execution;
using REC.Scope;
using REC.Scanner;
using REC.Tools;

namespace REC.Parser
{
    public static class Parser
    {
        public static IExpressionBlock ParseBlock(IBlockLiteral tokenBlock, IScope parentScope) {
            var block = new ExpressionBlock();
            var scope = new Scope {Parent = parentScope};
            foreach (var tokenLine in tokenBlock.Lines) {
                using (var it = tokenLine.Tokens.GetEnumerator()) {
                    if (it.MoveNext()) {
                        var done = false;
                        var expression = ParseBlockExpression(it, scope, ref done);
                        if (expression != null) block.Expressions.Add(expression);
                    }
                }
            }
            return block;
        }

        static IExpression ParseBlockExpression(IEnumerator<TokenData> tokens, IScope scope, ref bool done) {
            var token = tokens.Current;
            var isCompileTime = false;
            if (token.Type == Token.OperatorLiteral && token.Range.Text == "&") {
                isCompileTime = true;
                if (!tokens.MoveNext()) done = true;
                if (done) return null;
            }
            if (token.Type == Token.IdentifierLiteral) {
                var identifier = (IIdentifierLiteral)token.Data;
                if (identifier.Content == "var") return null; // ParseVarDecl(tokens, scope, isCompileTime);
                if (identifier.Content == "def") return null; // ParseDefineDecl(tokens, scope, isCompileTime);
                if (identifier.Content == "fn") return ParseFunctionDecl(tokens, scope, isCompileTime, ref done);
                if (identifier.Content == "module") return null; // ParseModuleDecl(tokens, scope, isCompileTime);
                if (identifier.Content == "with") return null; // ParseWithExpression(tokens, scope, isCompileTime);
            }
            var result = ParseExpressionTuple(tokens, scope, ref done);
            if (!isCompileTime) return result;
            return CompileTime.Execute(result, scope);
        }

        static IExpression ParseFunctionDecl(IEnumerator<TokenData> tokens, IScope parentScope, bool isCompileTime, ref bool done) {
            if (!tokens.MoveNext()) done = true;
            if (done) return null;
            var functionDecl = new FunctionDeclaration {
                IsCompileTimeOnly = isCompileTime,
                StaticScope = new Scope { Parent = parentScope},
            };
            var token = tokens.Current;
            #region Left Arguments
            if (token.Type == Token.BracketOpen) {
                functionDecl.LeftArguments = ParseArgumentsDecl(tokens, functionDecl.StaticScope, ref done);
                if (done) return null;
                token = tokens.Current;
            }
            else functionDecl.LeftArguments = new NamedCollection<IArgumentDeclaration>();
            #endregion
            #region Identifier
            if (token.Type != Token.IdentifierLiteral && token.Type != Token.IdentifierLiteral) {
                // TODO: error handling name missing
                // handling: mark functionDecl as error and continue to parse
                return null;
            }
            // TODO: allow more complex identifiers?
            functionDecl.Name = ((IIdentifierLiteral)token.Data).Content;
            // TODO: restrict valid identifiers (->, keywords are not allowed)
            // TODO: check for duplicate identifiers & function overloads
            parentScope.Identifiers.Add(
                new FunctionEnty {
                    FunctionDeclarations = { functionDecl }
                });
            if (!tokens.MoveNext()) done = true;
            if (done) return functionDecl; // fully forward declared
            #endregion
            #region Right Arguments
            functionDecl.RightArguments = ParseArgumentsDecl(tokens, functionDecl.StaticScope, ref done);
            if (done) return functionDecl;
            #endregion
            #region Results
            token = tokens.Current;
            if (token.Type == Token.OperatorLiteral && ((IIdentifierLiteral) token.Data).Content == "->") {
                if (!tokens.MoveNext()) done = true; // jump over the arrow
                if (done) return functionDecl;
                functionDecl.Results = ParseArgumentsDecl(tokens, functionDecl.StaticScope, ref done);
                if (done) return functionDecl;
                token = tokens.Current;
            }
            else functionDecl.Results = new NamedCollection<IArgumentDeclaration>();
            #endregion
            #region Implementation
            if (token.Type == Token.BlockStartIndentation) {
                var contentBlock = (BlockLiteral) token.Data;
                functionDecl.Implementation = ParseBlock(contentBlock, functionDecl.StaticScope);
                if (!tokens.MoveNext()) done = true;
                if (done) return functionDecl;
            } 
            #endregion
            // TODO: check for extra tokens
            return functionDecl;
        }

        static NamedCollection<IArgumentDeclaration> ParseArgumentsDecl(IEnumerator<TokenData> tokens, IScope scope, ref bool done) {
            var result = new NamedCollection<IArgumentDeclaration>();
            var token = tokens.Current;
            var withBracket = token.Type == Token.BracketOpen;
            if (withBracket) {
                if (!tokens.MoveNext()) done = true;
                if (done) return result; // TODO: report error dangling open bracket
                token = tokens.Current;
            }
            var withComma = false;
            while (true) {
                if (token.Type != Token.IdentifierLiteral) break;
                var argName = ((IIdentifierLiteral) token.Data).Content;
                // TODO: check for duplicate argument names

                var argument = new ArgumentDeclaration {Name = argName};
                result.Add(argument);
                scope.Identifiers.Add(new ArgumentEntry {Argument = argument});

                if (!tokens.MoveNext()) done = true;
                if (done) return result; // TODO: report error dangling open bracket
                token = tokens.Current;

                #region Argument Type
                if (token.Type == Token.OperatorLiteral && ((IIdentifierLiteral) token.Data).Content == ":") {
                    if (!tokens.MoveNext()) done = true;
                    if (done) return result; // TODO: report missing type & dangling open bracket
                    token = tokens.Current;

                    // TODO: expand ParseTypeExpression
                    if (token.Type == Token.IdentifierLiteral) {
                        var typeName = ((IIdentifierLiteral)token.Data).Content;
                        var typeEntry = scope.Identifiers[typeName] as IModuleEntry;
                        if (typeEntry != null && typeEntry.ModuleDeclaration.IsType()) {
                            argument.Type = typeEntry.ModuleDeclaration;
                        }
                        // TODO: report missing type

                        if (!tokens.MoveNext()) done = true;
                        if (done) return result; // TODO: report dangling open bracket
                        token = tokens.Current;
                    }
                }
                #endregion
                #region Default Value
                if (token.Type == Token.OperatorLiteral && ((IIdentifierLiteral)token.Data).Content == "=") {
                    if (!tokens.MoveNext()) done = true;
                    if (done) return result; // TODO: report missing value & dangling open bracket
                    argument.Value = ParseExpressionTuple(tokens, scope, ref done);
                    if (done) return result; // TODO: report missing dangling open bracket
                    token = tokens.Current;
                }
                #endregion
                #region Comma Separator
                if (token.Type == Token.CommaSeparator) {
                    if (!withComma && result.Count > 1) {
                        // TODO: report inconsistent use of commas
                        // handling: ignore
                    }
                    withComma = true;
                    if (!tokens.MoveNext()) done = true;
                    if (done) return result;
                }
                else if (withComma) {
                    // TODO: report inconsistent use of commas
                    // handling: ignore
                }
                #endregion
            }
            if (withBracket) {
                if (token.Type == Token.BracketClose) {
                    if (!tokens.MoveNext()) done = true;
                    if (done) return result;
                }
                else {
                    // TODO: report error
                    // handling: analyze token stream and guess where to stop
                }
            }
            return result;
        }

        static INamedExpressionTuple ParseExpressionTuple(IEnumerator<TokenData> tokens, IScope scope, ref bool done) {
            var result = new NamedExpressionTuple();
            while (!done) {
                var token = tokens.Current;
                switch (token.Type) {
                    case Token.IdentifierLiteral:
                        var identifierLiteral = (IIdentifierLiteral)token.Data;
                        var resolved = scope.Identifiers[identifierLiteral.Content];
                        if (null != resolved) {
                            var subexpression = ParseResolved((dynamic)resolved, result, tokens, scope, ref done);
                            result.Tuple.Add(new NamedExpression { Expression = subexpression != null ? subexpression : identifierLiteral });
                            continue;
                        }
                        result.Tuple.Add(new NamedExpression { Expression = identifierLiteral });
                        break;
                    case Token.OperatorLiteral:
                        var operatorLiteral = (IIdentifierLiteral)token.Data;
                        // TODO: split operators with parentScope
                        result.Tuple.Add(new NamedExpression { Expression = operatorLiteral });
                        break;
                    case Token.StringLiteral:
                        var stringLiteral = (IStringLiteral)token.Data;
                        result.Tuple.Add(new NamedExpression { Expression = stringLiteral });
                        break;
                    case Token.NumberLiteral:
                        var numberLiteral = (INumberLiteral)token.Data;
                        result.Tuple.Add(new NamedExpression { Expression = numberLiteral });
                        break;
                    case Token.BlockStartIndentation:
                        var tokenBlock = (IBlockLiteral)token.Data;
                        result.Tuple.Add(new NamedExpression { Expression = tokenBlock });
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
            FunctionEnty function,
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
            return CreateFunctionInvocation(filteredPool.First(), range, leftArguments, rightArguments);
        }

        static IList<IFunctionDeclaration> ParseAndFilterFunctionRightArguments(IList<IFunctionDeclaration> pool, ref INamedExpressionTuple rightArguments, IEnumerator<TokenData> tokens, IScope scope, ref bool done) {
            if (!pool.IsEmpty()) {
                rightArguments = ParseExpressionTuple(tokens, scope, ref done);
                return FilterFunctionRightArguments(pool, rightArguments);
            }
            return pool;
        }

        static IList<IFunctionDeclaration> FilterFunctionLeftArguments(IList<IFunctionDeclaration> pool, INamedExpressionTuple leftArguments) {
            return pool.Where(f => {
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
                        || (f.MaxRightArgumentCount() != null && f.MaxRightArgumentCount() < rightArguments.Tuple.Count))
                        return false;
                    return true;
                }).ToList();
        }


        static IExpression CreateFunctionInvocation(IList<IFunctionDeclaration> pool, TextFileRange range, INamedExpressionTuple leftArguments, INamedExpressionTuple rightArguments) {
            var filteredPool = FilterFunctionLeftArguments(pool, leftArguments);
            filteredPool = FilterFunctionRightArguments(filteredPool, rightArguments);
            if (filteredPool.Count != 1) return null;
            // TODO: proper error handling
            return CreateFunctionInvocation(filteredPool.First(), range, leftArguments, rightArguments);
        }

        static IExpression CreateFunctionInvocation(IFunctionDeclaration function, TextFileRange range, INamedExpressionTuple leftArguments, INamedExpressionTuple rightArguments) {
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
            foreach (var argument in arguments.Tuple) {
                result.Tuple.Add(argument);
            }
            arguments.Tuple.Clear();
            // TODO: check that enough arguments are given
            // TODO: check that conversion exists
            // TODO: consume only left arguments that are used
            return result;
        }
    }
}