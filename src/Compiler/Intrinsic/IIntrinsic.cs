using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using REC.AST;
using REC.Scope;
using REC.Tools;

namespace REC.Intrinsic
{
    using ArgumentDeclarationCollection = NamedCollection<IArgumentDeclaration>;

    public interface IArgumentValues
    {
        ITypedValue this[string label] { get; set; }
        ITypedValue this[int index] { get; set; }
        IReadOnlyDictionary<string, ITypedValue> NamedValues { get; }
        IReadOnlyCollection<ITypedValue> Values { get; }
    }

    public interface IIntrinsic : INamed
    {
        void InvokeCompileTime(IArgumentValues result, IArgumentValues right);

        ArgumentDeclarationCollection RightArguments { get; }
        ArgumentDeclarationCollection Results { get; }
    }

    class ArgumentValues : IArgumentValues
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