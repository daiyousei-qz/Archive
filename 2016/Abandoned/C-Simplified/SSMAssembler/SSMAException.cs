using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EdsLab.SSMA
{
    class SSMAException : Exception
    {
        public SSMAException(string message = "Unknown exception captured!") : base(message) { }
    }
}
