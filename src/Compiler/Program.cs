using REC.Intrinsic;
using REC.Intrinsic.IO;
using REC.Parser;

namespace REC
{
    class Compiler
    {
        public Compiler() {
            InjectedScope = new Parser.Scope();
            DeclarationConverter.BuildScope(InjectedScope,
                new IntrinsicDict {
                        Intrinsic.Types.U64Type.Get(),
                        Intrinsic.Types.API.NumberLiteralType.Get(),
                        PrintIntrinsic.Get(),
                        SimpleMathIntrinsic<ulong, UlongMath>.Get()
                });
        }

        IScope InjectedScope { get; }

        public void CompileFile(TextFile file) {
            var raw = Scanner.Scanner.ScanFile(file);
            var prepared = Parser.TokenPreparation.Apply(raw);
            var block = new Parser.BlockLineGrouping().Group(prepared);
            var ast = Parser.Parser.ParseBlock(block, InjectedScope);
            // TODO: use AST
        }
    }

    public static class Program
    {
        public static void Main(string[] args) {
            var compiler = new Compiler();
            compiler.CompileFile(
                new TextFile {
                    Content = "&Print Add 23 42",
                    Filename = "Test.rebuild"
                });
        }
    }
}
