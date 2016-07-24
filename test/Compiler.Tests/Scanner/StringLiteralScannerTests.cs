using NUnit.Framework;
using REC.Scanner;

namespace REC.Tests.Scanner
{
    [TestFixture()]
    public class StringLiteralScannerTests
    {
        [TestCase("\"\"", "")]
        [TestCase("\" \"", " ")]
        [TestCase("\"text\"", "text")]
        [TestCase("\"line1\nline2\"", "line1line2")]
        [TestCase("\"hello \\\"world\\\"\"", "hello \"world\"")]
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

        [TestCase("\"")]
        [TestCase("\"\\\"")]
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