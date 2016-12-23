using NUnit.Framework;
using REC.Scanner;

namespace REC.Tests.Scanner
{
    [TestFixture]
    public class IdentifierScannerTests
    {
        [TestCase(arg1: "normalId ", arg2: "normalId")]
        [TestCase(arg1: "all_lower", arg2: "all_lower")]
        [TestCase(arg1: "a+2", arg2: "a")] // combination
        [TestCase(arg1: "a$2", arg2: "a")] // currency (dollar not first)
        [TestCase(arg1: "a(", arg2: "a")] // partial bracket
        [TestCase(arg1: "a}", arg2: "a")] // partial bracket
        [TestCase(arg1: "a#", arg2: "a")] // comment
        [TestCase(arg1: "a,b", arg2: "a")] // comma is not part of id
        [TestCase(arg1: "a.b", arg2: "a")] // dot is nod part of id
        [TestCase(arg1: ".add", arg2: ".add")] // used for self shortcut

        public void ScanNewSuccess(string content, string id) {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = content,
                    Filename = ""
                }
            };

            var result = IdentifierScanner.Scan(input);

            Assert.IsNotNull(result);
            Assert.AreEqual(id, result.Content);
        }

        [TestCase(arg: "+")] // math symbol
        [TestCase(arg: "(+)")] // full brackets
        [TestCase(arg: "«+»")] // full quotations
        [TestCase(arg: "?!")] // allowed other punctuations
        [TestCase(arg: "½²")] // Other Number
        [TestCase(arg: "©®")] // Other Symbols
        [TestCase(arg: "123")] // reserved for numbers
        [TestCase(arg: ",add")] // comma is separator
        [TestCase(arg: "$add")] // $ is pattern
        [TestCase(arg: "(+ )")] // space is hard separator, closing bracket is missing, no entry left
        public void ScanNewFailure(string content) {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = content,
                    Filename = ""
                }
            };

            var result = IdentifierScanner.Scan(input);

            Assert.IsNull(result);
        }
    }
}