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
            Assert.That(input.EndChar, Is.EqualTo(expected: 'C'));
            Assert.That(input.Text, Is.Empty);
        }

        [Test]
        public void EndStringTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "ABC",
                    Filename = ""
                }
            };
            Assert.That(input.EndString(nChars: 4), Is.EqualTo(expected: "ABC"));
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
            Assert.That(input.Text, Is.EqualTo(expected: "AB"));
            Assert.That(input.End.Column, Is.EqualTo(expected: 3));
        }

        [Test]
        public void IsKeywordTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "ABC ",
                    Filename = ""
                }
            };
            Assert.That(input.IsKeyword(word: "ABC"), Is.True);
        }

        [Test]
        public void IsNotKeywordTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "ABCD",
                    Filename = ""
                }
            };
            Assert.That(input.IsKeyword(word: "ABC"), Is.False);
        }

        [Test]
        public void PeekCharEndTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "A",
                    Filename = ""
                }
            };
            Assert.That(input.PeekChar(), Is.EqualTo(expected: '\0'));
        }

        [Test]
        public void PeekCharStartTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "AB",
                    Filename = ""
                }
            };
            Assert.That(input.PeekChar(), Is.EqualTo(expected: 'B'));
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
            Assert.That(input.EndChar, Is.EqualTo(expected: 'C'));
        }
    }
}
