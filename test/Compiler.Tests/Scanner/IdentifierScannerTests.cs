using NUnit.Framework;
using REC.Scanner;

namespace REC.Tests.Scanner
{
    [TestFixture()]
    public class IdentifierScannerTests
    {
        internal class Identifier : IIdentifier
        {
            public string Label { get; set; }
        }

        [Test()]
        public void ScanExisting() {
            var map = new IdentifierScanner();
            var id1 = new Identifier { Label = "abc" };
            var id2 = new Identifier { Label = "abc2" };
            map.Add(id1);
            map.Add(id2);

            var input = new TextInputRange {
                File = new TextFile {
                    Content = "abcabc2abcab",
                    Filename = ""
                }
            };
            var scan1 = map.ScanExisting(input);
            var scan2 = map.ScanExisting(input);
            var scan3 = map.ScanExisting(input);
            var scan4 = map.ScanExisting(input);
            Assert.AreEqual(id1, scan1);
            Assert.AreEqual(id2, scan2);
            Assert.AreEqual(id1, scan3);
            Assert.AreEqual(null, scan4);
        }

        [TestCase("normalId ", "normalId")]
        [TestCase("all_lower", "all_lower")]
        [TestCase("+", "+")] // math symbol
        [TestCase("a+2", "a+2")] // combination
        [TestCase("a$2", "a$2")] // currency (dollar not first)
        [TestCase("?!", "?!")] // allowed other punctuations
        [TestCase("(+)", "(+)")] // full brackets
        [TestCase("«+»", "«+»")] // full quotations
        [TestCase("a(", "a")] // partial bracket
        [TestCase("a}", "a")] // partial bracket
        [TestCase("a#", "a")] // comment
        [TestCase("a,b", "a")] // comma is not part of id
        [TestCase("a.b", "a")] // dot is nod part of id
        [TestCase(".add", ".add")] // used for self shortcut
        [TestCase("½²", "½²")] // Other Number
        [TestCase("©®", "©®")] // Other Symbols
        public void ScanNewSuccess(string content, string id)
        {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = content,
                    Filename = ""
                }
            };

            var result = IdentifierScanner.ScanNew(input);

            Assert.IsNotNull(result);
            Assert.AreEqual(id, result.Content);
        }

        [TestCase("123")] // reserved for numbers
        [TestCase(",add")] // comma is separator
        [TestCase("$add")] // $ is pattern
        [TestCase("(+ )")] // space is hard separator, closing bracket is missing, no identifier left
        public void ScanNewFailure(string content) {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = content,
                    Filename = ""
                }
            };

            var result = IdentifierScanner.ScanNew(input);

            Assert.IsNull(result);
        }
    }
}