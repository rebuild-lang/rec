using NUnit.Framework;

namespace REC.Tests
{
    [TestFixture]
    public class TextInputRangeTests
    {
        [Test]
        public void CollapseWhitespacesTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "AB  C",
                    Filename = ""
                }
            };
            input.Extend(nChars: 2);
            input.CollapseWhitespaces();
            Assert.AreEqual(expected: 'C', actual: input.EndChar);
            Assert.AreEqual(expected: "", actual: input.Text);
        }

        [Test]
        public void EndStringTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "ABC",
                    Filename = ""
                }
            };
            var s = input.EndString(nChars: 4);
            Assert.AreEqual(expected: "ABC", actual: s);
        }

        [Test]
        public void ExtendTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "ABC ",
                    Filename = ""
                }
            };
            input.Extend(nChars: 2);
            Assert.AreEqual(expected: "AB", actual: input.Text);
            Assert.AreEqual(expected: 3, actual: input.End.Column);
        }

        [Test]
        public void IsKeywordTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "ABC ",
                    Filename = ""
                }
            };
            var s = input.IsKeyword(word: "ABC");
            Assert.AreEqual(expected: true, actual: s);
        }

        [Test]
        public void IsNotKeywordTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "ABCD",
                    Filename = ""
                }
            };
            var s = input.IsKeyword(word: "ABC");
            Assert.AreEqual(expected: false, actual: s);
        }

        [Test]
        public void PeekCharEndTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "A",
                    Filename = ""
                }
            };

            Assert.AreEqual(expected: '\0', actual: input.PeekChar());
        }

        [Test]
        public void PeekCharStartTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "AB",
                    Filename = ""
                }
            };

            var t = input.PeekChar();

            Assert.AreEqual(expected: 'B', actual: t);
        }

        [Test]
        public void SkipTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "ABC ",
                    Filename = ""
                }
            };
            input.Skip(nChars: 2);
            Assert.AreEqual(expected: 'C', actual: input.EndChar);
        }
    }
}
