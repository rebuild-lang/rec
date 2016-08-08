using REC.Scope;

namespace REC.AST
{
    public interface IModule : IBlock
    {
        string Name { get; }

        IScope Scope { get; }
    }

    internal class Module : Block, IModule
    {
        public string Name { get; set; }
        public IScope Scope { get; set; }
    }
}