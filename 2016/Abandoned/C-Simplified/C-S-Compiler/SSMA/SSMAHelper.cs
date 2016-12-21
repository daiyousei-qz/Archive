using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EdsLab.CSimplified
{
    static class SSMAHelper
    {
        public static void LoadVarAddress(SSMAEmitter emitter, string id)
        {
            var offset = CSAnalyzer._symbolTable[id];
            if (offset == 0)
                emitter.Emit(SSMAOpCode.push_r, SSMARegister.RSB);
            else
                emitter.Emit(SSMAOpCode.push_ra, SSMARegister.RSB, offset.ToString());
        }

        public static void PushUInt(SSMAEmitter emitter, uint val)
        {
            if (val == 0)
            {
                emitter.Emit(SSMAOpCode.push_c0);
            }
            else if (val == 1)
            {
                emitter.Emit(SSMAOpCode.push_c1);
            }
            else if (val < 256)
            {
                emitter.Emit(SSMAOpCode.push_1, val.ToString());
            }
            else
            {
                emitter.Emit(SSMAOpCode.push_4, val.ToString());
            }
        }
    }
}
