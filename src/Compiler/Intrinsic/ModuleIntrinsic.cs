using System.Collections;
using System.Collections.Generic;
using REC.Tools;

namespace REC.Intrinsic
{
    using Dict = Dictionary<string, IIntrinsic>;

    public interface IIntrinsicDict : IEnumerable<IIntrinsic>
    {
        Dict Dict { get; }

        IIntrinsic this[string key] { get; }
    }

    class IntrinsicDict : IIntrinsicDict
    {
        public Dict Dict { get; } = new Dict();

        public IIntrinsic this[string key] => Dict.GetOr(key, ()=>null);

        public IEnumerator<IIntrinsic> GetEnumerator() {
            return Dict.Values.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator() {
            return GetEnumerator();
        }

        public void Add(IIntrinsic value) {
            Dict[value.Name] = value;
        }
    }

    public interface IModuleIntrinsic : IIntrinsic
    {
        IIntrinsicDict Children { get; }
        IModuleIntrinsic Parent { get; }
    }

    class ModuleIntrinsic : Intrinsic, IModuleIntrinsic
    {
        public IIntrinsicDict Children { get; } = new IntrinsicDict();
        public IModuleIntrinsic Parent { get; set; }
    }
}
