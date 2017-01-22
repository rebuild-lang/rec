namespace REC.Intrinsic
{
    public enum LiteralConversion
    {
        Ok,
        Failed, // no conversion possible
        TruncatedMajor, // input number > range (1000 => byte)
        TruncatedMinor, // not enough precision (1.5 => int)
        TruncatedRepresentation // decimal float => binary float loss
    }
}