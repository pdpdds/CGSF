package javacli;

/**
 * Communication buffer used by Java Gigabase CLI to communicate with server
 */
class ComBuffer {
    static final int packShort(byte[] buf, int offs, int val) {
	 buf[offs++] = (byte)(val >> 8);
	 buf[offs++] = (byte)val;
	return offs;
    }
    static final int packInt(byte[] buf, int offs, int val) {
	 buf[offs++] = (byte)(val >> 24);
	 buf[offs++] = (byte)(val >> 16);
	 buf[offs++] = (byte)(val >> 8);
	 buf[offs++] = (byte)val;
	return offs;
    }
    static final int packLong(byte[] buf, int offs, long val) {
        return packInt(buf, packInt(buf, offs, (int)(val >> 32)), (int)val);
    }
    static final int packFloat(byte[] buf, int offs, float value) {
	return packInt(buf, offs, Float.floatToIntBits(value));
    }
    static final int packDouble(byte[] buf, int offs, double value) {
	return packLong(buf, offs, Double.doubleToLongBits(value));
    }

    static final short unpackShort(byte[] buf, int offs) {
        return (short)((buf[offs] << 8) + ( buf[offs+1] & 0xFF)); 
    }
    static final int unpackInt(byte[] buf, int offs) {
        return (buf[offs] << 24) + (( buf[offs+1] & 0xFF) << 16) 
            +  ((buf[offs+2] & 0xFF) << 8) + ( buf[offs+3] & 0xFF); 
    }
    static final long unpackLong(byte[] buf, int offs) {
        return ((long)unpackInt(buf, offs) << 32) 
            + ((long)unpackInt(buf, offs+4) & 0xFFFFFFFFL);
    }
    static final float unpackFloat(byte[] buf, int offs) {
        return Float.intBitsToFloat(unpackInt(buf, offs));
    }
    static final double unpackDouble(byte[] buf, int offs) {
        return Double.longBitsToDouble(unpackLong(buf, offs));
    }

    ComBuffer(int cmd, int id) {
        int size = 12;
        buf = new byte[size];
        pos = 0;
        putInt(size);
        putInt(cmd);
        putInt(id);
    }

    ComBuffer(int cmd) {
        this(cmd, 0);
    }

    void reset(int size) {
        if (buf.length < size) { 
            buf = new byte[size];
        }
        pos = 0;
    }


    void end() { 
        packInt(buf, 0, pos);
    }

    void extend(int len) {  
        if (pos + len > buf.length) { 
            int newLen = pos + len > len*2 ? pos + len : len*2;
            byte[] newBuf = new byte[newLen];
            System.arraycopy(buf, 0, newBuf, 0, buf.length); 
            buf = newBuf;
        }
    }

    void putByte(int val) {
        extend(1);
        buf[pos++] = (byte)val;
    }

    void putShort(int val) {
        extend(2);
        pos = packShort(buf, pos, val);
    }

    void putInt(int val) {
        extend(4);
        pos = packInt(buf, pos, val);
    }

    void putLong(long val) {
        extend(8);
        pos = packLong(buf, pos, val);
    }

    void putDouble(double val) {
        extend(8);
        pos = packDouble(buf, pos, val);
    }

    void putFloat(float val) {
        extend(4);
        pos = packFloat(buf, pos, val);
    }

    void putRectangle(Rectangle r) { 
        extend(16);
        pos = packInt(buf, pos, r.x0);
        pos = packInt(buf, pos, r.y0);
        pos = packInt(buf, pos, r.x1);
        pos = packInt(buf, pos, r.y1);
    }

    void putAsciiz(String str) { 
        if (str != null) { 
            byte[] bytes = str.getBytes();
            int len = bytes.length;
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

    void putByteArray(byte[] arr) { 
        int len = arr == null ? 0 : arr.length; 
        extend(len+4);
        pos = packInt(buf, pos, len);
        System.arraycopy(arr, 0, buf, pos, len);
        pos += len;
    }
    void putString(String str) { 
        if (str == null) { 
            extend(5);
            pos = packInt(buf, pos, 1);
        } else { 
            byte[] bytes = str.getBytes();
            int len = bytes.length;
            extend(len+5);
            pos = packInt(buf, pos, len+1);
            System.arraycopy(bytes, 0, buf, pos, len);
            pos += len;
        }
        buf[pos++] = (byte)'\0';
    }

    byte getByte() { 
        return buf[pos++];
    }

    short getShort() { 
        short value = unpackShort(buf, pos);
        pos += 2;
        return value;
    }

    int getInt() { 
        int value = unpackInt(buf, pos);
        pos += 4;
        return value;
    }

    long getLong() { 
        long value = unpackLong(buf, pos);
        pos += 8;
        return value;
    }

    float getFloat() { 
        float value = unpackFloat(buf, pos);
        pos += 4;
        return value;
    }

    double getDouble() { 
        double value = unpackDouble(buf, pos);
        pos += 8;
        return value;
    }

    String getAsciiz() { 
        byte[] p = buf;
        int i = pos, j = i;
        while (p[j++] != '\0');
        pos = j;
        return new String(p, i, j-i-1);
    }

    String getString() { 
        int len = getInt();
        String value = new String(buf, pos, len-1);
        pos += len;
        return value;
    }

    Rectangle getRectangle() { 
        Rectangle r = new Rectangle(unpackInt(buf, pos), 
                                    unpackInt(buf, pos+4), 
                                    unpackInt(buf, pos+8), 
                                    unpackInt(buf, pos+12));
        pos += 16;
        return r;
    }               

    byte buf[];
    int  pos;
}
