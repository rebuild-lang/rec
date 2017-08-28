namespace REC.Packaging.Image
{
    class ValueBinding<T> : IBinding where T : struct
    {
        public IValueProvider<T> Source { get; }
        public IBindTarget<T> Target { get; }

        public ValueBinding(IValueProvider<T> source, IBindTarget<T> target) {
            Source = source;
            Target = target;
            Source.ValueChanged += HandleChanged;
            Target.SetBinding(this);
            Refresh();
        }

        public void Unbind() {
            Source.ValueChanged -= HandleChanged;
            Target.SetBinding(null);
        }

        public void Refresh() => Target.SetValue(Source.Value);
        private void HandleChanged(T? old, T? @new) => Target.SetValue(@new);
    }

    static class ValueBinding
    {
        public static ValueBinding<T> Create<T>(IValueProvider<T> source, IBindTarget<T> target) where T : struct {
            if (source == null) {
                target.SetBinding(null);
                return null;
            }
            return new ValueBinding<T>(source, target);
        }
    }
}
