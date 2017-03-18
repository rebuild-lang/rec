using System.Collections.Generic;
using REC.AST;
using REC.Execution;
using REC.Instance;
using REC.Scanner;

namespace REC.Parser
{
    class VariableDeclParser
    {
        public static IExpression Parse(ITokenIterator tokens, IContext context) {
            if (!tokens.MoveNext()) return null;
            var token = tokens.Current;
            var result = new NamedExpressionTuple();

            while (true) {
                var isAssignable = false;
                var isCompileTime = false;
                while (true) {
                    if (token.Type == Token.OperatorLiteral && ((IIdentifierLiteral)token.Data).Content == "*")
                    {
                        isAssignable = true;
                        if (!tokens.MoveNext()) return result; // TODO: report missing value
                        token = tokens.Current;
                        continue;
                    }
                    if (token.Type == Token.OperatorLiteral && ((IIdentifierLiteral)token.Data).Content == "&")
                    {
                        isCompileTime = true;
                        if (!tokens.MoveNext()) return result; // TODO: report missing value
                        token = tokens.Current;
                        continue;
                    }
                    break;
                }

                #region Identifier

                if (token.Type != Token.IdentifierLiteral) break;
                var name = ((IIdentifierLiteral) token.Data).Content;
                // TODO: check for duplicate names

                var variable = new VariableDeclaration {Name = name, IsAssignable = isAssignable, IsCompileTimeOnly = isCompileTime};

                try {
                    if (!tokens.MoveNext()) return result; // TODO: report missing type
                    token = tokens.Current;

                    #endregion

                    #region Type

                    if (token.Type == Token.OperatorLiteral && ((IIdentifierLiteral) token.Data).Content == ":") {
                        if (!tokens.MoveNext()) return result; // TODO: report missing type
                        token = tokens.Current;

                        // TODO: expand ParseTypeExpression
                        // TODO: ensure we get instancable type
                        if (token.Type == Token.IdentifierLiteral) {
                            var typeName = ((IIdentifierLiteral) token.Data).Content;
                            if (context.Identifiers[typeName] is IModuleInstance typeEntry
                                && typeEntry.IsType()) {
                                variable.Type = typeEntry;
                            }
                            else {
                                // TODO: report missing type   
                            }

                            if (!tokens.MoveNext()) return result; // done
                            token = tokens.Current;
                        }
                    }

                    #endregion

                    #region Initializer Value

                    if (token.Type == Token.OperatorLiteral && ((IIdentifierLiteral) token.Data).Content == "=") {
                        if (!tokens.MoveNext()) return result; // TODO: report missing value
                        variable.Value = ExpressionParser.Parse(tokens, context);
                        if (tokens.Done) return result; // done
                        token = tokens.Current;
                    }

                    #endregion

                }
                finally {
                    // TODO: ensure variable got a type!

                    result.Tuple.Add(new NamedExpression { Expression = variable });
                    context.Identifiers.Add(new VariableInstance { Variable = variable });

                    #region Do the compile time stuff

                    if (isCompileTime) {
                        CompileTime.Execute(variable, context);
                    }

                    #endregion
                }

                #region Comma Separator

                if (token.Type == Token.CommaSeparator) {
                    if (!tokens.MoveNext()) return result;
                }
                else {
                    return result; // TODO: report expected comma
                }

                #endregion
            }

            return result;
        }
    }
}
