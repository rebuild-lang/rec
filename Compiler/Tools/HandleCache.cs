using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace REC.Tools
{

    class ObjRef<T>
    {
        public T Obj;
        public int RefCount;
    }

    public class HandleCache<T>
    {
        readonly IDictionary<T, int> ToHandle = new Dictionary<T, int>();
        readonly IDictionary<int, ObjRef<T>> HandleToObj = new Dictionary<int, ObjRef<T>>();
        int handle = 0;

        public int GetHandle(T obj) {
            return ToHandle.GetOrAdd(
                obj,
                () => {
                    var newHandle = handle++;
                    HandleToObj[handle] = new ObjRef<T> { Obj = obj, RefCount = 0};
                    return handle;
                });
        }

        public T GetValue(int handle) {
            return HandleToObj[handle].Obj;
        }

        public void AddRef(int handle) {
            HandleToObj[handle].RefCount++;
        }

        public void RemoveRef(int handle) {
            var objref = HandleToObj[handle];
            objref.RefCount--;
            if (objref.RefCount == 0) {
                ToHandle.Remove(objref.Obj);
                HandleToObj.Remove(handle);
            }
        }
    }
}
