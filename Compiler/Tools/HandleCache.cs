using System.Collections.Generic;

namespace REC.Tools
{
    class ObjRef<T> where T : class
    {
        public T Obj;
        public int RefCount;
    }

    public class HandleCache<T> where T : class
    {
        readonly IDictionary<int, ObjRef<T>> _handleToObj = new Dictionary<int, ObjRef<T>>();
        readonly IDictionary<T, int> _toHandle = new Dictionary<T, int>();
        int _handle;

        public int GetHandle(T obj) {
            if (null == obj) return 0;
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
            if (0 == handle) return null;
            return _handleToObj[handle].Obj;
        }

        public void AddRef(int handle) {
            if (0 == handle) return;
            _handleToObj[handle].RefCount++;
        }

        public void RemoveRef(int handle) {
            if (0 == handle) return;
            var objref = _handleToObj[handle];
            objref.RefCount--;
            if (objref.RefCount == 0) {
                _toHandle.Remove(objref.Obj);
                _handleToObj.Remove(handle);
            }
        }
    }
}
