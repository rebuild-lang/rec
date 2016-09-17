using System;
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
            var scope = new Scope.Scope(parentScope);
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
                if (identifier.Content == "fn") return null; // ParseFunctionDecl(tokens, scope, isCompileTime);
                if (identifier.Content == "module") return null; // ParseModuleDecl(tokens, scope, isCompileTime);
                if (identifier.Content == "with") return null; // ParseWithExpression(tokens, scope, isCompileTime);
            }
            var result = ParseExpressionTuple(tokens, scope, ref done);
            if (!isCompileTime) return result;
            return CompileTime.Execute(result, scope);
        }

        static INamedExpressionTuple ParseExpressionTuple(IEnumerator<TokenData> tokens, IScope scope, ref bool done) {
            var result = new NamedExpressionTuple();
            while (!done) {
                var token = tokens.Current;
                switch (token.Type) {
                    case Token.IdentifierLiteral:
                        var identifierLiteral = (IIdentifierLiteral)token.Data;
                        var resolved = scope[identifierLiteral.Content];
                        if (null != resolved) {
                            var subexpression = ParseResolved((dynamic)resolved, result, tokens, scope, ref done);
                            result.tuple.Add(new NamedExpression { Expression = subexpression != null ? subexpression : identifierLiteral });
                            continue;
                        }
                        result.tuple.Add(new NamedExpression { Expression = identifierLiteral });
                        break;
                    case Token.OperatorLiteral:
                        var operatorLiteral = (IIdentifierLiteral)token.Data;
                        // TODO: split operators with scope
                        result.tuple.Add(new NamedExpression { Expression = operatorLiteral });
                        break;
                    case Token.StringLiteral:
                        var stringLiteral = (IStringLiteral)token.Data;
                        result.tuple.Add(new NamedExpression { Expression = stringLiteral });
                        break;
                    case Token.NumberLiteral:
                        var numberLiteral = (INumberLiteral)token.Data;
                        result.tuple.Add(new NamedExpression { Expression = numberLiteral });
                        break;
                    case Token.BlockStartIndentation:
                        var tokenBlock = (IBlockLiteral)token.Data;
                        result.tuple.Add(new NamedExpression { Expression = tokenBlock });
                        break;
                }
                if (done) break;
                if (!tokens.MoveNext()) done = true;
            }
            return result;
        }

        static IExpression ParseResolved(
            TypeConstruct typed,
            INamedExpressionTuple leftArguments,
            IEnumerator<TokenData> tokens,
            IScope scope,
            ref bool done) {
            
            var result = new AST.TypedReference {
                Range = tokens.Current.Range,
                Type = typed.Type,
                Declaration = typed.Declaration
            };
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
                if (f.LeftArguments.Count > leftArguments.tuple.Count) return false;
                var o = leftArguments.tuple.Count - f.LeftArguments.Count;
                foreach (var fArg in f.LeftArguments) {
                    var givenArg = leftArguments.tuple[o];
                    if (!CanImplicitConvertExpressionTo(givenArg.Expression, fArg.Type)) return false;
                    o++;
                }
                return f.LeftArguments.Count <= leftArguments.tuple.Count;
            }).ToList();
        }

        static bool CanImplicitConvertExpressionTo(IExpression givenArgExpression, IModuleDeclaration fArgType) {
            return true;
        }

        static IList<IFunctionDeclaration> FilterFunctionRightArguments(IList<IFunctionDeclaration> pool, INamedExpressionTuple rightArguments) {
            return pool.Where(
                f => {
                    if (f.RightArguments == null) return true;
                    if (f.MandatoryRightArgumentCount() > rightArguments.tuple.Count
                        || (f.MaxRightArgumentCount() != null && f.MaxRightArgumentCount() < rightArguments.tuple.Count))
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

        static ICollection<IExpression> AssignArguments(NamedCollection<IArgumentDeclaration> argumentDeclarations, INamedExpressionTuple arguments) {
            var result = new List<IExpression>();
            return result;
        }
    }
}