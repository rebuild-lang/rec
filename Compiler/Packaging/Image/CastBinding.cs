using System;

namespace REC.Packaging.Image
{
    class CastBinding<S, T> : IBinding where S : struct where T : struct
    {
        public IValueProvider<S> Source { get; }
        public Func<S?, T?> Func { get; }
        public IBindTarget<T> Target { get; }

        public CastBinding(IValueProvider<S> source, Func<S?, T?> func, IBindTarget<T> target) {
            Source = source;
            Func = func;
            Target = target;
            Source.ValueChanged += HandleChanged;
            Target.SetBinding(this);
            Refresh();
        }

        public void Unbind() {
            Source.ValueChanged -= HandleChanged;
            Target.SetBinding(null);
        }

        public void Refresh() => Target.SetValue(Func(Source.Value));
        private void HandleChanged(S? old, S? @new) => Target.SetValue(Func(@new));
    }

    static class CastBinding
    {
        public static CastBinding<S, T> Create<S, T>(IValueProvider<S> source, Func<S?, T?> func, IBindTarget<T> target) where S : struct where T : struct {
            if (source == null) {
                target.SetBinding(null);
                return null;
            }
            return new CastBinding<S, T>(source, func, target);
        }
    }
}
