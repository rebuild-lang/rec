using REC.Tools;

namespace REC.Instance
{
    /* Instances are build from declarations
     * 
     * 
     */
    public interface IInstance : INamed
    {
        bool IsCompileTimeUsable { get; set; }
        bool IsRuntimeUsable { get; set; }
        bool IsCompileTimeOnly { get; set; }
        bool IsRuntimeOnly { get; set; }
    }

    abstract class AbstractInstance : IInstance
    {
        public abstract string Name { get; }

        public bool IsCompileTimeUsable { get; set; } = true;
        public bool IsRuntimeUsable { get; set; } = true;

        public bool IsCompileTimeOnly {
            get { return IsCompileTimeUsable && !IsRuntimeUsable; }
            set {
                if (value) {
                    IsCompileTimeUsable = true;
                    IsRuntimeUsable = false;
                }
                else
                    IsRuntimeUsable = true;
            }
        }

        public bool IsRuntimeOnly {
            get { return !IsCompileTimeUsable && IsRuntimeUsable; }
            set {
                if (value) {
                    IsCompileTimeUsable = false;
                    IsRuntimeUsable = true;
                }
                else
                    IsCompileTimeUsable = true;
            }
        }
    }
}