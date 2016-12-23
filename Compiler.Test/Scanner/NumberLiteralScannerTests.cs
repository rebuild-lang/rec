using NUnit.Framework;
using REC.Scanner;

namespace REC.Tests.Scanner
{
    [TestFixture]
    public class NumberLiteralScannerTests
    {
        [TestCase("12'3", 10, "123", null, null)]
        [TestCase("0.12'3", 10, "", "123", null)]
        [TestCase("0e12'3", 10, "", null, "123")]
        [TestCase("0.e0", 10, "", "", "")]
        [TestCase("1.2e-3", 10, "1", "2", "3")]
        [TestCase("0xab'c", 16, "abc", null, null)]
        [TestCase("0x.ab'c", 16, null, "abc", null)]
        [TestCase("0xp12'3", 16, null, null, "123")]
        [TestCase("0xA.Bp-1", 16, "A", "B", "1")]
        [TestCase("0o12'3", 8, "123", null, null)]
        [TestCase("0o.11'55", 8, null, "1155", null)]
        [TestCase("0op12'3", 8, null, null, "123")]
        [TestCase("0o1.3p-4", 8, "1", "3", "4")]
        [TestCase("0b10'1", 2, "101", null, null)]
        [TestCase("0b.11'01", 2, null, "1101", null)]
        [TestCase("0bp10'1", 2, null, null, "101")]
        [TestCase("0b1.1p-10", 2, "1", "1", "10")]
        [TestCase("0", 10, "", null, null)]
        [TestCase("0.", 10, "", "", null)]
        [TestCase("0x0", 16, "", null, null)]
        [TestCase("0x0.", 16, "", "", null)]
        [TestCase("0o0", 8, "", null, null)]
        [TestCase("0o0.", 8, "", "", null)]
        [TestCase("0b0", 2, "", null, null)]
        [TestCase("0b0.", 2, "", "", null)]
        public void ScanSuccess(string content, int radix, string integerPart, string fractionalPart, string exponentPart) {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = content,
                    Filename = ""
                }
            };

            var result = NumberLiteralScanner.Scan(input);

            Assert.IsNotNull(result);
            Assert.AreEqual(radix, result.Radix);
            Assert.AreEqual(integerPart, result.IntegerPart);
            Assert.AreEqual(fractionalPart, result.FractionalPart);
            Assert.AreEqual(exponentPart, result.ExponentPart);
        }

        [TestCase(arg: "0x")]
        [TestCase(arg: "0o")]
        [TestCase(arg: "0b")]
        [TestCase(arg: "0.e")]
        [TestCase(arg: ".0")]
        public void ScanFailures(string content) {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = content,
                    Filename = ""
                }
            };

            var result = NumberLiteralScanner.Scan(input);

            Assert.IsNull(result);
        }
    }
}
