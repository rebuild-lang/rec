namespace REC.Packaging.Code
{
    delegate void SizeChangedHandler(ISizeProvider provider, ulong oldSize);

    internal interface ISizeProvider
    {
        ulong Size { get; }
        event SizeChangedHandler SizeChanged;
    }
}
