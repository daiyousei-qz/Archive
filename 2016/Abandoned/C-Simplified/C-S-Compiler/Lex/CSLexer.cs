using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace EdsLab.CSimplified
{
    class CSLexer
    {
        static ISet<string> _operators = new HashSet<string> { "+", "-", "*", "/", "=", ">", "<", "<=", ">=", "==", "!="};
        static ISet<char> _symbols = new HashSet<char> { '+', '-', '*', '/', '=', '>', '<', '!', '(', ')', '{', '}', ';' };

        StreamReader _reader;
        StringBuilder _builder = new StringBuilder();
        bool _eof = false;
        char _cur = '\0';
        int _line = 1, _column = 0;
        Stack<Token> _bak = new Stack<Token>();

        public bool EOF { get { return _eof; } }

        public CSLexer(StreamReader reader)
        {
            _reader = reader;
            _NextChar();
        }

        void _NextChar()
        {
            if (_reader.EndOfStream) _eof = true;

            while(!_reader.EndOfStream)
            {
                _cur = (char)_reader.Read();

                // update character position counter
                if (_cur == '\r')
                {
                    // '\r' should be omitted
                    continue;
                }
                else if (_cur == '\n')
                {
                    ++_line;
                    _column = 0;
                }
                else
                {
                    ++_column;
                    return;
                }
            }

            // EOS encountered
            _cur = '\0';
        }

        void _AppendWhile(Func<char, bool> cond)
        {
            _NextChar();
            while (cond(_cur) && !_eof)
            {
                _builder.Append(_cur);
                _NextChar();
            }
        }

        public void PutBack(Token tk)
        {
            _bak.Push(tk);
        }

        public Token Scan()
        {
            // return element in if there's a putback
            if (_bak.Count > 0)
                return _bak.Pop();

            // load token from stream
            _builder.Clear();
            while (!_eof)
            {
                var pos = new CharPosition(_line, _column);

                if (char.IsDigit(_cur))
                {
                    // then it's a number constant
                    _builder.Append(_cur);
                    _AppendWhile((ch) => char.IsDigit(ch));

                    return new Token(TokenType.Constant, _builder.ToString(), pos);
                }
                else if(char.IsLetter(_cur) || _cur == '_')
                {
                    // then it's an identifier or a keyword
                    _builder.Append(_cur);
                    _AppendWhile((ch) => char.IsLetterOrDigit(ch) || ch == '_');

                    var name = _builder.ToString();
                    if (CSKeyword.IsKeyword(name))
                        return new Token(TokenType.Keyword, name, pos);
                    else
                        return new Token(TokenType.Identifier, name, pos);
                }
                else if(_symbols.Contains(_cur))
                {
                    // hard coded to check double-sized and single-sized operators
                    _builder.Append(_cur);
                    _NextChar();

                    string s = _builder.ToString();
                    string db = s + _cur;

                    if(CSOperator.IsOperator(db))
                    {
                        _NextChar();
                        return new Token(TokenType.Operator, db, pos);
                    }
                    else if(CSOperator.IsOperator(s))
                    {
                        return new Token(TokenType.Operator, s, pos);
                    }
                    else
                    {
                        return new Token(TokenType.Symbol, s, pos);
                    }
                }
                else if(char.IsWhiteSpace(_cur))
                {
                    // omit white space
                    _NextChar();
                }
                else
                {
                    // any other character should invoke an error
                    throw new Exception();
                }
            }

            // return null when _eof is set
            return null;
        }

        public List<Token> ScanAll()
        {
            var list = new List<Token>();
            for(var token = Scan(); token != null; token = Scan())
            {
                list.Add(token);
            }

            return list;
        }
    }
}
