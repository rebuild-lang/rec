using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace REC.Cpp
{
    public interface IIndentedTextBuilder
    {
        string Indentation { get; }
        void AddLine(string line);
        void WithIndent(Action<IIndentedTextBuilder> action);
        void With(string indent, Action<IIndentedTextBuilder> action);
        string Build();
    }

    class IndentedTextBuilder : IIndentedTextBuilder
    {
        bool _blocked;
        public string Indentation { get; set; } = string.Empty;
        IList<string> Lines { get; set; } = new List<string>();

        public void AddLine(string line) {
            Debug.Assert(!_blocked);
            Lines.Add(Indentation + line);
        }

        public void WithIndent(Action<IIndentedTextBuilder> action) {
            With(indent: "  ", action: action);
        }

        public void With(string indent, Action<IIndentedTextBuilder> action) {
            Debug.Assert(!_blocked);
            _blocked = true;
            var sub = new IndentedTextBuilder {
                Indentation = Indentation + indent,
                Lines = Lines
            };
            action(sub);
            _blocked = false;
        }

        public string Build() {
            Debug.Assert(!_blocked);
            return string.Join(separator: "\n", values: Lines);
        }
    }
}