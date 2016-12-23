using REC.Tools;

namespace REC.AST
{
    // A declaration creates an entry in a scope
    // Uses are:
    // * Typed (Variables & Arguments)
    // * Functions
    // * Modules
    public interface IDeclaration : IExpression, INamed
    {
        // Name might be null or empty! (allowed for unnamed modules)

        bool IsCompileTimeUsable { get; }
        bool IsRuntimeUsable { get; }
        bool IsCompileTimeOnly { get; }
        bool IsRuntimeOnly { get; }
    }

    abstract class Declaration : Expression, IDeclaration
    {
        public string Name { get; set; }
        public bool IsCompileTimeUsable { get; set; } = true;
        public bool IsRuntimeUsable { get; set; } = true;

        public bool IsCompileTimeOnly {
            get { return IsCompileTimeUsable && !IsRuntimeUsable; }
            set {
                if (value) {
                    IsCompileTimeUsable = true;
                    IsRuntimeUsable = false;
                }
                else IsRuntimeUsable = true;
            }
        }

        public bool IsRuntimeOnly {
            get { return !IsCompileTimeUsable && IsRuntimeUsable; }
            set {
                if (value) {
                    IsCompileTimeUsable = false;
                    IsRuntimeUsable = true;
                }
                else IsCompileTimeUsable = true;
            }
        }
    }

    public interface ITypedDeclaration : IDeclaration
    {
        IModuleDeclaration Type { get; }
    }

    abstract class TypedDeclaration : Declaration, ITypedDeclaration
    {
        public IModuleDeclaration Type { get; set; }
    }
}
