using System;
using System.Collections.Generic;

namespace REC.Tools
{
    public static class DictionaryExtensionMethods
    {
        public static TValue GetOrAdd<TKey, TValue>(this Dictionary<TKey, TValue> dictionary, TKey key, Func<TValue> valueFactory) {
            if (dictionary == null) throw new ArgumentNullException("dictionary");
            if (key == null) throw new ArgumentNullException("key");
            if (valueFactory == null) throw new ArgumentNullException("valueFactory");

            if (dictionary.ContainsKey(key)) return dictionary[key];
            var value = valueFactory();
            dictionary[key] = value;
            return value;
        }
    }
}
