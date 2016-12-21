using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EdsLab.SSMA
{
    class Program
    {
        #region OpSize
        const uint OP_V_Size = 1;
        const uint OP_U1_Size = 2;
        const uint OP_U2_Size = 3;
        const uint OP_U4_Size = 5;
        const uint OP_S1_Size = 2;
        const uint OP_S2_Size = 3;
        const uint OP_S4_Size = 5;
        const uint OP_U1U2_Size = 4;

        static readonly Dictionary<SSMAOpCode, uint> OpSize = new Dictionary<SSMAOpCode, uint>
        {
            [SSMAOpCode.term] =  OP_V_Size,
            [SSMAOpCode.nop] = OP_V_Size,

            [SSMAOpCode.add] = OP_V_Size,
            [SSMAOpCode.sub] = OP_V_Size,
            [SSMAOpCode.mul] = OP_V_Size,
            [SSMAOpCode.div] = OP_V_Size,
            [SSMAOpCode.mod] = OP_V_Size,
            [SSMAOpCode.b_and] = OP_V_Size,
            [SSMAOpCode.b_or] = OP_V_Size,
            [SSMAOpCode.b_xor] = OP_V_Size,
            [SSMAOpCode.b_rev] = OP_V_Size,
            [SSMAOpCode.not] = OP_V_Size,
            [SSMAOpCode.inc] = OP_V_Size,
            [SSMAOpCode.dec] = OP_V_Size,
            [SSMAOpCode.cmp] = OP_V_Size,

            [SSMAOpCode.push_c0] = OP_V_Size,
            [SSMAOpCode.push_c1] = OP_V_Size,
            [SSMAOpCode.push_1] = OP_U1_Size,
            [SSMAOpCode.push_4] = OP_U4_Size,
            [SSMAOpCode.push_r] = OP_U1_Size,
            [SSMAOpCode.push_ra] = OP_U1U2_Size,
            [SSMAOpCode.pop] = OP_V_Size,
            [SSMAOpCode.swap] = OP_V_Size,
            [SSMAOpCode.dup] = OP_V_Size,
            [SSMAOpCode.rec] = OP_V_Size,
            [SSMAOpCode.store_1] = OP_V_Size,
            [SSMAOpCode.store_2] = OP_V_Size,
            [SSMAOpCode.store_4] = OP_V_Size,
            [SSMAOpCode.load_1] = OP_V_Size,
            [SSMAOpCode.load_2] = OP_V_Size,
            [SSMAOpCode.load_4] = OP_V_Size,
            [SSMAOpCode.alloc] = OP_U2_Size,

            [SSMAOpCode.jmp] = OP_S2_Size,
            [SSMAOpCode.jmpz] = OP_S2_Size,
            [SSMAOpCode.jmpx] = OP_S2_Size,
            [SSMAOpCode.jmpn] = OP_S2_Size,
            [SSMAOpCode.call] = OP_U4_Size,
            [SSMAOpCode.ret] = OP_V_Size,
        };
        #endregion
        static readonly Dictionary<string, byte> Registers = new Dictionary<string, byte>
        {
            ["$RIP"] = 0,
            ["$RSB"] = 1,
            ["$RST"] = 2,
            ["$RTD"] = 3,
            ["$RMB"] = 4,
        };
        const string LabelMacro = "LABEL";
        const string SubroutineMacro = "SUBROUTINE";


        static List<string[]> Resolve(StreamReader reader)
        {
            List<string[]> lines = new List<string[]>();

            while(!reader.EndOfStream)
            {
                string line = reader.ReadLine();

                // line starting with ; is comment
                if (line.Length > 1 && line[0] != ';')
                {
                    var lexemes = line.Split(' ')
                                      .Where((s) => !string.IsNullOrWhiteSpace(s))
                                      .ToArray();

                    // skip empty lines
                    if (lexemes.Length > 0)
                        lines.Add(lexemes);
                }
            }

            return lines;
        }

        static Dictionary<string, uint> LoadLabels(List<string[]> lines)
        {
            var tmp = new Dictionary<string, uint>();
            uint offset = 0;

            // first scan iteration, load labels
            foreach (var line in lines)
            {
                if (line[0] == LabelMacro || line[0] == SubroutineMacro)
                {
                    // Label redefinition
                    if (tmp.ContainsKey(line[1]))
                        throw new Exception();

                    tmp[line[1]] = offset;
                }
                else // in this edition, must be a command
                {
                    SSMAOpCode op;
                    if (Enum.TryParse(line[0], out op))
                    {
                        offset += OpSize[op];
                    }
                    else
                    {
                        throw new SSMAException("Unexpected command at LINE " + lines.IndexOf(line));
                    }
                }
            }

            return tmp;
        }

        static byte[] Compile(StreamReader reader)
        {
            var lines = Resolve(reader);

            // first scanning iteration
            // this is a map from LABEL name to its offset
            Dictionary<string, uint> labels = LoadLabels(lines);

            // second scanning iteration, tranlate commands
            uint offset = 0;
            SSMAEmitter emitter = new SSMAEmitter();
            foreach (var line in lines)
            {
                // omit label definition
                if (line[0] == LabelMacro || line[0] == SubroutineMacro)
                    continue;

                // no need to validate command as previously done
                SSMAOpCode op;
                Enum.TryParse(line[0], out op);
                // update offset
                offset += OpSize[op];
                // emit opcode
                emitter.Emit(op);
                // spcial opcode with operand
                try
                {
                    switch (op)
                    {
                        case SSMAOpCode.push_1:
                            emitter.EmitUInt8(Convert.ToByte(line[1]));
                            break;
                        case SSMAOpCode.push_4:
                            emitter.EmitUInt32(Convert.ToUInt32(line[1]));
                            break;
                        case SSMAOpCode.push_r:
                            emitter.EmitUInt8(Registers[line[1]]);
                            break;
                        case SSMAOpCode.push_ra:
                            emitter.EmitUInt8(Registers[line[1]]);
                            emitter.EmitUInt16(Convert.ToUInt16(line[2]));
                            break;
                        case SSMAOpCode.alloc:
                            emitter.EmitUInt16(Convert.ToUInt16(line[1]));
                            break;
                        case SSMAOpCode.jmp:
                        case SSMAOpCode.jmpz:
                        case SSMAOpCode.jmpx:
                        case SSMAOpCode.jmpn:
                            var pos = labels[line[1]];
                            emitter.EmitInt16((short)(pos - offset));
                            break;
                        case SSMAOpCode.call:
                            emitter.EmitUInt32(labels[line[1]]);
                            break;
                    }
                }
                catch(FormatException)
                {
                    throw new SSMAException("Bad-formatted operand at LINE " + lines.IndexOf(line));
                }
                catch(OverflowException)
                {
                    throw new SSMAException("Overflowed operand at LINE " + lines.IndexOf(line));
                }
                catch(KeyNotFoundException)
                {
                    throw new SSMAException("Unknown register or label at LINE " + lines.IndexOf(line));
                }
            }

            return emitter.ToArray();
        }

        static void Main(string[] args)
        {
            var reader = new StreamReader(@"D:\ssma.txt");

            try
            {
                var code = Compile(reader);

                StringBuilder builder = new StringBuilder();
                foreach (var val in code)
                {
                    builder.Append(val);
                    builder.Append(", ");
                }

                string test = builder.ToString();
            }
            catch(SSMAException ex)
            {
                Console.WriteLine("Compilation interupted.");
                Console.WriteLine(ex.Message);
            }

            Console.ReadKey();
        }
    }
}
