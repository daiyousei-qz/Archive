using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EdsLab.CSimplified
{
    class CSAnalyzer
    {
        public static int _varOffset = 0;
        public static IDictionary<string, int> _symbolTable = new Dictionary<string, int>();

        // note in this version, unary operator is not yet supported
        Expr LoadExpr(CSLexer lexer)
        {
            Token tk = lexer.Scan();
            Expr left = null;
            switch(tk.Type)
            {
                case TokenType.Identifier:
                    left = new IdExpr(tk.Symbol);
                    break;
                case TokenType.Constant:
                    left = new ConstExpr(tk.Symbol);
                    break;
                case TokenType.Symbol:
                    // it must be ( in this case
                    if(tk.Symbol != CSSymbol.LEFT_PARENTHESIS)
                        throw new Exception();

                    lexer.PutBack(tk);
                    left = LoadParenthesedExpr(lexer);
                    break;
                default:
                    throw new Exception();
            }

            // peek to see if expression is ended
            // only operators are allowed to join two expression
            tk = lexer.Scan();
            if(tk.Type == TokenType.Operator)
            {
                Expr right = LoadExpr(lexer);

                return new LBinaryExpr(left, right, tk.Symbol);
            }
            else
            {
                // put back if expression cannot be expanded
                lexer.PutBack(tk);
                return left;
            }
        }

        Expr LoadParenthesedExpr(CSLexer lexer)
        {
            lexer.DiscardEnsure(CSSymbol.LEFT_PARENTHESIS);
            Expr expr = LoadExpr(lexer);
            lexer.DiscardEnsure(CSSymbol.RIGHT_PARENTHESIS);

            return expr;
        }

        Stmt LoadBasicStmt(CSLexer lexer)
        {
            var lex = lexer.Scan();
            if (lex == null) return null;

            if (lex.Type == TokenType.Keyword)
            {
                switch (lex.Symbol)
                {
                    // starts with int
                    // it's a declaration
                    case CSKeyword.INT:
                        // load other components
                        Token id = lexer.Scan();
                        if (id.Type != TokenType.Identifier) throw new Exception();

                        lexer.DiscardEnsure(CSSymbol.SEMICOLON);

                        // construct declaration statement
                        return new DeclStmt(CSKeyword.INT, id.Symbol);

                    // branch statement
                    case CSKeyword.IF:
                        Expr exp_if = LoadParenthesedExpr(lexer);
                        Stmt yes = LoadBracedStmt(lexer);

                        Stmt no = null; // up to next token to determine if else part should be considered
                        if (lexer.DiscardIf(CSKeyword.ELSE))
                        {
                            no = LoadBracedStmt(lexer);
                        }

                        // construct statement
                        return new BranchStmt(exp_if, yes, no);

                    // while iteration statement
                    case CSKeyword.WHILE:
                        Expr exp_w = LoadParenthesedExpr(lexer);
                        Stmt body = LoadBracedStmt(lexer);

                        // construct statement
                        return new WhileStmt(exp_w, body);

                    case CSKeyword.PRINT:
                        Expr exp_p = LoadExpr(lexer);
                        lexer.DiscardEnsure(CSSymbol.SEMICOLON);

                        // construct statement
                        return new PrintStmt(exp_p);
                    default:
                        throw new NotImplementedException();
                }
            }
            else if (lex.Type == TokenType.Identifier)
            {
                // put back loaded token
                lexer.PutBack(lex);
                // load expression as a statement
                var stmt = new ExprStmt(LoadExpr(lexer));
                lexer.DiscardEnsure(CSSymbol.SEMICOLON);

                return stmt;
            }
            else
            {
                // no other cases to start a statement
                throw new Exception();
            }
        }

        // load stmts in {}
        Stmt LoadBracedStmt(CSLexer lexer)
        {
            Stmt cur = null;
            lexer.DiscardEnsure(CSSymbol.LEFT_BRACE);
            while(!lexer.DiscardIf(CSSymbol.RIGHT_BRACE))
            {
                Stmt stmt = LoadBasicStmt(lexer);
                cur = cur == null ? stmt : new SeqStmt(cur, stmt);
            }

            return cur;
        }

        public Stmt Analyze(CSLexer lexer)
        {
            Stmt cur = null;

            while(!lexer.EOF)
            {
                var stmt = LoadBasicStmt(lexer);
                if(stmt != null)
                {
                    cur = cur == null ? stmt : new SeqStmt(cur, stmt);
                }
            }

            return cur;
        }
    }
}
