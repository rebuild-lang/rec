using System.Collections.Generic;

namespace REC.Packaging.Code
{
    internal interface IAddressConsumer
    {
        void ConsumeAddress(ulong address);
    }

    internal interface IAddressProvider
    {
        bool IsAddressValid { get; }
        ulong Address { get; }
        IEnumerable<IAddressConsumer> AddressConsumers { get; }

        void AddAddressConsumer(IAddressConsumer consumer);
        void RemoveAddressConsumer(IAddressConsumer consumer);
    }

    internal abstract class AbstractAddressProvider : IAddressProvider
    {
        private ulong _address = ulong.MaxValue;
        private IList<IAddressConsumer> _addressConsumers = new List<IAddressConsumer>();

        public ulong Address {
            get => _address;
            set {
                if (_address == value) return;
                _address = value;
                foreach (var ac in _addressConsumers) ac.ConsumeAddress(_address);
            }
        }
        public IEnumerable<IAddressConsumer> AddressConsumers => _addressConsumers;

        public bool IsAddressValid => Address != ulong.MaxValue;

        public void AddAddressConsumer(IAddressConsumer consumer) {
            if (_addressConsumers.Contains(consumer)) return;
            _addressConsumers.Add(consumer);
        }

        public void RemoveAddressConsumer(IAddressConsumer consumer) {
            _addressConsumers.Remove(consumer);
        }
    }
}
