using System;

namespace REC.Cpp
{
    using GenerateFunc = Func<string>;

    // used for intrinsic function generation
    public interface ICppIntrinsic
    {
        IIndentedTextBuilder Runtime { get; }
        string MakeLocalName(string hint = "temp");
        void EnsureGlobal(string concept, GenerateFunc func);
        string LeftArgument(string name);
        string RightArgument(string name);
        string ResultArgument(string name);
    }

    class CppIntrinsic : ICppIntrinsic
    {
        public ICppScope Scope { get; set; }

        public IIndentedTextBuilder Runtime => Scope.Runtime;

        public string MakeLocalName(string hint = "temp") {
            return Scope.MakeLocalName(hint);
        }

        public void EnsureGlobal(string concept, Func<string> func) {
            Scope.EnsureGlobal(concept, func);
        }

        public string LeftArgument(string name) {
            return name;
        }

        public string RightArgument(string name) {
            return name;
        }

        public string ResultArgument(string name) {
            return name;
        }
    }
}
