using System.Diagnostics.CodeAnalysis;
using REC.AST;
using REC.Parser;
using System.Collections.Generic;
using System.Linq;
using REC.Execution;
using REC.Instance;
using REC.Tools;

#pragma warning disable 649

namespace REC.Intrinsic.API
{
    public interface INestedIdentifier : IExpression
    {
        IList<IIdentifierLiteral> Path { get; }
    }

    static class DeclareModuleExt
    {
        static IModuleInstance GetOrCreateModule(this IContext context, IIdentifierLiteral identifier) {
            var existing = context.Identifiers[identifier.Content];
            if (existing is IModuleInstance existingInstance) return existingInstance;
            var newInstance = new ModuleInstance(identifier.Content);
            if (null != existing) {
                // TODO: report error;
                return newInstance; // return untracked instance
            }
            context.Identifiers.Add(newInstance);
            return newInstance;
        }

        static IModuleInstance GetOrCreateSubmodule(this IModuleInstance module, IIdentifierLiteral identifier) {
            var existing = module.Identifiers.LocalIdentifiers[identifier.Content];
            if (existing is IModuleInstance existingInstance) return existingInstance;
            var newInstance = new ModuleInstance(identifier.Content);
            if (null != existing) {
                // TODO: report error;
                return newInstance; // return untracked instance
            }
            module.Identifiers.Add(newInstance);
            return newInstance;
        }

        internal static IModuleInstance GetOrCreateModule(this IContext context, INestedIdentifier nested) {
            var path = nested.Path;
            if (path.IsEmpty()) return null; // no name, no instance
            var instance = context.GetOrCreateModule(path.First());
            return path.Skip(count: 1).Aggregate(instance, (current, name) => current.GetOrCreateSubmodule(name));
        }
    }


    public class DeclareModule
    {
        public static IFunctionIntrinsic Get() {
            return new FunctionIntrinsic {
                Name = ".DeclareModule",
                RightArgumentsType = typeof(RightArguments),
                ResultType = typeof(ResultArguments),
                CompileTimeApi = (left, right, result, context) => EvalCompileTime((RightArguments) right, (ResultArguments) result, context),
                IsCompileTimeOnly = true
            };
        }

        static void EvalCompileTime(RightArguments right, ResultArguments result, IContext context) {
            var instance = context.GetOrCreateModule(right.NestedName);
            result.Instance = instance;
            if (null == right.Block) {
                result.Declaration = null;
                if (null == instance) {
                    // report error
                }
            }
            else {
                var declaration = new ModuleDeclaration {
                    Range = right.Block.Range,
                    Name = instance?.Name,
                    IsCompileTimeOnly = right.IsCompileTimeOnly
                };
                result.Declaration = declaration;
                if (null == instance) instance = new ModuleInstance(declaration);
                else instance.Declarations.Add(declaration);
                var moduleContext = new Context(instance.Identifiers, new LocalValueScope()) {Parent = context};
                declaration.Block = BlockParser.ParseWithContext(right.Block, moduleContext);
            }
        }

        class RightArguments : IRightArguments
        {
            public INestedIdentifier NestedName;
            public IBlockLiteral Block;
            public bool IsCompileTimeOnly = false;
        }

        [SuppressMessage(category: "ReSharper", checkId: "NotAccessedField.Local")]
        class ResultArguments : IResultArguments
        {
            [ArgumentAssignable] public IModuleInstance Instance;
            [ArgumentAssignable] public IModuleDeclaration Declaration;
        }
    }
}
