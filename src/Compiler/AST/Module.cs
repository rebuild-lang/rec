namespace REC.AST
{
    public interface IModule : IBlock
    {
        string Name { get; }
    }

    internal class Module : Block, IModule
    {
        public string Name { get; set; }
    }
}