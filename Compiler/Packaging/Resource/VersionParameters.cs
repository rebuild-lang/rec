using REC.Packaging.Tools;
using REC.Tools;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace REC.Packaging.Resource
{
    public enum VersionKeys
    {
        Comments,
        CompanyName,
        FileDescription,
        FileVersion,
        InternalName,
        LegalCopyright,
        LegalTrademarks,
        OriginalFilename,
        PrivateBuild,
        ProductName,
        ProductVersion,
        SpecialBuild,
    }

    public class VersionParameters
    {
        public string Name;
        public uint Ordinal;
        public Languages Language = Languages.UsEnglish;
        public CodePages CodePage = 0;

        public Fixed FixedData { get; } = new Fixed();
        public Dictionary<LanguageCodePage, Dictionary<VersionKeys, string>> StringTables { get; } = new Dictionary<LanguageCodePage, Dictionary<VersionKeys, string>>();

        private IEnumerable<(LanguageCodePage, IEnumerable<KeyValuePair<VersionKeys, string>>)> ValidStringTables
            => StringTables
                .Select(e => (e.Key, e.Value.Where(x => !string.IsNullOrEmpty(x.Value))))
                .Where(e => e.Item2.Count() != 0);

        public struct LanguageCodePage
        {
            public Languages Language;
            public CodePages CodePage;

            static internal uint WriteSize => 4;
            internal void Write(BinaryWriter bw) {
                bw.Write((ushort)Language);
                bw.Write((ushort)CodePage);
            }
        }

        public class Fixed
        {
            public uint Signature => 0xFEEF04BD;
            public uint StrucVersion = 0x0001_0000;
            public (ushort, ushort, ushort, ushort) FileVersion; // 1.0.5.7
            public (ushort, ushort, ushort, ushort) ProductVersion;
            public FileFlag FileFlagsMask = (FileFlag)Enum.GetValues(typeof(FileFlag)).Cast<int>().Sum(v => v);
            public FileFlag FileFlags;
            public FileOS FileOS = FileOS.NT_WINDOWS32;
            public FileType FileType = FileType.APP;
            public uint FileSubtype => 0; // only used for DRV & FONT
            public ulong FileDate => 0; // not used (encoding unknown)

            public static uint WriteSize => 13 * 4;

            public void Write(BinaryWriter bw) {
                bw.Write(Signature);
                bw.Write(StrucVersion);
                bw.Write((((uint)FileVersion.Item1) << 16) + FileVersion.Item2);
                bw.Write((((uint)FileVersion.Item3) << 16) + FileVersion.Item4);
                bw.Write((((uint)ProductVersion.Item1) << 16) + ProductVersion.Item2);
                bw.Write((((uint)ProductVersion.Item3) << 16) + ProductVersion.Item4);
                bw.Write((uint)FileFlagsMask);
                bw.Write((uint)FileFlags);
                bw.Write((uint)FileOS);
                bw.Write((uint)FileType);
                bw.Write(FileSubtype);
                bw.Write(FileDate);
            }
        }

        uint TranslationsWriteVarSize => (uint)(32 + ValidStringTables.Count() * LanguageCodePage.WriteSize);
        void TranslationsWriteVar(BinaryWriter bw) {
            bw.Write((ushort)TranslationsWriteVarSize);
            bw.Write((ushort)(StringTables.Count * LanguageCodePage.WriteSize));
            bw.Write((ushort)0);
            bw.Write(Encoding.Unicode.GetBytes("Translation\0"));
            bw.PadPosition(4);
            foreach (var t in ValidStringTables) t.Item1.Write(bw);
        }

        uint TranslationsWriteSize => StringTables.IsEmpty() ? 0 : 32 + TranslationsWriteVarSize;
        void TranslationsWrite(BinaryWriter bw) {
            if (StringTables.IsEmpty()) return;
            bw.Write((ushort)TranslationsWriteSize);
            bw.Write((ushort)0);
            bw.Write((ushort)1); // Type
            bw.Write(Encoding.Unicode.GetBytes("VarFileInfo\0"));
            bw.PadPosition(4);
            TranslationsWriteVar(bw);
        }

        uint StringTablesWriteSize => (6u + 15 * 2).AlignTo(4)
                    + (uint)ValidStringTables.Sum(e => StringTableWriteSize(e.Item1, e.Item2));
        void StringTablesWrite(BinaryWriter bw) {
            if (StringTables.IsEmpty()) return;
            bw.Write((ushort)StringTablesWriteSize);
            bw.Write((ushort)0);
            bw.Write((ushort)1); // Type
            bw.Write(Encoding.Unicode.GetBytes("StringFileInfo\0"));
            bw.PadPosition(4);
            foreach (var entry in ValidStringTables)
                StringTableWrite(entry.Item1, entry.Item2, bw);
        }

        private static uint StringTableWriteSize(LanguageCodePage key, IEnumerable<KeyValuePair<VersionKeys, string>> dict) {
            return (6u + (8 + 1) * 2).AlignTo(4)
                + (uint)dict.Sum(e => StringTableEntryWriteSize(e.Key, e.Value));
        }
        private static void StringTableWrite(LanguageCodePage key, IEnumerable<KeyValuePair<VersionKeys, string>> dict, BinaryWriter bw) {
            bw.Write((ushort)StringTableWriteSize(key, dict)); // Size
            bw.Write((ushort)0);
            bw.Write((ushort)1); // Type
            bw.Write(Encoding.Unicode.GetBytes($"{(ushort)key.Language:X4}{(ushort)key.CodePage:X4}\0"));
            bw.PadPosition(4);
            foreach (var entry in dict)
                StringTableEntryWrite(entry.Key, entry.Value, bw);
        }

        private static uint StringTableEntryWriteSize(VersionKeys key, string value) {
            return (6 + (uint)Encoding.Unicode.GetByteCount(key.ToString()) + 2).AlignTo(4)
                + ((uint)Encoding.Unicode.GetByteCount(value) + 2).AlignTo(4);
        }
        private static void StringTableEntryWrite(VersionKeys key, string value, BinaryWriter bw) {
            var keyString = key.ToString();
            var keyBytes = Encoding.Unicode.GetBytes(keyString);
            var valueBytes = Encoding.Unicode.GetBytes(value);
            bw.Write((ushort)StringTableEntryWriteSize(key, value)); // Size
            bw.Write((ushort)(value.Length + 1)); // includes the zero
            bw.Write((ushort)1); // Type
            bw.Write(keyBytes);
            bw.Write((ushort)0);
            bw.PadPosition(4);
            bw.Write(valueBytes);
            bw.Write((ushort)0);
            bw.PadPosition(4);
        }

        public uint WriteSize => (6u + (15 + 1) * 2).AlignTo(4) + Fixed.WriteSize + StringTablesWriteSize + TranslationsWriteSize;
        public void Write(BinaryWriter bw) {
            bw.Write((ushort)(WriteSize));
            bw.Write((ushort)Fixed.WriteSize);
            bw.Write((ushort)0);
            bw.Write(Encoding.Unicode.GetBytes("VS_VERSION_INFO\0"));
            bw.PadPosition(4);
            FixedData.Write(bw);
            bw.PadPosition(4);
            StringTablesWrite(bw);
            TranslationsWrite(bw);
        }
    }


    [Flags]
    public enum FileFlag : uint
    {
        Debug = 1 << 0,
        PreRelease = 1 << 1,
        Patched = 1 << 2,
        PrivateBuild = 1 << 3,
        InfoInferred = 1 << 4,
        SpecialBuild = 1 << 5,
    }

    public enum FileOS : uint
    {
        UNKNOWN = 0,
        DOS = 0x0001_0000,
        OS216 = 0x0002_0000, // OS/2
        OS232 = 0x0003_0000, // OS/2
        NT = 0x0004_0000,

        WINDOWS16 = 0x0000_0001,
        PM16 = 0x0000_0002, // Presentation Manager
        PM32 = 0x0000_0003, // Presentation Manager
        WINDOWS32 = 0x0000_0004,

        // Valid Combos
        DOS_WINDOWS16 = 0x0001_0001,
        DOS_WINDOWS32 = 0x0001_0004,
        NT_WINDOWS32 = 0x0004_0004,
        OS216_PM16 = 0x0002_0002,
        OS232_PM32 = 0x0003_0003,
    }

    public enum FileType
    {
        UNKNOWN,
        APP,
        DLL,
        DRV,
        FONT,
        VXD,
        STATIC_LIB = 7,
    }
}
