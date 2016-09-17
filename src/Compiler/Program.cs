using REC.Intrinsic;
using REC.Scope;

namespace REC
{
    class Compiler
    {
        public Compiler() {
            IntrinsicScope = new IntrinsicScope {
                new PrintIntrinsic()
            }.Build();
        }

        IScope IntrinsicScope { get; }

        public void CompileFile(TextFile file) {
            var raw = Scanner.Scanner.ScanFile(file);
            var prepared = Parser.TokenPreparation.Apply(raw);
            var block = new Parser.BlockLineGrouping().Group(prepared);
            var ast = Parser.Parser.ParseBlock(block, IntrinsicScope);
            // TODO: use AST
        }
    }

    public static class Program
    {
        public static void Main(string[] args) {
            var compiler = new Compiler();
            compiler.CompileFile(
                new TextFile {
                    Content = "&Print \"Hello\"",
                    Filename = "Test.rebuild"
                });
        }
    }
}