namespace CSharpCLI { 

using System;
using System.Text;

/// <summary>
/// Communication buffer used by Java Gigabase CLI to communicate with server
/// </summary>
internal class ComBuffer {
    static Encoding encoder = new UTF8Encoding();

    internal static int packShort(byte[] buf, int offs, int val) {
        buf[offs++] = (byte)(val >> 8);
        buf[offs++] = (byte)val;
        return offs;
    }
    internal static int packInt(byte[] buf, int offs, int val) {
        buf[offs++] = (byte)(val >> 24);
        buf[offs++] = (byte)(val >> 16);
        buf[offs++] = (byte)(val >> 8);
        buf[offs++] = (byte)val;
        return offs;
    }
    internal static int packLong(byte[] buf, int offs, long val) {
        return packInt(buf, packInt(buf, offs, (int)(val >> 32)), (int)val);
    }
    internal static int packFloat(byte[] buf, int offs, float value) {
        return packInt(buf, offs, BitConverter.ToInt32(BitConverter.GetBytes(value), 0));
    }
    internal static int packDouble(byte[] buf, int offs, double value) {
        return packLong(buf, offs, BitConverter.DoubleToInt64Bits(value));
    }

    internal static short unpackShort(byte[] buf, int offs) {
        return (short)((buf[offs] << 8) + ( buf[offs+1] & 0xFF)); 
    }
    internal static int unpackInt(byte[] buf, int offs) {
        return (buf[offs] << 24) + (( buf[offs+1] & 0xFF) << 16) 
            +  ((buf[offs+2] & 0xFF) << 8) + ( buf[offs+3] & 0xFF); 
    }
    internal static long unpackLong(byte[] buf, int offs) {
        return ((long)unpackInt(buf, offs) << 32) 
            + ((long)unpackInt(buf, offs+4) & 0xFFFFFFFFL);
    }
    internal static float unpackFloat(byte[] buf, int offs) {
        return BitConverter.ToSingle(BitConverter.GetBytes(unpackInt(buf, offs)), 0);
    }
    internal static double unpackDouble(byte[] buf, int offs) {
        return BitConverter.Int64BitsToDouble(unpackLong(buf, offs));
    }

    internal ComBuffer(Connection.CLICommand cmd, int id) {
        int size = 12;
        buf = new byte[size];
        pos = 0;
        putInt(size);
        putInt((int)cmd);
        putInt(id);
    }

    internal ComBuffer(Connection.CLICommand cmd) : this(cmd, 0) {}

    internal void reset(int size) {
        if (buf.Length < size) { 
            buf = new byte[size];
        }
        pos = 0;
    }


    internal void end() { 
        packInt(buf, 0, pos);
    }

    internal void extend(int len) {  
        if (pos + len > buf.Length) { 
            int newLen = pos + len > len*2 ? pos + len : len*2;
            byte[] newBuf = new byte[newLen];
            System.Array.Copy(buf, 0, newBuf, 0, buf.Length); 
            buf = newBuf;
        }
    }

    internal void putByte(int val) {
        extend(1);
        buf[pos++] = (byte)val;
    }

    internal void putShort(int val) {
        extend(2);
        pos = packShort(buf, pos, val);
    }

    internal void putInt(int val) {
        extend(4);
        pos = packInt(buf, pos, val);
    }

    internal void putLong(long val) {
        extend(8);
        pos = packLong(buf, pos, val);
    }

    internal void putDouble(double val) {
        extend(8);
        pos = packDouble(buf, pos, val);
    }

    internal void putFloat(float val) {
        extend(4);
        pos = packFloat(buf, pos, val);
    }

    internal void putRectangle(Rectangle r) { 
        extend(16);
        pos = packInt(buf, pos, r.x0);
        pos = packInt(buf, pos, r.y0);
        pos = packInt(buf, pos, r.x1);
        pos = packInt(buf, pos, r.y1);
    }

    internal void putAsciiz(String str) { 
        if (str != null) { 
            byte[] bytes = encoder.GetBytes(str);
            int len = bytes.Length;
            int i, j;
            extend(len+1);
            byte[] dst = buf;
            for (i = pos, j = 0; j < len; dst[i++] = bytes[j++]);
            dst[i++] = (byte)'\0';
            pos = i;
        } else { 
            extend(1);
            buf[pos++] = (byte)'\0';
        }
    }

    internal void putByteArray(byte[] arr) { 
        int len = arr == null ? 0 : arr.Length; 
        extend(len+4);
        pos = packInt(buf, pos, len);
        System.Array.Copy(arr, 0, buf, pos, len);
        pos += len;
    }
    internal void putString(String str) { 
        if (str == null) { 
            extend(5);
            pos = packInt(buf, pos, 1);
        } else { 
            byte[] bytes = encoder.GetBytes(str);
            int len = bytes.Length;
            extend(len+5);
            pos = packInt(buf, pos, len+1);
            System.Array.Copy(bytes, 0, buf, pos, len);
            pos += len;
        }
        buf[pos++] = (byte)'\0';
    }

    internal byte getByte() { 
        return buf[pos++];
    }

    internal short getShort() { 
        short value = unpackShort(buf, pos);
        pos += 2;
        return value;
    }

    internal int getInt() { 
        int value = unpackInt(buf, pos);
        pos += 4;
        return value;
    }

    internal long getLong() { 
        long value = unpackLong(buf, pos);
        pos += 8;
        return value;
    }

    internal float getFloat() { 
        float value = unpackFloat(buf, pos);
        pos += 4;
        return value;
    }

    internal double getDouble() { 
        double value = unpackDouble(buf, pos);
        pos += 8;
        return value;
    }

    internal string getAsciiz() { 
        byte[] p = buf;
        int i = pos, j = i;
        while (p[j++] != '\0');
        pos = j;
        return encoder.GetString(p, i, j-i-1);
    }

    internal string getString() { 
        int len = getInt();
        String value = encoder.GetString(buf, pos, len-1);
        pos += len;
        return value;
    }

    internal Rectangle getRectangle() { 
        Rectangle r = new Rectangle(unpackInt(buf, pos), 
                                    unpackInt(buf, pos+4), 
                                    unpackInt(buf, pos+8), 
                                    unpackInt(buf, pos+12));
        pos += 16;
        return r;
    }               

    internal byte[] buf;
    internal int    pos;
}

}