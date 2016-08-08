using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using REC.AST;
using REC.Scope;
using REC.Tools;

namespace REC.Intrinsic
{
    public interface ITypedValue
    {
        IModule Type { get; }
        dynamic Value { get; }
    }

    public interface IArgumentValues
    {
        ITypedValue this[string label] { get; set; }
        ITypedValue this[int index] { get; set; }
        IReadOnlyDictionary<string, ITypedValue> NamedValues { get; }
        IReadOnlyCollection<ITypedValue> Values { get; }
    }

    public interface IIntrinsic : ICallableEntry
    {
        void InvokeCompileTime(IArgumentValues result, IArgumentValues left, IArgumentValues right);
    }

    internal class TypedValue : ITypedValue
    {
        public IModule Type { get; set; }
        public dynamic Value { get; set; }
    }

    internal class ArgumentValues : IArgumentValues
    {
        public class Entry
        {
            public int Index;
            public string Label;
            public ITypedValue Content;
        }

        public IDictionary<string, Entry> Named { get; } = new Dictionary<string, Entry>();
        public IList<Entry> Ordered { get; } = new List<Entry>();

        ITypedValue IArgumentValues.this[string label]
        {
            get { return Named[label].Content; }
            set { Named[label].Content = value; }
        }

        ITypedValue IArgumentValues.this[int index]
        {
            get { return Ordered[index].Content; }
            set { Ordered[index].Content = value; }
        }

        public IReadOnlyDictionary<string, ITypedValue> NamedValues => new ReadOnlyDictionary<string, ITypedValue>(Named.ToDictionary(k => k.Key, v => v.Value.Content));
        public IReadOnlyCollection<ITypedValue> Values => new ReadOnlyCollection<ITypedValue>(Ordered.Select(e => e.Content).ToList());

        public void Append(string label, ITypedValue value) {
            var entry = Named.GetOrAdd(label,
                () => {
                    var newEntry = new Entry { Label = label, Index = Ordered.Count };
                    Ordered.Add(newEntry);
                    return newEntry;
                });
            entry.Content = value;
        }

        public void Append(ITypedValue value) {
            var newEntry = new Entry {Index = Ordered.Count, Content = value};
            Ordered.Add(newEntry);
        }
    }
}