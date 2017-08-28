namespace REC.Packaging.Image
{
    interface IBindTarget<T> : IValueSink<T> where T : struct
    {
        void SetBinding(IBinding binding);
    }
}
