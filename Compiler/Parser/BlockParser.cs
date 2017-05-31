using REC.AST;
using REC.Scanner;

namespace REC.Parser
{
    public static class BlockParser
    {
        public static IExpressionBlock Parse(IBlockLiteral tokenBlock, IContext parentContext) {
            var context = new Context {Parent = parentContext};
            return ParseWithContext(tokenBlock, context);
        }

        public static IExpressionBlock ParseWithContext(IBlockLiteral tokenBlock, IContext context) {
            var block = new ExpressionBlock();
            foreach (var tokenLine in tokenBlock.Lines) {
                var tokens = OperatorLiteralSplitter.SplitAll(tokenLine.Tokens, context);
                using (var it = tokens.GetIterator()) {
                    if (it.Active) {
                        var expression = ParseLineExpression(it, context);
                        if (expression != null)
                            block.Expressions.Add(expression);
                    }
                }
            }
            return block;
        }

        static IExpression ParseLineExpression(ITokenIterator tokens, IContext context) {
            var token = tokens.Current;
            if (token.Type == Token.IdentifierLiteral) {
                var identifier = ((IIdentifierLiteral) token.Data).Content;
                if (identifier == "let") return VariableDeclParser.Parse(tokens, context);
                if (identifier == "fn") return FunctionDeclParser.Parse(tokens, context);
                if (identifier == "module") return ModuleDeclParser.Parse(tokens, context);
                if (identifier == "phase") return PhaseDeclParser.Parse(tokens, context);
                if (identifier == "with") return null; // WithExpressionParser.Parse(tokens, context, ref done);
            }
            return ExpressionParser.ParseTuple(tokens, context);
        }
    }
}
