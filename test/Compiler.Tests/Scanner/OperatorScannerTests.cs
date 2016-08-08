using NUnit.Framework;
using REC.Scanner;

namespace REC.Tests.Scanner
{
    [TestFixture]
    public class OperatorScannerTests
    {
        [TestCase(arg1: "+", arg2: "+")] // math symbol
        [TestCase(arg1: "+*2", arg2: "+*")] // combination
        [TestCase(arg1: "+$2", arg2: "+")] // currency (dollar not allowed)
        [TestCase(arg1: "?!", arg2: "?!")] // allowed other punctuations
        [TestCase(arg1: "{+}", arg2: "{+}")] // full brackets
        [TestCase(arg1: "«+»", arg2: "«+»")] // full quotations
        [TestCase(arg1: "+(", arg2: "+")] // partial bracket
        [TestCase(arg1: "+}", arg2: "+")] // partial bracket
        [TestCase(arg1: "+#", arg2: "+")] // comment
        [TestCase(arg1: "½²", arg2: "½²")] // Other Number
        [TestCase(arg1: "©®", arg2: "©®")] // Other Symbols
        public void ScanNewSuccess(string content, string id) {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = content,
                    Filename = ""
                }
            };

            var result = OperatorScanner.Scan(input);

            Assert.IsNotNull(result);
            Assert.AreEqual(id, result.Content);
        }

        [TestCase(arg: "normalId ")]
        [TestCase(arg: "all_lower")]
        [TestCase(arg: "123")] // reserved for numbers
        [TestCase(arg: ",+")] // comma is separator
        [TestCase(arg: "$+")] // $ is pattern
        [TestCase(arg: "&+")] // & is compile time execution
        [TestCase(arg: "(+ )")] // space is hard separator, closing bracket is missing, no entry left
        public void ScanNewFailure(string content) {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = content,
                    Filename = ""
                }
            };

            var result = OperatorScanner.Scan(input);

            Assert.IsNull(result);
        }
    }
}