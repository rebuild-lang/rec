using System.Collections;
using System.Collections.Generic;
using REC.Tools;

namespace REC.Intrinsic
{
    class IntrinsicDict : IIntrinsicDict
    {
        public Dictionary<string, IIntrinsic> Dict { get; } = new Dictionary<string, IIntrinsic>();

        public IIntrinsic this[string key] => Dict.GetOr(key, () => null);

        public IEnumerator<IIntrinsic> GetEnumerator() {
            return Dict.Values.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator() {
            return GetEnumerator();
        }

        public void Add(IIntrinsic value) {
            Dict[value.Name] = value;
        }

        public void Add(IFunctionIntrinsic[] value) {
            foreach (var intrinsic in value) Add(intrinsic);
        }
    }
}