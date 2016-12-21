using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EdsLab.CSimplified
{
    static class CSOperator
    {
        public const string ASSIGN = "=";

        public const string PLUS = "+";
        public const string MINUS = "-";
        public const string MULTIPLY = "*";
        public const string DIVIDE = "/";
        public const string MODULO = "%";
        public const string BITWISE_AND = "&";
        public const string BITWISE_OR = "|";
        public const string BITWISE_XOR = "^";

        public const string GREATER = ">";
        public const string GREATER_EQUAL = ">=";
        public const string LESS = "<";
        public const string LESS_EQUAL = "<=";
        public const string EQUAL = "==";
        public const string UNEQUAL = "!=";

        static ISet<string> _operators = new HashSet<string>
        { ASSIGN, PLUS, MINUS, MULTIPLY, DIVIDE, MODULO, GREATER, GREATER_EQUAL, LESS, LESS_EQUAL, EQUAL, UNEQUAL };

        public static bool IsOperator(string s)
        {
            return _operators.Contains(s);
        }
    }
}
