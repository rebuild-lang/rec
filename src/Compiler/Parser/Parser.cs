using System;
using System.Collections.Generic;
using REC.AST;
using REC.Scope;
using REC.Scanner;

namespace REC.Parser
{
    public static class Parser
    {
        public static IBlock ParseBlock(IEnumerator<TokenData> tokens, IScope parentScope) {
            var block = new Block();
            var scope = new Scope.Scope(parent: parentScope);
            do {
                var token = tokens.Current;
                switch (token.Type) {
                    case Token.NewLineIndentation:
                        break;
                    case Token.CompileTimeOperator:
                    case Token.BracketOpen:
                    case Token.StringLiteral:
                    case Token.NumberLiteral:
                    case Token.IdentifierLiteral:
                    case Token.OperatorLiteral:
                        var expression = ParseBlockExpression(tokens, scope);
                        if (expression != null) block.Expressions.Add(expression);
                        break;
                    case Token.BracketClose:
                    case Token.SquareBracketOpen:
                    case Token.SquareBracketClose:
                    case Token.InvalidCharacter:
                    case Token.Comment:
                    case Token.CommaSeparator:
                    case Token.SemicolonSeparator:
                        // TODO: these are errors
                        break;
                    default:
                        throw new ArgumentOutOfRangeException();
                }
            } while (tokens.MoveNext());
            return block;
        }

        static IExpression ParseBlockExpression(IEnumerator<TokenData> tokens, IScope scope) {
            var token = tokens.Current;
            var isCompileTime = token.Type == Token.CompileTimeOperator;
            if (isCompileTime) {
                if (!tokens.MoveNext()) return null; // TODO: create error Expression
                token = tokens.Current;
            }
            if (token.Type == Token.IdentifierLiteral) {
                var identifier = (IIdentifierLiteral)token.Data;
                if (identifier.Content == "var") {
                    return null; // ParseVarDecl(tokens, scope, isCompileTime);
                }
                if (identifier.Content == "def") {
                    return null; // ParseDefineDecl(tokens, scope, isCompileTime);
                }
                if (identifier.Content == "fn") {
                    return null; // ParseFunctionDecl(tokens, scope, isCompileTime);
                }
                if (identifier.Content == "module") {
                    return null; // ParseModuleDecl(tokens, scope, isCompileTime);
                }
                if (identifier.Content == "with") {
                    return null; // ParseWithExpression(tokens, scope, isCompileTime);
                }
            }
            return ParseExpression(tokens, scope, isCompileTime);
        }

        static IExpression ParseExpression(IEnumerator<TokenData> tokens, IScope scope, bool isCompileTime) {
            var token = tokens.Current;
            if (token.Type == Token.IdentifierLiteral) {
                var identifier = (IIdentifierLiteral) token.Data;
                var resolved = scope[identifier.Content];
                if (null == resolved) return null; // TODO proper error handling "unknown entry"
                var expr = ParseResolved((dynamic) resolved, tokens, scope);
                return /*isCompileTime ? Evaluate(expr) :*/ expr;
            }
            // TODO: Untangle Operators
            // 
            return null;
        }

        static IExpression ParseResolved(IFunction function, IEnumerator<TokenData> tokens, IScope scope) {
            if (!tokens.MoveNext()) return null;
            var invocation = new Invocation {Function = function.Declaration};
            var argument = ParseExpression(tokens, scope, false);
            if (argument != null) invocation.Right.Add(argument);
            return invocation;
        }
    }
}