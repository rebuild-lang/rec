using System.Collections.Generic;
using System.Collections.ObjectModel;
using REC.AST;
using REC.Instance;
using REC.Tools;

namespace REC.Execution
{
    using ValueDict = Dictionary<ITypedInstance, ITypedValue>;
    using ReadValueDict = ReadOnlyDictionary<ITypedInstance, ITypedValue>;

    // The LocalValueScope is used for compile time variables and compile time execution
    public interface ILocalValueScope
    {
        ReadValueDict Locals { get; }

        ITypedValue this[ITypedInstance key] { get; }

        void Add(ITypedInstance instance, ITypedValue value);
    }

    class LocalValueScope : ILocalValueScope
    {
        readonly ValueDict _locals = new ValueDict();

        public ReadValueDict Locals => new ReadValueDict(_locals);

        public ITypedValue this[ITypedInstance key] => _locals.Fetch(key, defaultValue: null);

        public void Add(ITypedInstance instance, ITypedValue value) {
            _locals.Add(instance, value);
        }
    }
}
