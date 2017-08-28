using System;

namespace REC.Packaging.Image
{
    class Func2Binding<T> : IBinding where T : struct
    {
        public IValueProvider<T> Arg1 { get; }
        public IValueProvider<T> Arg2 { get; }
        public Func<T?, T?, T?> Func { get; }
        public IBindTarget<T> Target { get; }

        public Func2Binding(IValueProvider<T> arg1, IValueProvider<T> arg2, Func<T?, T?, T?> func, IBindTarget<T> target) {
            Arg1 = arg1;
            Arg2 = arg2;
            Func = func;
            Target = target;
            Arg1.ValueChanged += HandleChanged;
            Arg2.ValueChanged += HandleChanged;
            Target.SetBinding(this);
            Refresh();
        }

        public void Unbind() {
            Arg1.ValueChanged -= HandleChanged;
            Arg2.ValueChanged -= HandleChanged;
            Target.SetBinding(null);
        }

        public void Refresh() => Target.SetValue(Func(Arg1.Value, Arg2.Value));
        private void HandleChanged(T? old, T? @new) => Refresh();
    }

    class FuncTriggerBinding<T> : IBinding where T : struct
    {
        public ITriggerProvider[] Triggers { get; }
        public Func<T?> Func { get; }
        public IBindTarget<T> Target { get; }

        public FuncTriggerBinding(ITriggerProvider[] triggers, Func<T?> func, IBindTarget<T> target) {
            Triggers = triggers;
            Func = func;
            Target = target;
            foreach (var trigger in Triggers) trigger.Triggered += Refresh;
            Target.SetBinding(this);
            Refresh();
        }

        public void Unbind() {
            foreach (var trigger in Triggers) trigger.Triggered -= Refresh;
            Target.SetBinding(null);
        }

        public void Refresh() => Target.SetValue(Func());
    }

    internal static class FuncBinding
    {
        public static Func2Binding<T> Create<T>(IValueProvider<T> arg1, IValueProvider<T> arg2, Func<T?, T?, T?> func, IBindTarget<T> target)
            where T : struct {
            return new Func2Binding<T>(arg1, arg2, func, target);
        }

        public static FuncTriggerBinding<T> Create<T>(ITriggerProvider[] triggers, Func<T?> func, IBindTarget<T> target)
            where T : struct {
            return new FuncTriggerBinding<T>(triggers, func, target);
        }
    }
}
