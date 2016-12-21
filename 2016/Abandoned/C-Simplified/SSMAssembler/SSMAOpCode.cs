using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EdsLab.SSMA
{
    // This accords to SSMA specification
    enum SSMAOpCode : byte
    {
        term = 0,
        nop = 1,

        add = 2,
        sub = 3,
        mul = 4,
        div = 5,
        mod = 6,
        b_and = 7,
        b_or = 8,
        b_xor = 9,
        b_rev = 10,
        not = 11,
        inc = 12,
        dec = 13,
        cmp = 14,

        push_c0 = 20,
        push_c1 = 21,
        push_1 = 22, // [1+1] push a byte into ES
        push_4 = 23, // [1+4] push an 32-bit int into ES
        push_r = 24, // [1+1] push the value of a specific register into ES
        push_ra = 25,
        pop = 26,
        swap = 27,
        dup = 28,
        rec = 29,
        store_1 = 30,
        store_2 = 31,
        store_4 = 32,
        load_1 = 33,
        load_2 = 34,
        load_4 = 35,
        alloc = 36,

        jmp = 40, // [1+2] append an offset of 16-bit int to $RCP
        jmpz = 41, // [1+2] jmp if 0 on the top of ES, and pop
        jmpx = 42,
        jmpn = 43, // [1+2] jmp if negative on the top of ES, and pop
        call = 44,
        ret = 45,
    }
}
