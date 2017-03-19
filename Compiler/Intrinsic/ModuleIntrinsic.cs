namespace REC.Intrinsic
{
    class ModuleIntrinsic : Intrinsic, IModuleIntrinsic
    {
        public IIntrinsicDict Children { get; } = new IntrinsicDict();
        public IModuleIntrinsic Parent { get; set; }
    }
}
