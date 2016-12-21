using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EdsLab.CSimplified
{
    // an expanded Expr should always push an element onto Evaluation Stack
    abstract class Expr
    {
        // by default, it's unassignable
        public virtual bool Assignable { get { return false; } }

        public virtual void Expand(SSMAEmitter emitter)
        {
            // no operation
        }
    }

    class IdExpr : Expr
    {
        public override bool Assignable { get { return true; } }

        public string Id { get; private set; }

        public IdExpr(string id)
        {
            Id = id;
        }

        // note this function expand the Identifier as a value
        public override void Expand(SSMAEmitter emitter)
        {
            SSMAHelper.LoadVarAddress(emitter, Id);
            emitter.Emit(SSMAOpCode.load_4);
        }
    }

    class ConstExpr : Expr
    {
        public string Value { get; private set; }
        
        public ConstExpr(string val)
        {
            Value = val;
        }

        public override void Expand(SSMAEmitter emitter)
        {
            // assert Value is in an integer format
            uint val = Convert.ToUInt32(Value);
            SSMAHelper.PushUInt(emitter, val);
        }
    }

    class LBinaryExpr :Expr
    {
        public Expr Left { get; private set; }
        public Expr Right { get; private set; }
        public string Operator { get; private set; }

        public LBinaryExpr(Expr left, Expr right, string op)
        {
            Left = left;
            Right = right;
            Operator = op;
        }

        public override void Expand(SSMAEmitter emitter)
        {
            //emitter.InsertComment("A LBinaryExpr is referred here =>" + Operator);

            if(Operator == CSOperator.ASSIGN)
            {
                if (!Left.Assignable) throw new Exception();
                
                // do assignment
                var target = Left as IdExpr;
                Right.Expand(emitter);
                emitter.Emit(SSMAOpCode.dup); // dup for a value to return
                SSMAHelper.LoadVarAddress(emitter, target.Id);
                emitter.Emit(SSMAOpCode.store_4);
            }
            else
            {
                // load two operands
                Left.Expand(emitter);
                Right.Expand(emitter);

                switch(Operator)
                {
                    case CSOperator.PLUS:
                        emitter.Emit(SSMAOpCode.add);
                        break;
                    case CSOperator.MINUS:
                        emitter.Emit(SSMAOpCode.sub);
                        break;
                    case CSOperator.MULTIPLY:
                        emitter.Emit(SSMAOpCode.mul);
                        break;
                    case CSOperator.DIVIDE:
                        emitter.Emit(SSMAOpCode.div);
                        break;
                    case CSOperator.MODULO:
                        emitter.Emit(SSMAOpCode.mod);
                        break;
                    case CSOperator.BITWISE_AND:
                        emitter.Emit(SSMAOpCode.b_and);
                        break;
                    case CSOperator.BITWISE_OR:
                        emitter.Emit(SSMAOpCode.b_or);
                        break;
                    case CSOperator.BITWISE_XOR:
                        emitter.Emit(SSMAOpCode.b_xor);
                        break;

                    // if condition met, value should be non-0
                    case CSOperator.EQUAL:
                        emitter.Emit(SSMAOpCode.cmp);
                        emitter.Emit(SSMAOpCode.not);
                        break;
                    case CSOperator.UNEQUAL:
                        emitter.Emit(SSMAOpCode.cmp);
                        break;
                    case CSOperator.GREATER:
                        emitter.Emit(SSMAOpCode.cmp);
                        emitter.Emit(SSMAOpCode.dec);
                        emitter.Emit(SSMAOpCode.not);
                        break;
                    case CSOperator.GREATER_EQUAL:
                        emitter.Emit(SSMAOpCode.cmp);
                        emitter.Emit(SSMAOpCode.inc);
                        break;
                    case CSOperator.LESS:
                        emitter.Emit(SSMAOpCode.cmp);
                        emitter.Emit(SSMAOpCode.inc);
                        emitter.Emit(SSMAOpCode.not);
                        break;
                    case CSOperator.LESS_EQUAL:
                        emitter.Emit(SSMAOpCode.cmp);
                        emitter.Emit(SSMAOpCode.dec);
                        break;
                }
            }
        }
    }
}
