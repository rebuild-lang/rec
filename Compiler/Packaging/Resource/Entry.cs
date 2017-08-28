using System.IO;

namespace REC.Packaging.Resource
{
    public enum Types
    {
        CURSOR = 1, GROUP_CURSOR = CURSOR + 11,
        BITMAP = 2,
        ICON = 3, GROUP_ICON = ICON + 11,
        MENU = 4,
        DIALOG = 5,
        STRING = 6, // String-table entry.
        FONTDIR = 7,
        FONT = 8,
        ACCELERATOR = 9,
        RCDATA = 10, // Raw Data
        MESSAGETABLE = 11,
        VERSION = 16,
        DLGINCLUDE = 17,
        PLUGPLAY = 19,
        VXD = 20,
        ANICURSOR = 21, // Animated cursor.
        ANIICON = 22, // Animated icon.
        HTML = 23,
        MANIFEST = 24,
        FirstUserType = 256,
    }

    public interface IEntryData
    {
        uint WriteSize { get; }
        void Write(BinaryWriter bw);
    }

    public struct Entry
    {
        public Types Type;
        public string Name;
        public uint Ordinal; // used when Name is null or empty
        public Languages Language;
        public CodePages CodePage;
        public IEntryData Data;
    }

    public class PlainData : IEntryData
    {
        public byte[] Data;
        public uint WriteSize => (uint)Data.Length;

        public void Write(BinaryWriter bw) {
            bw.Write(Data);
        }
    }
}
