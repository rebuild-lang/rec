using System.Collections.Generic;
using REC.AST;
using REC.Scanner;

namespace REC.Parser
{
    public static class Parser
    {
        public static IExpressionBlock ParseBlock(IBlockLiteral tokenBlock, IContext parentContext) {
            var context = new Context {Parent = parentContext};
            return ParseBlockWithContext(tokenBlock, context);
        }

        public static IExpressionBlock ParseBlockWithContext(IBlockLiteral tokenBlock, IContext context) {
            var block = new ExpressionBlock();
            foreach (var tokenLine in tokenBlock.Lines) {
                using (var it = tokenLine.Tokens.GetEnumerator()) {
                    if (it.MoveNext()) {
                        var done = false;
                        var expression = ParseLineExpression(it, context, ref done);
                        if (expression != null)
                            block.Expressions.Add(expression);
                    }
                }
            }
            return block;
        }

        static IExpression ParseLineExpression(IEnumerator<TokenData> tokens, IContext context, ref bool done) {
            var token = tokens.Current;
            if (token.Type == Token.IdentifierLiteral) {
                var identifier = ((IIdentifierLiteral) token.Data).Content;
                if (identifier == "let") return VariableDeclParser.Parse(tokens, context, ref done);
                if (identifier == "fn") return FunctionDeclParser.Parse(tokens, context, ref done);
                if (identifier == "module") return ModuleDeclParser.Parse(tokens, context, ref done);
                if (identifier == "with") return null; // WithExpressionParser.Parse(tokens, context, ref done);
            }
            return ExpressionParser.Parse(tokens, context, ref done);
        }
    }
}
