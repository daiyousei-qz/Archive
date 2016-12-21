using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EdsLab.CSimplified
{
    static class CSKeyword
    {
        public const string INT = "int";
        public const string WHILE = "while";
        public const string IF = "if";
        public const string ELSE = "else";
        public const string PRINT = "print";

        static ISet<string> _keywords = new HashSet<string>
        {
            INT, WHILE, IF, ELSE, PRINT
        };

        public static bool IsKeyword(string s)
        {
            return _keywords.Contains(s);
        }
    }
}
