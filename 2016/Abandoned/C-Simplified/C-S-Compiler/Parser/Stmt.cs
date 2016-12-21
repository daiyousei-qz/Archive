using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EdsLab.CSimplified
{
    // Statement
    abstract class Stmt
    {
        // default implementation
        // no operation
        public virtual void Expand(SSMAEmitter emitter) { }
    }

    class DeclStmt : Stmt
    {
        public string Type { get; private set; }
        public string Id { get; private set; }

        public DeclStmt(string type, string id)
        {
            Type = type;
            Id = id;
        }

        public override void Expand(SSMAEmitter emitter)
        {
            if(Type == "int")
            {
                // add Id to symbal table
                CSAnalyzer._symbolTable.Add(Id, CSAnalyzer._varOffset);
                CSAnalyzer._varOffset += 4;

                // expand into SSMA
                emitter.Emit(SSMAOpCode.alloc, "4");
            }
        }
    }

    class BranchStmt : Stmt
    {
        public Expr Condition { get; private set; }
        public Stmt Yes { get; private set; }
        public Stmt No { get; private set; }

        public BranchStmt(Expr exp, Stmt yes, Stmt no)
        {
            Condition = exp;
            Yes = yes;
            No = no;
        }

        public override void Expand(SSMAEmitter emitter)
        {
            Condition.Expand(emitter);

            if(No == null)
            {
                var endOfBranch = emitter.ReserveLabel();

                // 0 if condition not met
                emitter.Emit(SSMAOpCode.jmpz, endOfBranch);
                Yes.Expand(emitter);
                emitter.InsertLabel(endOfBranch);
            }
            else
            {
                var startOfElse = emitter.ReserveLabel();
                var endOfBranch = emitter.ReserveLabel();

                emitter.Emit(SSMAOpCode.jmpz, startOfElse);
                Yes.Expand(emitter);
                emitter.Emit(SSMAOpCode.jmp, endOfBranch);
                emitter.InsertLabel(startOfElse);
                No.Expand(emitter);
                emitter.InsertLabel(endOfBranch);
            }
        }
    }

    class WhileStmt:Stmt
    {
        public Expr Condition { get; private set; }
        public Stmt Body { get; private set; }

        public WhileStmt(Expr exp, Stmt body)
        {
            Condition = exp;
            Body = body;
        }

        public override void Expand(SSMAEmitter emitter)
        {
            var startOfIteration = emitter.ReserveLabel();
            var endOfIteration = emitter.ReserveLabel();

            emitter.InsertLabel(startOfIteration);
            Condition.Expand(emitter);
            emitter.Emit(SSMAOpCode.jmpz, endOfIteration);
            Body.Expand(emitter);
            emitter.Emit(SSMAOpCode.jmp, startOfIteration);
            emitter.InsertLabel(endOfIteration);
        }
    }

    class ExprStmt : Stmt
    {
        public Expr Expression { get; private set; }

        public ExprStmt(Expr exp)
        {
            Expression = exp;
        }

        public override void Expand(SSMAEmitter emitter)
        {
            Expression.Expand(emitter);
            emitter.Emit(SSMAOpCode.pop);
        }
    }

    class PrintStmt : Stmt
    {
        public Expr Expression { get; private set; }

        public PrintStmt(Expr exp)
        {
            Expression = exp;
        }

        public override void Expand(SSMAEmitter emitter)
        {
            emitter.InsertComment(" print statement is skipped.");
        }
    }

    class SeqStmt : Stmt
    {
        public Stmt A { get; private set; }
        public Stmt B { get; private set; }

        public SeqStmt(Stmt a, Stmt b)
        {
            A = a;
            B = b;
        }

        public override void Expand(SSMAEmitter emitter)
        {
            if (A != null) A.Expand(emitter);
            if (B != null) B.Expand(emitter);

        }
    }
}
