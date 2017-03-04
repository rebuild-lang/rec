using System.Collections.Generic;
using REC.AST;
using REC.Instance;
using REC.Scope;

namespace REC.Tests.Parser
{
    static class InstanceHelpers
    {
        internal static IFunctionOverloads ToFunctionInstance(params IFunctionDeclaration[] declarations)
        {
            var result = new FunctionOverloads();
            foreach (var declaration in declarations)
            {
                var instance = new FunctionInstance(declaration);
                ToArgumentInstances(instance.LeftArguments, instance.ArgumentIdentifiers, declaration.LeftArguments, ArgumentSide.Left);
                ToArgumentInstances(instance.RightArguments, instance.ArgumentIdentifiers, declaration.RightArguments, ArgumentSide.Right);
                ToArgumentInstances(instance.Results, instance.ArgumentIdentifiers, declaration.Results, ArgumentSide.Result);
                result.Overloads.Add(instance);
            }
            return result;
        }

        internal static void ToArgumentInstances(
            ICollection<IArgumentInstance> instances,
            ILocalIdentifierScope scope,
            IEnumerable<IArgumentDeclaration> declarations,
            ArgumentSide side)
        {
            foreach (var declaration in declarations)
            {
                var instance = new ArgumentInstance { Argument = declaration, Side = side };
                scope.Add(instance);
                instances.Add(instance);
            }
        }
    }
}
