using System.IO;
using REC.Cpp;
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
            var prepared = TokenPreparation.Apply(raw);
            var block = new BlockLineGrouping().Group(prepared);
            var ast = Parser.Parser.ParseBlock(block, InjectedScope);
            using (var writer = File.CreateText(path: "compiled.cpp")) {
                var generator = new CppGenerator();
                generator.Generate(writer, ast);
            }
        }
    }

    public static class Program
    {
        public static void Main(string[] args) {
            var compiler = new Compiler();
            compiler.CompileFile(
                new TextFile {
                    Content = "Print 42",
                    Filename = "Test.rebuild"
                });
        }
    }
}
