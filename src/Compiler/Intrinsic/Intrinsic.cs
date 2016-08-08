using System.Collections.Generic;
using REC.AST;
using REC.Scope;

namespace REC.Intrinsic
{
    public class Intrinsic : Entry, IIntrinsic
    {
        public virtual void InvokeCompileTime(IArgumentValues result, IArgumentValues left, IArgumentValues right) {
            throw new System.NotImplementedException();
        }

        public ICollection<IArgumentDeclaration> LeftArguments => null;
        public ICollection<IArgumentDeclaration> RightArguments { get; } = new List<IArgumentDeclaration>();
        public ICollection<IArgumentDeclaration> Results { get; } = new List<IArgumentDeclaration>();
        public ISet<IFunction> Preferred => null;
        public Association Associative => Association.Right;
    }

    public class PrintIntrinsic : Intrinsic
    {
        public PrintIntrinsic() {
            Label = "Print";
            RightArguments.Add(new ArgumentDeclaration {Name = "Text"});
        }
        public override void InvokeCompileTime(IArgumentValues result, IArgumentValues left, IArgumentValues right) {
            var text = right["Text"].Value as string;
            System.Console.WriteLine(text);
        }
    }
}