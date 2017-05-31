using REC.AST;
using REC.Execution;
using REC.Instance;
using REC.Scanner;
using REC.Scope;

namespace REC.Parser
{
    static class PhaseDeclParser
    {
        public static IExpression Parse(ITokenIterator tokens, IContext parentContext)
        {
            if (!tokens.MoveNext()) return null;
            var phaseDecl = new PhaseDeclaration();
            var token = tokens.Current;

            #region Identifier

            if (token.Type != Token.IdentifierLiteral)
            {
                // TODO: error handling name missing
                // handling: mark functionDecl as error and continue to parse
                return null;
            }

            var identifierLiteral = (IIdentifierLiteral)token.Data;
            if (identifierLiteral.SplittedFrom != null) return null; // is a splitted operator
            phaseDecl.Name = identifierLiteral.Content;
            var phaseInst = parentContext.AddPhase(phaseDecl);
            if (!tokens.MoveNext()) return phaseDecl; // fully forward declared
            token = tokens.Current;

            #endregion

            #region Body

            if (token.Type != Token.BlockStartIndentation)
            {
                // TODO: error handling block missing
                // handling: mark phaseDecl as error and continue to parse
                return phaseDecl;
            }
            var identifiers = phaseInst?.Identifiers ?? new LocalIdentifierScope();
            var context = new Context(identifiers, new LocalValueScope()) { Parent = parentContext };
            var contentBlock = (BlockLiteral)token.Data;
            // TODO: replace with specialized parser that allows no nested phases and modules
            phaseDecl.Block = BlockParser.ParseWithContext(contentBlock, context);

            #endregion

            #region Export

            foreach (var instance in identifiers) {
                if (instance is IFunctionInstance funcInstance) {
                    funcInstance.Phase = phaseInst;
                    parentContext.Identifiers.Add(funcInstance);
                }
            }

            #endregion 

            tokens.MoveNext();
            return phaseDecl;
        }
    }

    static class ContextPhaseExt
    {
        internal static IPhaseInstance AddPhase(this IContext context, IPhaseDeclaration phaseDeclaration)
        {
            var existing = context.Identifiers[phaseDeclaration.Name];
            if (existing == null) {
                var newInstance = new PhaseInstance { Declarations = { phaseDeclaration }};
                context.Identifiers.Add(newInstance);
                return newInstance;
            }
            else if (existing is IPhaseInstance instance)
            {
                instance.Declarations.Add(phaseDeclaration);
                return instance;
            }
            else
            {
                // TODO: report error name clash!
                return null;
            }
        }
    }
}
