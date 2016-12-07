using NUnit.Framework;
using REC.Scanner;
using System;

namespace REC.Tests.Scanner
{
    [TestFixture]
    public class CommentScannerTests
    {
        [TestCase("#comment")]
        [TestCase("# comment")]
        [TestCase("# comment#")]
        [TestCase("#com ment#comment#")]
        [TestCase("#comment #comment##")]
        public void TestPositiveLineComments(string content)
        {
            var input = new TextInputRange
            {
                File = new TextFile
                {
                    Content = content,
                    Filename = ""
                }
            };

            var output = CommentScanner.Scan(input);
            Assert.IsTrue(output);
            Assert.IsFalse(input.IsEndValid);
        }

        [TestCase("#comment\n comment#")]
        public void TestNegativeLineEndLineComments(string content)
        {
            var input = new TextInputRange
            {
                File = new TextFile
                {
                    Content = content,
                    Filename = ""
                }
            };

            var output = CommentScanner.Scan(input);
            Assert.IsTrue(output);
            Assert.IsTrue(input.IsEndValid);
            Assert.AreEqual('\n', input.EndChar);
        }

        [TestCase("a#jkdajslk")]
        public void TestNegativeLineComments(string content)
        {
            var input = new TextInputRange
            {
                File = new TextFile
                {
                    Content = content,
                    Filename = ""
                }
            };

            var output = CommentScanner.Scan(input);
            Assert.IsFalse(output);
            Assert.AreEqual(content[0], input.EndChar);
        }

        [TestCase("####",1)]
        [TestCase("###A#\n#A##",2)]
        [TestCase("###A#\n#A# ##",2)]
        [TestCase("##\n##",2)]
        [TestCase("##AAA##",1)]
        [TestCase("#A##A#",1)]
        [TestCase("#A#\n#A#",2)]
        [TestCase("#A#\n####\n#A#",3)]
        [TestCase("#A#\n#B##B#\n#A#",3)]
        [TestCase("#A##B##B##A#",1)]
        [TestCase("#A#A#A#",1)]
        public void TestPositiveBlockComments(string content,int line)
        {
            var input = new TextInputRange
            {
                File = new TextFile
                {
                    Content = content,
                    Filename = ""
                }
            };

            var output = CommentScanner.Scan(input);
            Assert.IsTrue(output);
            Assert.IsFalse(input.IsEndValid);
            Assert.AreEqual(line, input.End.Line);
        }

        [TestCase("## Kill")]
        public void TestNegativeBlockComments(string content)
        {
            var input = new TextInputRange
            {
                File = new TextFile
                {
                    Content = content,
                    Filename = ""
                }
            };

            var exception = Assert.Throws<Exception>(() => CommentScanner.Scan(input));
            Assert.True(exception.Message.Equals("Line Comment not Escaped."));
        }
    }
}