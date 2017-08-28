using System.Collections.Generic;

namespace REC.Packaging.Image
{
    delegate void Triggered();
    interface ITriggerProvider
    {
        event Triggered Triggered;
    }

    delegate void ValueChanged<T>(T? old, T? @new) where T : struct;

    interface IValueProvider<T> : ITriggerProvider where T : struct
    {
        T? Value { get; }
        event ValueChanged<T> ValueChanged;
    }

    interface IValueProviderSink<T> : IValueProvider<T>, IValueSink<T> where T : struct { }

    struct ValueProvider<T> : IValueProviderSink<T> where T : struct
    {
        private T? _value;

        public T? Value {
            get => _value;
            set {
                if (EqualityComparer<T?>.Default.Equals(value, _value)) return;
                var old = _value;
                _value = value;
                ValueChanged?.Invoke(old, _value);
                Triggered?.Invoke();
            }
        }

        public event Triggered Triggered;
        public event ValueChanged<T> ValueChanged;

        public void SetValue(T? value) => Value = value;
    }
}
