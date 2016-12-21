using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EdsLab.CSimplified
{
    enum TokenType
    {
        Undefined,
        Keyword,
        Identifier,
        Operator,
        Constant,
        Symbol
    }

    struct CharPosition
    {
        public readonly int Line;
        public readonly int Column;

        public CharPosition(int line, int column)
        {
            Line = line;
            Column = column;
        }
    }

    class Token
    {
        public readonly TokenType Type;
        public readonly string Symbol;
        public readonly CharPosition Position;

        public Token(TokenType type, string symbol, CharPosition pos)
        {
            Type = type;
            Symbol = symbol;
            Position = pos;
        }

        public override string ToString()
        {
            return Symbol;
        }
    }

}
