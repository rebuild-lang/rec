using NUnit.Framework;
using REC.Scanner;

namespace REC.Tests.Scanner
{
    [TestFixture]
    public class StringLiteralScannerTests
    {
        [TestCase(arg1: "\"\"", arg2: "")]
        [TestCase(arg1: "\" \"", arg2: " ")]
        [TestCase(arg1: "\"text\"", arg2: "text")]
        [TestCase(arg1: "\"line1\nline2\"", arg2: "line1line2")]
        [TestCase(arg1: "\"hello \\\"world\\\"\"", arg2: "hello \"world\"")]
        [TestCase(arg1: "\"line1\\nline2\"", arg2: "line1\nline2")]
        public void ScanSuccess(string content, string output) {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = content,
                    Filename = ""
                }
            };

            var result = StringLiteralScanner.Scan(input);
            Assert.IsNotNull(result);
            Assert.AreEqual(output, result.Content);
        }

        [TestCase(arg: "\"")]
        [TestCase(arg: "\"\\\"")]
        public void ScanFailures(string content) {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = content,
                    Filename = ""
                }
            };

            var result = StringLiteralScanner.Scan(input);
            Assert.IsNull(result);
        }
    }
}
