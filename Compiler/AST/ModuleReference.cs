using REC.Instance;

namespace REC.AST
{
    public interface IModuleReference : IExpression
    {
        IModuleInstance Reference { get; }
        
    }

    class ModuleReference : Expression, IModuleReference
    {
        public IModuleInstance Reference { get; set; }
    }
}