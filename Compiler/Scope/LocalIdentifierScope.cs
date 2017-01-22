using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using REC.Instance;
using REC.Tools;

namespace REC.Scope
{
    using IdentifierDict = Dictionary<string, IInstance>;
    using ReadIdentifierDict = ReadOnlyDictionary<string, IInstance>;

    // The identifierScope is used for name lookups during parsing
    public interface ILocalIdentifierScope : IEnumerable<IInstance>
    {
        ReadIdentifierDict LocalIdentifiers { get; }

        IInstance this[string key] { get; }

        bool Add(IInstance instance);
        bool Replace(IInstance oldInstance, IInstance newInstance);
    }

    class LocalIdentifierScope : ILocalIdentifierScope
    {
        readonly IdentifierDict _idDict = new IdentifierDict();

        public ReadIdentifierDict LocalIdentifiers => new ReadIdentifierDict(_idDict);

        public IInstance this[string key] => _idDict.Fetch(key, defaultValue: null);

        public bool Add(IInstance instance) {
            var label = instance.Name;
            if (_idDict.ContainsKey(label))
                return false; // already exists
            _idDict.Add(label, instance);
            return true;
        }


        public bool Replace(IInstance oldInstance, IInstance newInstance) {
            if (oldInstance.Name != newInstance.Name) {
                throw new ArgumentException(message: "Replace does not change names");
                //return false;
            }
            var label = oldInstance.Name;
            if (!_idDict.ContainsKey(label)) {
                return false; // does not exist
            }
            _idDict[label] = newInstance;
            return true;
        }

        IEnumerator IEnumerable.GetEnumerator() {
            return _idDict.Values.GetEnumerator();
        }

        public IEnumerator<IInstance> GetEnumerator() {
            return _idDict.Values.GetEnumerator();
        }
    }
}
