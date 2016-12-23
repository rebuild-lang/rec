using System;
using System.Diagnostics;
using System.IO;
using REC.Cpp;
using REC.Intrinsic;
using REC.Intrinsic.IO;
using REC.Intrinsic.Types;
using REC.Intrinsic.Types.API;
using REC.Parser;

namespace REC
{
    class Compiler
    {
        public Compiler() {
            InjectedScope = new Parser.Scope();
            DeclarationConverter.BuildScope(
                InjectedScope,
                new IntrinsicDict {
                    U64Type.Get(),
                    NumberLiteralType.Get(),
                    PrintIntrinsic.Get(),
                    SimpleMathIntrinsic<ulong, UlongMath>.Get()
                });
        }

        IScope InjectedScope { get; }

        static string GetTempFileName(string basename = "", string extension = "tmp") {
            return Path.GetTempPath() + basename + Guid.NewGuid() + '.' + extension;
        }

        public void CompileFile(TextFile file) {
            var raw = Scanner.Scanner.ScanFile(file);
            var prepared = TokenPreparation.Apply(raw);
            var block = new BlockLineGrouping().Group(prepared);
            var ast = Parser.Parser.ParseBlock(block, InjectedScope);
            //var cppFileName = GetTempFileName(Path.GetFileNameWithoutExtension(file.Filename), extension: "cpp");
            var cppFileName = Path.ChangeExtension(file.Filename, extension: "cpp"); // use this for debugging cpp output
            using (var writer = File.CreateText(cppFileName)) {
                CppGenerator.Generate(writer, ast);
            }
            RunCppCompiler(cppFileName, Path.ChangeExtension(file.Filename, extension: "exe"));
        }

        static string Escape(string path) {
            return $"\"{path}\"";
        }

        void RunCppCompiler(string sourcePath, string finalPath) {
            var shellEnv = @"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\amd64\vcvars64.bat";
            var objPath = Path.ChangeExtension(sourcePath, extension: "obj");

            var info = new ProcessStartInfo {
                FileName = "cmd.exe",
                Arguments = string.Join(
                    separator: " ",
                    value: new[] {
                        "/c",
                        "\"",
                        Escape(shellEnv), "&&",
                        "cl.exe",
                        "/nologo", // clean output
                        "/EHsc", // enable exceptions for STL
                        #region optimized
                        "/O2", // optimize
                        "/MD", // multi threaded DLL
                        "/DNDEBUG", // omit debug code
                        #endregion
                        #region debug

                        //"/MDd", // multi threaded DLL debug
                        //"/Zi", // complete debug infos  
                        #endregion
                        "/TP", // force to C++
                        Escape($"/Fo{objPath}"),
                        Escape(sourcePath),
                        "/link", // here start the linker options
                        Escape($"/PDB:{Path.ChangeExtension(finalPath, extension: "pdb")}"), // separate debug symbols
                        Escape($"/out:{finalPath}"),
                        "\""
                    }),
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                UseShellExecute = false,
                WorkingDirectory = Directory.GetCurrentDirectory()
            };

            var process = new Process {StartInfo = info};
            process.OutputDataReceived += (sender, args) => Console.WriteLine(args.Data);
            process.ErrorDataReceived += (sender, args) => Console.WriteLine(args.Data);
            process.Start();
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
            process.WaitForExit();
        }
    }

    public static class Program
    {
        public static void Main(string[] args) {
            var compiler = new Compiler();
            compiler.CompileFile(
                new TextFile {
                    Content = @"
# assignment is undefined
fn (*l : u64) mov (r : u64):
    Assign l, r
end
fn (a : u64) add (b : u64) -> *r : u64:
    Assign r, Add a b
end
fn test (x : u64):
    Print x
end
#test Add 23 12
&Print 42 add 23
#&test Add 23 32
Print 42 add 22",
                    Filename = "Test.rebuild"
                });
        }
    }
}
