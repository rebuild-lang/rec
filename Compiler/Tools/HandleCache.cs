using System.Collections.Generic;

namespace REC.Tools
{
    class ObjRef<T>
    {
        public T Obj;
        public int RefCount;
    }

    public class HandleCache<T>
    {
        readonly IDictionary<int, ObjRef<T>> HandleToObj = new Dictionary<int, ObjRef<T>>();
        readonly IDictionary<T, int> ToHandle = new Dictionary<T, int>();
        int handle;

        public int GetHandle(T obj) {
            return ToHandle.GetOrAdd(
                obj,
                () => {
                    var newHandle = handle++;
                    HandleToObj[handle] = new ObjRef<T> {Obj = obj, RefCount = 0};
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
