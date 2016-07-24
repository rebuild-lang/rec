using System.Collections.Generic;
using REC.Scanner;

namespace REC
{
    using Dict = Dictionary<string, IIdentifier>;

    public class IdentifierScope
    {
        private readonly Dict _identifiers = new Dict();
        private readonly IdentifierScope _parent;

        public IIdentifier this[string key] => _identifiers[key] ?? _parent?[key];

        public IdentifierScope(IdentifierScope parent = null) {
            _parent = parent;
        }

        public bool Add(IIdentifier identifier) {
            var label = identifier.Label;
            if (_identifiers.ContainsKey(label))
                return false;
            _identifiers.Add(label, identifier);
            return true;
        }
    }
}
