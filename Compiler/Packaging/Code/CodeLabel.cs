using System.IO;

namespace REC.Packaging.Code
{
    internal interface ILabel : IInstruction
    {
        string Name { get; }
    }

    internal class CodeLabel : AbstractInstruction, ILabel
    {
        public string Name { get; set; }

        public override InstructionFlags Flags => InstructionFlags.Fixed;

        public override void Write(BinaryWriter binaryWriter) {}

        public CodeLabel() => Size.SetValue(0);
    }
}
