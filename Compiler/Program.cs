using System;
using System.Diagnostics;
using System.IO;
using REC.AST;
using REC.Cpp;
using REC.Intrinsic;
using REC.Intrinsic.IO;
using REC.Intrinsic.Types;
using REC.Intrinsic.Types.API;
using REC.Parser;
using REC.Scanner;

namespace REC
{
    class Compiler
    {
        readonly IContext _injectedContext = new Func<IContext>(
            () => {
                var context = new Context();
                DeclarationConverter.BuildContext(
                    context,
                    new IntrinsicDict {
                        U64Type.Get(), // TODO: move to Rebuild.Intrinsic when we can alias types
                        new ModuleIntrinsic {
                            Name = "Rebuild", // TODO: split to "Intrinsic" and "API"
                            Children = {
                                LiteralType<INumberLiteral>.Get(name: "NumberLiteral"),
                                LiteralType<IBlockLiteral>.Get(name: "BlockLiteral"),
                                PrintIntrinsic.Get(),
                                SimpleMathIntrinsic<ulong, UlongMath>.Get()
                            }
                        }
                    });
                return context;
            })();

        public void CompileFile(TextFile file) {
            var raw = TokenScanner.ScanFile(file);
            var prepared = TokenPreparation.Prepare(raw);
            var block = BlockLineGrouping.Group(prepared);
            var ast = BlockParser.Parse(block, _injectedContext);
#if DEBUG
            var cppFileName = Path.ChangeExtension(file.Filename, extension: "cpp") ?? "test.cpp"; // use this for debugging cpp output
#else
            var cppFileName = $"{Path.GetTempPath()}{Path.GetFileNameWithoutExtension(file.Filename)}{Guid.NewGuid()}.cpp";
#endif
            using (var writer = File.CreateText(cppFileName)) {
                CppGenerator.Generate(writer, ast);
            }
            RunCppCompiler(cppFileName, Path.ChangeExtension(file.Filename, extension: "exe"));
        }

        #region RunCppCompiler

        static void RunCppCompiler(string sourcePath, string finalPath)
        {
            const string shellEnv = @"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\amd64\vcvars64.bat";
            var objPath = Path.ChangeExtension(sourcePath, extension: "obj");

            string Escape(string path)
            {
                return $"\"{path}\"";
            }

            var info = new ProcessStartInfo
            {
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

            var process = new Process { StartInfo = info };
            process.OutputDataReceived += (sender, args) => Console.WriteLine(args.Data);
            process.ErrorDataReceived += (sender, args) => Console.WriteLine(args.Data);
            process.Start();
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
            process.WaitForExit();
        }

        #endregion
    }

    public static class Program
    {
        public static void Main(string[] args) {
            var compiler = new Compiler();
            compiler.CompileFile(
                new TextFile {
                    Content = @"
fn (*l : u64) = (r : u64):
    Rebuild.Assign l, r
end
fn (a : u64) + (b : u64) -> *r : u64:
    r = Rebuild.Add a b
end
fn (*l : u64) += (r : u64):
    l = l + r
end
fn test (x : u64):
    Rebuild.Print x
end
let *x : u64
x = 23 + 12
test x

# let & *y : u64
# &y = 42 + 12
# &Rebuild.Print y

# &test Rebuild.Add 23 32
Rebuild.Print 42 + 22",
                    Filename = "Test.rebuild"
                });
        }
    }
}
