using System;
using NUnit.Framework;
using REC.Scanner;

namespace REC.Tests.Scanner
{
    [TestFixture]
    public class CommentScannerTests
    {
        [TestCase(arg: "#comment")]
        [TestCase(arg: "# comment")]
        [TestCase(arg: "# comment#")]
        [TestCase(arg: "#com ment#comment#")]
        [TestCase(arg: "#comment #comment##")]
        public void TestPositiveLineComments(string content) {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = content,
                    Filename = ""
                }
            };

            var output = CommentScanner.Scan(input);
            Assert.IsTrue(output);
            Assert.IsFalse(input.IsEndValid);
        }

        [TestCase(arg: "#comment\n comment#")]
        public void TestNegativeLineEndLineComments(string content) {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = content,
                    Filename = ""
                }
            };

            var output = CommentScanner.Scan(input);
            Assert.IsTrue(output);
            Assert.IsTrue(input.IsEndValid);
            Assert.AreEqual(expected: '\n', actual: input.EndChar);
        }

        [TestCase(arg: "a#jkdajslk")]
        public void TestNegativeLineComments(string content) {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = content,
                    Filename = ""
                }
            };

            var output = CommentScanner.Scan(input);
            Assert.IsFalse(output);
            Assert.AreEqual(content[index: 0], input.EndChar);
        }

        [TestCase(arg1: "####", arg2: 1)]
        [TestCase(arg1: "###A#\n#A##", arg2: 2)]
        [TestCase(arg1: "###A#\n#A# ##", arg2: 2)]
        [TestCase(arg1: "##\n##", arg2: 2)]
        [TestCase(arg1: "##AAA##", arg2: 1)]
        [TestCase(arg1: "#A##A#", arg2: 1)]
        [TestCase(arg1: "#A#\n#A#", arg2: 2)]
        [TestCase(arg1: "#A#\n####\n#A#", arg2: 3)]
        [TestCase(arg1: "#A#\n#B##B#\n#A#", arg2: 3)]
        [TestCase(arg1: "#A##B##B##A#", arg2: 1)]
        [TestCase(arg1: "#A#A#A#", arg2: 1)]
        public void TestPositiveBlockComments(string content, int line) {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = content,
                    Filename = ""
                }
            };

            var output = CommentScanner.Scan(input);
            Assert.IsTrue(output);
            Assert.IsFalse(input.IsEndValid);
            Assert.AreEqual(line, input.End.Line);
        }

        [TestCase(arg: "## Kill")]
        public void TestNegativeBlockComments(string content) {
            var input = new TextInputRange {
                File = new TextFile {
                    Content = content,
                    Filename = ""
                }
            };

            var exception = Assert.Throws<Exception>(() => CommentScanner.Scan(input));
            Assert.True(exception.Message.Equals(value: "Line Comment not Escaped."));
        }
    }
}
