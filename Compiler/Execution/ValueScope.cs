using System.Collections.Generic;
using System.Collections.ObjectModel;
using REC.AST;
using REC.Tools;

namespace REC.Execution
{
    using ValueDict = Dictionary<ITypedDeclaration, ITypedValue>;
    using NamedDict = Dictionary<string, ITypedValue>;
    using ReadValueDict = ReadOnlyDictionary<ITypedDeclaration, ITypedValue>;

    // The identifierScope is used for declaration lookups during compile time execution
    public interface IValueScope
    {
        IValueScope Parent { get; }

        ReadValueDict Locals { get; }

        ITypedValue this[ITypedDeclaration key] { get; }

        ITypedValue this[string key] { get; }

        void Add(ITypedDeclaration declaration, ITypedValue value);
    }

    class ValueScope : IValueScope
    {
        readonly ValueDict _locals = new ValueDict();
        readonly NamedDict _names = new NamedDict();

        public IValueScope Parent { get; set; }
        public ReadValueDict Locals => new ReadValueDict(_locals);

        public ITypedValue this[ITypedDeclaration key] => _locals.GetOr(key, () => Parent?[key]);
        public ITypedValue this[string key] => _names.GetOr(key, () => Parent?[key]);

        public void Add(ITypedDeclaration declaration, ITypedValue value) {
            _locals.Add(declaration, value);
            _names.Add(declaration.Name, value);
        }
    }
}