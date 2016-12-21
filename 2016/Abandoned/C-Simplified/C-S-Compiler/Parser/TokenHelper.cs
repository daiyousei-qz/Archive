using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EdsLab.CSimplified
{
    static class TokenHelper
    {
        public static bool Match(this Token token, string symbol)
        {
            if (token == null || token.Symbol != symbol)
                return false;

            return true;
        }

        public static bool DiscardIf(this CSLexer lexer, string symbol)
        {
            var tmp = lexer.Scan();
            if (tmp.Match(symbol))
            {
                return true;
            }
            else
            {
                lexer.PutBack(tmp);
                return false;
            }
        }

        public static void DiscardEnsure(this CSLexer lexer, string symbol)
        {
            if (!lexer.DiscardIf(symbol))
                throw new Exception();
        }
    }
}
