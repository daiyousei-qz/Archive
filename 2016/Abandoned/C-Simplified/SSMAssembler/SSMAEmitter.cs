using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EdsLab.SSMA
{
    class SSMAEmitter
    {
        List<byte> _buf = new List<byte>();
        int _index = 0;

        public void Emit(SSMAOpCode val)
        {
            EmitUInt8((byte)val);
        }

        public void EmitUInt8(byte val)
        {
            _buf.Add(val);
            ++_index;
        }

        public void EmitUInt16(ushort val)
        {
            _buf.AddRange(BitConverter.GetBytes(val));
            _index += sizeof(ushort);
        }

        public void EmitUInt32(uint val)
        {
            _buf.AddRange(BitConverter.GetBytes(val));
            _index += sizeof(uint);
        }

        public void EmitInt16(short val)
        {
            _buf.AddRange(BitConverter.GetBytes(val));
            _index += sizeof(short);
        }

        public byte[] ToArray()
        {
            return _buf.ToArray();
        }
    }
}
