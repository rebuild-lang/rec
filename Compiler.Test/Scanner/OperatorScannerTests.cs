using NUnit.Framework;
using REC.Scanner;

namespace REC.Tests.Scanner
{
    [TestFixture]
    public class OperatorScannerTests
    {
        [TestCase(arg1: "+", arg2: "+", TestName = "math symbol")]
        [TestCase(arg1: "+*2", arg2: "+*", TestName = "combination")]
        [TestCase(arg1: "+$2", arg2: "+", TestName = "currency (dollar not allowed)")]
        [TestCase(arg1: "?!", arg2: "?!", TestName = "allowed other punctuations")]
        [TestCase(arg1: "{+}", arg2: "{+}", TestName = "full brackets")]
        [TestCase(arg1: "«+»", arg2: "«+»", TestName = "full quotations")]
        [TestCase(arg1: "+(", arg2: "+", TestName = "partial open bracket")]
        [TestCase(arg1: "+}", arg2: "+", TestName = "partial close bracket")]
        [TestCase(arg1: "+#", arg2: "+", TestName = "comment")]
        [TestCase(arg1: "½²", arg2: "½²", TestName = "Other Number")]
        [TestCase(arg1: "©®", arg2: "©®", TestName = "Other Symbols")]
        [TestCase(arg1: "-", arg2: "-", TestName = "Dash punctuation")]
        [TestCase(arg1: "&+", arg2: "&+", TestName = "& is a normal operator")]
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

        [TestCase(arg: "normalId ", TestName = "normalId")]
        [TestCase(arg: "all_lower", TestName = "all_lower")]
        [TestCase(arg: "123", TestName = "reserved for numbers")]
        [TestCase(arg: ",+", TestName = "comma is separator")]
        [TestCase(arg: "$+", TestName = "$ is pattern")]
        [TestCase(arg: "(+ )", TestName = "space is hard separator")] // closing bracket is missing, no entry left
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
