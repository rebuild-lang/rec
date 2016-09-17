using REC.AST;
using REC.Tools;

namespace REC.Intrinsic
{
    using ArgumentDeclarationCollection = NamedCollection<IArgumentDeclaration>;

    public class Intrinsic : IIntrinsic
    {
        public string Name { get; protected set; }

        public virtual void InvokeCompileTime(IArgumentValues result, IArgumentValues right) {
            throw new System.NotImplementedException();
        }

        public ArgumentDeclarationCollection RightArguments { get; } = new ArgumentDeclarationCollection();
        public ArgumentDeclarationCollection Results { get; } = new ArgumentDeclarationCollection();
    }

    public class PrintIntrinsic : Intrinsic
    {
        public PrintIntrinsic() {
            Name = "Print";
            RightArguments.Add(new ArgumentDeclaration { Name = "Text" });
        }
        public override void InvokeCompileTime(IArgumentValues result, IArgumentValues right) {
            //var text = right["Text"].Value as string;
            //System.Console.WriteLine(text);
            System.Console.WriteLine("Print was called");
        }
    }
}