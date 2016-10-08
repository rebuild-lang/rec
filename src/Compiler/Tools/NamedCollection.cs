using System;
using System.Collections.ObjectModel;

namespace REC.Tools
{
    public interface INamed
    {
        string Name { get; }
    }

    public class NamedCollection<TValue> : KeyedCollection<string, TValue> where TValue : INamed {
        protected override string GetKeyForItem(TValue item) => item.Name;
    }
}
