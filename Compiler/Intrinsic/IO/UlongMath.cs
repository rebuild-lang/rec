namespace REC.Intrinsic.IO
{
    class UlongMath : ISimpleMath<ulong>
    {
        public ulong Add(ulong l, ulong r) {
            return l + r;
        }

        public ulong Sub(ulong l, ulong r) {
            return l - r;
        }
    }
}