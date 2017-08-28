namespace REC.Packaging.Image
{
    interface IAlignAdapter<T> where T : struct
    {
        T? AlignTo(T? value, T alignment);
    }

    class AlignedBinding<T, Aligner> : IBinding
        where T : struct
        where Aligner : struct, IAlignAdapter<T>
    {
        public T Alignment { get; }
        public IValueProvider<T> Source { get; }
        public IBindTarget<T> Target { get; }

        public AlignedBinding(T alignment, IValueProvider<T> source, IBindTarget<T> target) {
            Alignment = alignment;
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

        public void Refresh() => Target.SetValue((new Aligner()).AlignTo(Source.Value, Alignment));
        private void HandleChanged(T? old, T? @new) => Target.SetValue((new Aligner()).AlignTo(@new, Alignment));
    }
}
