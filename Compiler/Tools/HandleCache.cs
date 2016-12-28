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
        readonly IDictionary<int, ObjRef<T>> _handleToObj = new Dictionary<int, ObjRef<T>>();
        readonly IDictionary<T, int> _toHandle = new Dictionary<T, int>();
        int _handle;

        public int GetHandle(T obj) {
            return _toHandle.GetOrAdd(
                obj,
                () => {
                    _handle++;
                    var objHandle = _handle;
                    _handleToObj[objHandle] = new ObjRef<T> {Obj = obj, RefCount = 0};
                    return objHandle;
                });
        }

        public T GetValue(int handle) {
            return _handleToObj[handle].Obj;
        }

        public void AddRef(int handle) {
            _handleToObj[handle].RefCount++;
        }

        public void RemoveRef(int handle) {
            var objref = _handleToObj[handle];
            objref.RefCount--;
            if (objref.RefCount == 0) {
                _toHandle.Remove(objref.Obj);
                _handleToObj.Remove(handle);
            }
        }
    }
}
