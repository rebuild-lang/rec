using System.Collections.Generic;
using System.Linq;
using REC.AST;
using REC.Execution;
using REC.Scanner;
using REC.Scope;
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
                        var expression = ParseLineExpression(it, scope, ref done);
                        if (expression != null) block.Expressions.Add(expression);
                    }
                }
            }
            return block;
        }

        static IExpression ParseLineExpression(IEnumerator<TokenData> tokens, IScope scope, ref bool done) {
            var token = tokens.Current;
            if (token.Type == Token.IdentifierLiteral) {
                var identifier = (IIdentifierLiteral) token.Data;
                if (identifier.Content == "let") return null; // ParseVariableDecl(tokens, scope, ref done);
                if (identifier.Content == "fn") return FunctionDeclParser.Parse(tokens, scope, ref done);
                if (identifier.Content == "module") return null; // ParseModuleDecl(tokens, scope, ref done);
                if (identifier.Content == "with") return null; // ParseWithExpression(tokens, scope, ref done);
            }
            return ExpressionParser.Parse(tokens, scope, ref done);
        }
    }
}
