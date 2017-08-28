namespace REC.Packaging.Image
{
    interface IValueSink<T> where T : struct
    {
        void SetValue(T? value);
    }
}
