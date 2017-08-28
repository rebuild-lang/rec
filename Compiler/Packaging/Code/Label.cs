using System.IO;

namespace REC.Packaging.Code
{
    internal interface ILabel : IInstruction
    {
        string Name { get; }
    }

    internal class Label : AbstractInstruction, ILabel
    {
        public string Name { get; set; }

        public override InstructionFlags Flags => InstructionFlags.Fixed;

        public override void Write(BinaryWriter binaryWriter) {}

        public Label() => Size.SetValue(0);
    }
}
