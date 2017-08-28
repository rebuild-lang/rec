using System.Linq;

namespace REC.Packaging.Image
{
    interface IAddAdapter<T> where T : struct
    {
        T? Add(T? a, T? b);
    }

    struct SumBinding<T, Adder> : IBinding
        where T : struct
        where Adder : struct, IAddAdapter<T>
    {
        public IValueProvider<T>[] Sources { get; }
        public IBindTarget<T> Target { get; }

        public SumBinding(IValueProvider<T>[] sources, IBindTarget<T> target) {
            Sources = sources;
            Target = target;
            foreach (var source in Sources) source.ValueChanged += HandleChanged;
            Target.SetBinding(this);
            Refresh();
        }

        public void Unbind() {
            foreach (var source in Sources) source.ValueChanged -= HandleChanged;
            Target.SetBinding(null);
        }

        public void Refresh() {
            if (Sources.Length == 0) return;
            var value = Sources.First().Value;
            foreach (var source in Sources.Skip(1)) {
                if (!value.HasValue) break;
                value = (new Adder()).Add(value, source.Value);
            }
            Target.SetValue(value);
        }

        private void HandleChanged(T? old, T? @new) => Refresh();
    }
}
