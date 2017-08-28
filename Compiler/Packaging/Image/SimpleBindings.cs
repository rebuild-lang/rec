using REC.Packaging.Tools;

namespace REC.Packaging.Image
{
    struct ULongAdapter : IAddAdapter<ulong>, IAlignAdapter<ulong>
    {
        public ulong? Add(ulong? a, ulong? b) => a + b.GetValueOrDefault(0);
        public ulong? AlignTo(ulong? value, ulong alignment) => value?.AlignTo(alignment);
    }
    struct UIntAdapter : IAddAdapter<uint>, IAlignAdapter<uint>
    {
        public uint? Add(uint? a, uint? b) => a + b.GetValueOrDefault(0);
        public uint? AlignTo(uint? value, uint alignment) => value?.AlignTo(alignment);
    }

    static class SumBinding
    {
        public static SumBinding<ulong, ULongAdapter> Create(IValueProvider<ulong>[] sources, IBindTarget<ulong> target) {
            return new SumBinding<ulong, ULongAdapter>(sources, target);
        }
        public static SumBinding<uint, UIntAdapter> Create(IValueProvider<uint>[] sources, IBindTarget<uint> target) {
            return new SumBinding<uint, UIntAdapter>(sources, target);
        }
    }
    static class AlignedBinding
    {
        public static AlignedBinding<ulong, ULongAdapter> Create(ulong alignment, IValueProvider<ulong> source, IBindTarget<ulong> target) {
            return new AlignedBinding<ulong, ULongAdapter>(alignment, source, target);
        }
        public static AlignedBinding<uint, UIntAdapter> Create(uint alignment, IValueProvider<uint> source, IBindTarget<uint> target) {
            return new AlignedBinding<uint, UIntAdapter>(alignment, source, target);
        }
    }
}
