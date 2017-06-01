using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using REC.AST;
using REC.Cpp;
using REC.Intrinsic;
using REC.Intrinsic.API;
using REC.Intrinsic.IO;
using REC.Intrinsic.Types;
using REC.Intrinsic.Types.API;
using REC.Parser;
using REC.Scanner;
using REC.Tools;

namespace REC
{
    public interface ICompiler
    {
        // used through the API to add static text
        void AddCode(string code, string fileName = null, TextPosition position = null);

        //void AddFromFile(string fileName);

        void AddTextFile(TextFile file);

        IEnumerable<IExpressionBlock> BuildAst();

        void CompileToExecutable(string fileName = null);
    }

    class Compiler : ICompiler
    {
        readonly IContext _injectedContext = new Func<IContext>(
            () => {
                var context = new Context();
                DeclarationConverter.BuildContext(
                    context,
                    new IntrinsicDict {
                        U64Type.Get(), // TODO: move to Rebuild.Intrinsic when we can alias types
                        CompilerType.Get(),
                        new ModuleIntrinsic {
                            Name = "Rebuild", // TODO: split to "Intrinsic" and "API"
                            Children = {
                                LiteralType<INumberLiteral>.Get(name: "NumberLiteral"),
                                LiteralType<IBlockLiteral>.Get(name: "BlockLiteral"),
                                LiteralType<IStringLiteral>.Get(name: "StringLiteral"),
                                LiteralType<IExpression>.Get(name: "Expression"),
                                PrintIntrinsic.Get(),
                                EvalExpression.Get(),
                                SimpleMathIntrinsic<ulong, UlongMath>.Get(),
                                CompilerInstance.Get(),
                                AddCodeToCompiler.Get()
                            }
                        }
                    });
                return context;
            })();

        IList<TextFile> _files = new List<TextFile>();

        public void AddCode(string code, string fileName = null, TextPosition position = null) {
            Console.WriteLine("AddCode: " + code);
            _files.Add(new TextFile{Content = code, Filename = fileName ?? "__Added__"});
        }

        public void AddTextFile(TextFile file) {
            _files.Add(file);
        }

        public IEnumerable<IExpressionBlock> BuildAst() {
            return _files.Select(
                file => {
                    var raw = TokenScanner.ScanFile(file);
                    var prepared = TokenPreparation.Prepare(raw);
                    var block = BlockLineGrouping.Group(prepared);
                    return BlockParser.Parse(block, _injectedContext);
                });
        }

        public void CompileToExecutable(string fileName = null) {
            if (_files.IsEmpty()) return;
            if (fileName == null) fileName = Path.ChangeExtension(_files.First().Filename, extension: "exe") ?? "test.exe";
#if DEBUG
            var cppFileName = Path.ChangeExtension(fileName, extension: "cpp"); // use this for debugging cpp output
#else
            var cppFileName = $"{Path.GetTempPath()}{Path.GetFileNameWithoutExtension(fileName)}{Guid.NewGuid()}.cpp";
#endif
            using (var writer = File.CreateText(cppFileName))
            {
                foreach (var ast in BuildAst()) CppGenerator.Generate(writer, ast);
            }
            RunCppCompiler(cppFileName, fileName);
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
}