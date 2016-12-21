using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EdsLab.CSimplified
{
    static class CSSymbol
    {
        public const string LEFT_PARENTHESIS = "(";
        public const string RIGHT_PARENTHESIS = ")";
        public const string LEFT_BRACE = "{";
        public const string RIGHT_BRACE = "}";
        public const string SEMICOLON = ";";

        static ISet<string> _symbols = new HashSet<string>
        {
            LEFT_PARENTHESIS, RIGHT_PARENTHESIS, LEFT_BRACE, RIGHT_BRACE, SEMICOLON
        };

        public static bool IsSymbol(string s)
        {
            return _symbols.Contains(s);
        }
    }
}
