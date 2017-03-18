using REC.AST;
using REC.Execution;
using REC.Instance;
using REC.Scanner;
using REC.Scope;

namespace REC.Parser
{
    static class ModuleDeclParser
    {
        public static IExpression Parse(ITokenIterator tokens, IContext parentContext) {
            if (!tokens.MoveNext()) return null;
            var moduleDecl = new ModuleDeclaration();
            var token = tokens.Current;

            #region Identifier

            var identifiers = (ILocalIdentifierScope) new LocalIdentifierScope();
            if (token.Type == Token.IdentifierLiteral) {
                moduleDecl.Name = ((IIdentifierLiteral) token.Data).Content;
                identifiers = parentContext.AddModule(moduleDecl);
                if (!tokens.MoveNext()) return moduleDecl; // fully forward declared
                token = tokens.Current;
            }

            #endregion

            #region Body

            if (token.Type == Token.BlockStartIndentation) {
                var context = new Context(identifiers, new LocalValueScope()) {Parent = parentContext};
                var contentBlock = (BlockLiteral) token.Data;
                moduleDecl.Block = BlockParser.ParseWithContext(contentBlock, context);
                if (!tokens.MoveNext()) return moduleDecl;
            }

            #endregion

            return moduleDecl;
        }
    }

    static class ContextModuleExt
    {
        internal static ILocalIdentifierScope AddModule(this IContext context, IModuleDeclaration moduleDeclaration) {
            var existing = context.Identifiers[moduleDeclaration.Name];
            if (existing == null) {
                var newInstance = new ModuleInstance(moduleDeclaration);
                context.Identifiers.Add(newInstance);
                return newInstance.Identifiers;
            }
            else if (existing is IModuleInstance instance) {
                instance.Declarations.Add(moduleDeclaration);
                return instance.Identifiers;
            }
            else {
                // TODO: report error name clash!
                return null;
            }
        }
    }
}
