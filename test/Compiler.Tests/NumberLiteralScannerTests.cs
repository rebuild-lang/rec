using NUnit.Framework;

namespace REC.Tests
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
        [TestCase("0", 10, "", null, null)]
        [TestCase("0.", 10, "", "", null)]
        [TestCase("0x0", 16, "", null, null)]
        [TestCase("0x0.", 16, "", "", null)]
        public void ScanSuccess(string content, int radix, string integerPart, string fractionalPart, string exponentPart)
        {
            var input = new TextInputRange
            {
                File = new TextFile
                {
                    Content = content,
                    Filename = ""
                }
            };

            var result = NumberLiteralScanner.Scan(input);

            Assert.IsNotNull(result);
            Assert.AreEqual(radix, result.BaseRadix);
            Assert.AreEqual(integerPart, result.IntegerPart);
            Assert.AreEqual(fractionalPart, result.FractionalPart);
            Assert.AreEqual(exponentPart, result.ExponentPart);
        }

        [TestCase("0x")]
        [TestCase("0.e")]
        [TestCase(".0")]
        public void ScanFailures(string content)
        {
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
