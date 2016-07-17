using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace REC
{
    using Dict = Dictionary<char, Entry>;

    public interface IIdentifier
    {
        string Label { get; }
    }
    public interface IEntry
    { }

    public struct ScanResult
    {
        public IIdentifier Identifier;
        public IEntry Next;
    }

    internal static class ExtensionMethods
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

    internal class Entry : IEntry
    {
        private int _depth;
        private readonly Dict _options = new Dict(); // decision options after last streak char
        private IIdentifier _identifier; // Identifier if nothing matches

        public IIdentifier Scan(TextInputRange input) {
            if (_options.Count == 0 || !input.IsEndValid)
                return _identifier;
            Entry next;
            if (!_options.TryGetValue(input.EndChar, out next))
                return _identifier;
            input.Extend();
            var result = next.Scan(input);
            if (result != null) return result;
            input.Extend(-1);
            return _identifier;
        }

        public void Add(IIdentifier identifier) {
            var label = identifier.Label;
            if (label.Length == _depth)
            {
                _identifier = identifier;
                return;
            }
            Debug.Assert(label.Length > _depth);
            var option = label[_depth];
            var next = _options.GetOrAdd(option, () => new Entry { _depth = _depth + 1 });
            next.Add(identifier);
        }
    }

    public class IdentifierScanner
    {
        private readonly Entry _root = new Entry();

        public IIdentifier Scan(TextInputRange input) => _root.Scan(input);
        public void Add(IIdentifier identifier) => _root.Add(identifier);
    }
}