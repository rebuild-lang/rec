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
        public void ScanTest() {
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
            var scan1 = map.Scan(input);
            var scan2 = map.Scan(input);
            var scan3 = map.Scan(input);
            var scan4 = map.Scan(input);
            Assert.AreEqual(id1, scan1);
            Assert.AreEqual(id2, scan2);
            Assert.AreEqual(id1, scan3);
            Assert.AreEqual(null, scan4);
        }
    }
}