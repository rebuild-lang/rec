using NUnit.Framework;
using REC;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace REC.Tests
{
    [TestFixture()]
    public class NumericalTests
    {
        [TestCase(10, "0", 1, true)]
        [TestCase(10, "255", 1, true)]
        [TestCase(10, "256", 1, false)]
        [TestCase(16, "FF", 1, true)]
        [TestCase(16, "100", 1, false)]
        public void FitsUnsignedTest(int radix, string integerPart, int byteCount, bool fits)
        {
            var num = new Numerical
            {
                BaseRadix = radix, IntegerPart = integerPart
            };

            var result = num.FitsUnsigned(byteCount);

            Assert.AreEqual(fits, result);
        }

        [TestCase(10, "0", 1, true)]
        [TestCase(10, "128", 1, true)]
        [TestCase(10, "129", 1, false)]
        [TestCase(16, "80", 1, true)]
        [TestCase(16, "81", 1, false)]
        public void FitsNegativeTest(int radix, string integerPart, int byteCount, bool fits) {
            var num = new Numerical {
                BaseRadix = radix, IntegerPart = integerPart
            };

            var result = num.FitsNegative(byteCount);

            Assert.AreEqual(fits, result);
        }

        [TestCase(10, "0", 1, true)]
        [TestCase(10, "127", 1, true)]
        [TestCase(10, "128", 1, false)]
        [TestCase(16, "7F", 1, true)]
        [TestCase(16, "80", 1, false)]
        public void FitsPositiveTest(int radix, string integerPart, int byteCount, bool fits) {
            var num = new Numerical {
                BaseRadix = radix, IntegerPart = integerPart
            };

            var result = num.FitsPositive(byteCount);

            Assert.AreEqual(fits, result);
        }
    }
}