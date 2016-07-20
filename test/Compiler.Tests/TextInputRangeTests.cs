using NUnit.Framework;
using System;

namespace REC.Tests
{
    [TestFixture()]
    public class TextInputRangeTests
    {
        [Test()]
        public void PeekCharStartTest()
        {
            var input = new TextInputRange
            {
                File = new TextFile
                {
                    Content = "AB",
                    Filename = ""
                }
            };

            var t = input.PeekChar();

            Assert.AreEqual('B', t);
        }

        [Test()]
        public void PeekCharEndTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "A",
                    Filename = ""
                }
            };

            Assert.AreEqual('\0', input.PeekChar());
        }

        [Test()]
        public void EndStringTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "ABC",
                    Filename = ""
                }
            };
            var s = input.EndString(4);
            Assert.AreEqual("ABC", s);
        }

        [Test()]
        public void IsKeywordTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "ABC ",
                    Filename = ""
                }
            };
            var s = input.IsKeyword("ABC");
            Assert.AreEqual(true, s);
        }

        [Test()]
        public void IsNotKeywordTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "ABCD",
                    Filename = ""
                }
            };
            var s = input.IsKeyword("ABC");
            Assert.AreEqual(false, s);
        }

        [Test()]
        public void SkipTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "ABC ",
                    Filename = ""
                }
            };
            input.Skip(2);
            Assert.AreEqual('C', input.EndChar);
        }

        [Test()]
        public void ExtendTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "ABC ",
                    Filename = ""
                }
            };
            input.Extend(2);
            Assert.AreEqual("AB", input.Text);
            Assert.AreEqual(3, input.End.Column);
        }

        [Test()]
        public void CollapseWhitespacesTest() {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = "AB  C",
                    Filename = ""
                }
            };
            input.Extend(2);
            input.CollapseWhitespaces();
            Assert.AreEqual('C', input.EndChar);
            Assert.AreEqual("", input.Text);
        }
    }
}