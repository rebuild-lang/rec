using System.Linq;
using System.Security.Cryptography.X509Certificates;
using REC.Scope;
using REC.Tools;

namespace REC.AST
{
    using ArgumentDeclarationCollection = NamedCollection<IArgumentDeclaration>;

    public interface IFunctionDeclaration : IDeclaration
    {
        ArgumentDeclarationCollection LeftArguments { get; }
        ArgumentDeclarationCollection RightArguments { get; }
        ArgumentDeclarationCollection Results { get; }

        IBindingLevel BindingLevel { get; set; }

        IExpressionBlock Implementation { get; }
    }

    public static class FunctionDeclarationExt
    {
        public static int MandatoryRightArgumentCount(this IFunctionDeclaration declaration) {
            return declaration.RightArguments.Count(arg => 
                !arg.IsUnrolled // TODO: check for fixed array size
                || arg.Value == null);
        }

        public static int? MaxRightArgumentCount(this IFunctionDeclaration declaration) {
            var count = declaration.RightArguments.Count;
            foreach (var argument in declaration.RightArguments) {
                if (argument.IsUnrolled) {
                    return null; // TODO: check for fixed array size
                }
            }
            return count;
        }

        public static bool IsFunction(this IFunctionDeclaration declaration) {
            return declaration.LeftArguments.Count > 0
                && declaration.Results.Count > 0
                && declaration.RightArguments.Count > 0;
        }
    }


    class FunctionDeclaration : Declaration, IFunctionDeclaration
    {
        public ArgumentDeclarationCollection LeftArguments { get; set; }
        public ArgumentDeclarationCollection RightArguments { get; set; }
        public ArgumentDeclarationCollection Results { get; set; }

        public IBindingLevel BindingLevel { get; set; }

        public IExpressionBlock Implementation { get; set; }
    }
}