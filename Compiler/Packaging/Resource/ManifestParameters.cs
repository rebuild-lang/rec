using System.IO;

namespace REC.Packaging.Resource
{
    public class ManifestParameters
    {
        public string Name;
        public uint Ordinal;
        public Languages Language = Languages.UsEnglish;
        public CodePages CodePage = 0;
        //public string Text;
        public Stream Stream;
    }
}
