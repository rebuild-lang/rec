namespace REC
{
    public class Numerical
    {
        public int BaseRadix; // 1, 8, 10, 16
        public string IntegerPart;
        public string FractionalPart;
        public string ExponentPart;

        public bool FitsUnsigned(int byteCount) => false;
        public bool FitsSigned(int byteCount) => false;
        public bool FitsFloat(int byteCount) => false;

        public byte[] toUnsigned(int byteCount) => new byte[byteCount];
        public byte[] toSigned(int byteCount) => new byte[byteCount];
        public byte[] toFloat(int byteCount) => new byte[byteCount];

        public static Numerical Scan(TextInputRange input)
        {
            var result = new Numerical();


            return result;
        }
    }
}