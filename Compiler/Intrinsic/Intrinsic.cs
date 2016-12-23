using REC.Tools;

namespace REC.Intrinsic
{
    public interface IIntrinsic : INamed
    {}

    abstract class Intrinsic : IIntrinsic
    {
        public string Name { get; set; }
    }
}
