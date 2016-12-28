using System.Collections.Generic;
using REC.AST;
using REC.Scanner;
using REC.Scope;
using REC.Tools;

namespace REC.Parser
{
    static class FunctionDeclParser
    {
        internal static IExpression Parse(IEnumerator<TokenData> tokens, IScope parentScope, ref bool done) {
            if (!tokens.MoveNext()) done = true;
            if (done) return null;
            var functionDecl = new FunctionDeclaration {
                StaticScope = new Scope {Parent = parentScope}
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

            if (token.Type == Token.OperatorLiteral && ((IIdentifierLiteral) token.Data).Content == "&") {
                functionDecl.IsCompileTimeOnly = true;
                if (!tokens.MoveNext()) done = true;
                if (done) return null; // TODO: report missing name
            }

            if (token.Type != Token.IdentifierLiteral && token.Type != Token.IdentifierLiteral) {
                // TODO: error handling name missing
                // handling: mark functionDecl as error and continue to parse
                return null;
            }
            // TODO: allow more complex identifiers?
            functionDecl.Name = ((IIdentifierLiteral) token.Data).Content;
            // TODO: restrict valid identifiers (->, keywords are not allowed)
            // TODO: check for duplicate identifiers & function overloads
            parentScope.Identifiers.Add(
                new FunctionEntry {
                    FunctionDeclarations = {functionDecl}
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
                // TODO: check for uninitialized unassignable results
            }
            else functionDecl.Results = new NamedCollection<IArgumentDeclaration>();

            #endregion

            #region Implementation

            if (token.Type == Token.BlockStartIndentation) {
                var contentBlock = (BlockLiteral) token.Data;
                functionDecl.Implementation = Parser.ParseBlock(contentBlock, functionDecl.StaticScope);
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
                var isAssignable = false;
                if (token.Type == Token.OperatorLiteral && ((IIdentifierLiteral) token.Data).Content == "*") {
                    isAssignable = true;
                    if (!tokens.MoveNext()) done = true;
                    if (done) return result; // TODO: report missing value & dangling open bracket
                    token = tokens.Current;
                }
                if (token.Type != Token.IdentifierLiteral) break;
                var argName = ((IIdentifierLiteral) token.Data).Content;
                // TODO: check for duplicate argument names

                var argument = new ArgumentDeclaration {Name = argName, IsAssignable = isAssignable};
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
                        var typeName = ((IIdentifierLiteral) token.Data).Content;
                        if (scope.Identifiers[typeName] is IModuleEntry typeEntry
                            && typeEntry.ModuleDeclaration.IsType()) {
                            argument.Type = typeEntry.ModuleDeclaration;
                        }
                        else {
                            // TODO: report missing type   
                        }

                        if (!tokens.MoveNext()) done = true;
                        if (done) return result; // TODO: report dangling open bracket
                        token = tokens.Current;
                    }
                }

                #endregion

                #region Default Value

                if (token.Type == Token.OperatorLiteral && ((IIdentifierLiteral) token.Data).Content == "=") {
                    if (!tokens.MoveNext()) done = true;
                    if (done) return result; // TODO: report missing value & dangling open bracket
                    argument.Value = ExpressionParser.Parse(tokens, scope, ref done);
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
    }
}
