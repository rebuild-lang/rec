namespace REC.Intrinsic
{
    public interface IModuleIntrinsic : IIntrinsic
    {
        IIntrinsicDict Children { get; }
        IModuleIntrinsic Parent { get; }
    }
}