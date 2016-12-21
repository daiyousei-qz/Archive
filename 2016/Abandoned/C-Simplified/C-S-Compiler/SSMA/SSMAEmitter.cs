using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EdsLab.CSimplified
{
    // it's up to user to ensure proper Emit is invoked
    class SSMAEmitter
    {
        int _labelIndex = 0;
        StringBuilder _builder = new StringBuilder(4096);

        // it's up to user to ensure valid SSMA is generated
        public void Emit(SSMAOpCode code, params string[] operands)
        {
            _builder.Append(code.ToString());

            foreach(var val in operands)
            {
                _builder.Append(' ');
                _builder.Append(val);
            }

            _builder.Append('\r');
            _builder.Append('\n');
        }

        public void InsertComment(string comment)
        {
            _builder.Append(';');
            _builder.Append(comment);
            _builder.Append('\r');
            _builder.Append('\n');
        }

        public string ReserveLabel()
        {
            return "Label_" + (_labelIndex++).ToString();
        }

        public void InsertLabel(string label)
        {
            _builder.AppendLine("LABEL " + label);
        }

        public override string ToString()
        {
            return _builder.ToString();
        }
    }
}
