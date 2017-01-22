using System.Collections.Generic;
using System.Linq;

namespace REC.Instance
{
    public interface IFunctionOverloads : IInstance
    {
        IList<IFunctionInstance> Overloads { get; }
    }

    class FunctionOverloads : AbstractInstance, IFunctionOverloads
    {
        public override string Name => Overloads?.First()?.Name;
        public IList<IFunctionInstance> Overloads { get; } = new List<IFunctionInstance>();
    }
}
