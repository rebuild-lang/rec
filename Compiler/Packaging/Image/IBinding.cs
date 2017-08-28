namespace REC.Packaging.Image
{
    interface IBinding
    {
        void Unbind(); // removes all binding notifiers
        void Refresh(); // updates the value without a notifier
    }
}
