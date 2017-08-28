using System.Collections.Generic;

namespace REC.Packaging.Image
{
    interface IBindProvider<T> : IBindTarget<T>, IValueProvider<T> where T : struct { }

    struct BindProvider<T> : IBindProvider<T> where T : struct
    {
        private T? _value;
        private IBinding _binding;

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

        public event ValueChanged<T> ValueChanged;
        public event Triggered Triggered;

        public void SetBinding(IBinding binding) {
            _binding = binding;
        }

        public void SetValue(T? value) => Value = value;

        public BindProvider(T? value) {
            _value = value;
            _binding = null;
            ValueChanged = null;
            Triggered = null;
        }
    }
}
