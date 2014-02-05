package jnicli;

import java.lang.reflect.*;
import java.util.*;

/**
 * Interface to database using Java native interface. 
 * This class should be used by applications accessing database locally
 */
public class DatabaseJNI implements Database {     
    public void open(int accessType, String databaseName, String databasePath, long initSize, int transactionCommitDelay) {
        db = jniOpen(accessType, databaseName, databasePath, initSize, transactionCommitDelay);
        String table;
        classes = new Hashtable();
        while ((table = jniNextTable(db)) != null) { 
            try { 
                Class c = Class.forName(table);
                ClassDescriptor desc = new ClassDescriptor(c);
                classes.put(c, desc);
                desc.td = jniUpdateTable(db, table, getTableDescriptor(desc)); 
            } catch (ClassNotFoundException x) { 
                new CliException("Class '" + table + "' not found");
            } catch (Exception x) { 
                new CliException(x.getMessage());
            }
        }
    }
    
    public void close() {
        jniClose(db);
        classes = null;
    }

    public Cursor select(Class table, String condition, int flags) {
        ClassDescriptor desc = (ClassDescriptor)classes.get(table);
        if (desc == null) { 
            throw new CliException("Table not found: " + table);
        }
        long result = jniSelect(db, desc.td, condition);
        Cursor cursor;
        if (remote || (flags & AUTOCOMMIT) != 0) {
            cursor = new PrefetchedCursor(this, result, desc);
            if ((flags & AUTOCOMMIT) != 0) {
                jniCommit(db);
            }
        } else { 
            cursor = (flags & FOR_UPDATE) != 0
                ? (Cursor)new IncrementalCursorForUpdate(this, result, desc)
                : (Cursor)new IncrementalCursor(this, result, desc);
        }
        return cursor;
    }


    Object unswizzleObject(long cursor, ClassDescriptor desc) { 
        int[] types = desc.types;
        Field[] fields = desc.fields;        
        try {
            Object obj = desc.newInstance();
            for (int i = 0; i < types.length; i++) {
                Field f = fields[i];
                switch (types[i]) {
                case TP_BOOL:
                    f.setBoolean(obj, jniGetByte(cursor) != 0);
                    continue;
                case TP_CHAR:
                    f.setChar(obj, (char)jniGetShort(cursor));
                    continue;
                case TP_INT1:
                    f.setByte(obj, jniGetByte(cursor));
                    continue;
                case TP_INT2:
                    f.setShort(obj, jniGetShort(cursor));
                    continue;
                case TP_INT4:
                    f.setInt(obj, jniGetInt(cursor));
                    continue;
                case TP_INT8:
                    f.setLong(obj, jniGetLong(cursor));
                    continue;
                case TP_REAL4:
                    f.setFloat(obj, Float.intBitsToFloat(jniGetInt(cursor)));
                    continue;
                case TP_REAL8:
                    f.setDouble(obj, Double.longBitsToDouble(jniGetLong(cursor)));
                    continue;
                case TP_STRING:
                    f.set(obj, jniGetString(cursor));
                    continue;
                case TP_DATE:
                {
                    long timestamp = jniGetLong(cursor);
                    f.set(obj, timestamp < 0 ? null : new Date(timestamp));
                    continue;                   
                }
                case TP_BOOL_ARRAY:
                    f.set(obj, jniGetBoolArray(cursor));
                    continue;
                case TP_CHAR_ARRAY:
                    f.set(obj, jniGetCharArray(cursor));
                    continue;
                 case TP_INT1_ARRAY:
                    f.set(obj, jniGetByteArray(cursor));
                    continue;
                case TP_INT2_ARRAY:
                    f.set(obj, jniGetShortArray(cursor));
                    continue;
                case TP_INT4_ARRAY:
                    f.set(obj, jniGetIntArray(cursor));
                    continue;
                case TP_INT8_ARRAY:
                    f.set(obj, jniGetLongArray(cursor));
                    continue;
                case TP_REAL4_ARRAY:
                    f.set(obj, jniGetFloatArray(cursor));
                    continue;
                case TP_REAL8_ARRAY:
                    f.set(obj, jniGetDoubleArray(cursor));
                    continue;
                case TP_STRING_ARRAY:
                    f.set(obj, jniGetStringArray(cursor));
                    continue;
                default:
                    throw new CliException("Invalid field type: " + types[i]);
                }
            }
            return obj;              
        } catch (Exception x) { 
            throw new CliException(x.getMessage());
        }
    }

    private long swizzleObject(ClassDescriptor desc, Object obj) {
        long buf = jniCreateBuffer();
        int[] types = desc.types;
        Field[] fields = desc.fields;        
        try {
            for (int i = 0; i < types.length; i++) {
                Field f = fields[i];
                switch (types[i]) {
                case TP_BOOL:
                    jniSetByte(buf, (byte)(f.getBoolean(obj) ? 1 : 0));
                    continue;
                case TP_CHAR:
                    jniSetShort(buf, (short)f.getChar(obj));
                    continue;
                case TP_INT1:
                    jniSetByte(buf, f.getByte(obj));
                    continue;
                case TP_INT2:
                    jniSetShort(buf, f.getShort(obj));
                    continue;
                case TP_INT4:
                    jniSetInt(buf, f.getInt(obj));
                    continue;
                case TP_INT8:
                    jniSetLong(buf, f.getLong(obj));
                    continue;
                case TP_REAL4:
                    jniSetInt(buf, Float.floatToIntBits(f.getFloat(obj)));
                    continue;
                case TP_REAL8:
                    jniSetLong(buf, Double.doubleToLongBits(f.getDouble(obj)));
                    continue;
                case TP_STRING:
                    jniSetString(buf, (String)f.get(obj));
                    continue;
                case TP_DATE:
                {
                    Date d = (Date)f.get(obj);
                    jniSetLong(buf, d != null ? d.getTime() : -1);
                    continue;                   
                }
                case TP_BOOL_ARRAY:
                    jniSetBoolArray(buf, (boolean[])f.get(obj));
                    continue;
                case TP_CHAR_ARRAY:
                    jniSetCharArray(buf, (char[])f.get(obj));
                    continue;
                case TP_INT1_ARRAY:
                    jniSetByteArray(buf, (byte[])f.get(obj));
                    continue;
                case TP_INT2_ARRAY:
                    jniSetShortArray(buf, (short[])f.get(obj));
                    continue;
                case TP_INT4_ARRAY:
                    jniSetIntArray(buf, (int[])f.get(obj));
                    continue;
                case TP_INT8_ARRAY:
                    jniSetLongArray(buf, (long[])f.get(obj));
                    continue;
                case TP_REAL4_ARRAY:
                    jniSetFloatArray(buf, (float[])f.get(obj));
                    continue;
                case TP_REAL8_ARRAY:
                    jniSetDoubleArray(buf, (double[])f.get(obj));
                    continue;
                case TP_STRING_ARRAY:
                    jniSetStringArray(buf, (String[])f.get(obj));
                    continue;
                default:
                    throw new CliException("Invalid field type: " + types[i]);
                }
            }
        } catch (Exception x) { 
            throw new CliException(x.getMessage());
        }
        return buf;
    }

    public void update(long oid, Object obj) {
        Class c = obj.getClass();
        ClassDescriptor desc = (ClassDescriptor)classes.get(c);
        if (desc == null) { 
            throw new CliException("Table not found: " + c);
        }
        jniUpdate(db, oid, desc.td, swizzleObject(desc, obj));
    }

    
    
    public long insert(Object obj) {
        Class c = obj.getClass();
        String table = c.getName();
        ClassDescriptor desc = (ClassDescriptor)classes.get(c);
        if (desc == null) { 
            desc = new ClassDescriptor(c);
            classes.put(c, desc);
            desc.td = jniCreateTable(db, table, getTableDescriptor(desc));            
        }
        return jniInsert(db, desc.td, swizzleObject(desc, obj));
    }
        
    public int delete(Class table, String condition) {
        ClassDescriptor desc = (ClassDescriptor)classes.get(table);
        if (desc == null) { 
            throw new CliException("Table not found: " + table);
        }
        return jniDelete(db, desc.td, condition);
    }


    public void commit() {
        jniCommit(db);
    }

    public void rollback() {
        jniRollback(db);
    }

    public void lock() {
        jniLock(db);
    }    

    public void attach() {
        jniAttach(db);
    }

    public void detach(int flags) {
        jniDetach(db, flags);
    }

    private long getTableDescriptor(ClassDescriptor desc) { 
        String[] fieldNames = desc.getFieldNames();        
        try { 
            ConstraintParser constraintParser = new ConstraintParser(fieldNames, desc.getConstraints());
            return jniCreateTableDescriptor(db,
                                            desc.cls.getName(), 
                                            fieldNames, 
                                            desc.types, 
                                            constraintParser.getReferencedTables(), 
                                            constraintParser.getInverseFields(), 
                                            constraintParser.getConstraintMasks());
        } catch (Exception x) {
            throw new CliException(x.getMessage());
        }
    } 

    public void setThreadContext(long ctx) {
        jniSetThreadContext(db, ctx);
    }

    public long getThreadContext() {
        return jniGetThreadContext(db);
    }

    public DatabaseJNI() { 
        this(false);
    }
    
    public DatabaseJNI(boolean remote) { 
        this.remote = remote;
    }

    native long   jniOpen(int accessType, String databaseName, String databasePath, long initSize, int transactionCommitDelay);
    native void   jniClose(long db);

    native String jniNextTable(long db);

    native void   jniSetByte(long buf, byte v);
    native void   jniSetShort(long buf, short v);
    native void   jniSetInt(long buf, int v);
    native void   jniSetLong(long buf, long v);
    native void   jniSetString(long buf, String v);
    native void   jniSetBoolArray(long buf, boolean[] v);
    native void   jniSetCharArray(long buf, char[] v);
    native void   jniSetByteArray(long buf, byte[] v);
    native void   jniSetShortArray(long buf, short[] v);
    native void   jniSetIntArray(long buf, int[] v);
    native void   jniSetLongArray(long buf, long[] v);
    native void   jniSetFloatArray(long buf, float[] v);
    native void   jniSetDoubleArray(long buf, double[] v);
    native void   jniSetStringArray(long buf, String[] v);

    native long   jniCreateBuffer();
    native long   jniCreateTable(long db, String table, long desc);
    native long   jniUpdateTable(long db, String table, long desc);

    native long   jniNext(long cursor);
    native byte   jniGetByte(long cursor);
    native short  jniGetShort(long cursor);
    native int    jniGetInt(long cursor);
    native long   jniGetLong(long cursor);
    native String jniGetString(long cursor);
    native boolean[] jniGetBoolArray(long cursor);
    native char[]    jniGetCharArray(long cursor);
    native byte[]    jniGetByteArray(long cursor);
    native short[]   jniGetShortArray(long cursor);
    native int[]     jniGetIntArray(long cursor);
    native long[]    jniGetLongArray(long cursor);
    native float[]   jniGetFloatArray(long cursor);
    native double[]  jniGetDoubleArray(long cursor);
    native String[]  jniGetStringArray(long cursor);
    native void   jniCloseCursor(long cursor);

    native void   jniUpdate(long db, long oid, long tableDescriptor, long buf);
    native long   jniInsert(long db, long tableDescriptor, long buf);

    native long   jniSelect(long db, long tableDescriptor, String condition);
    native int    jniDelete(long db, long tableDescriptor, String condition);

    native int    jniGetNumberOfSelectedRecords(long cursor);

    native void   jniCommit(long db);
    native void   jniRollback(long db);
    native void   jniLock(long db);

    native long   jniGetThreadContext(long db);
    native void   jniSetThreadContext(long db, long context);

    native void   jniAttach(long db);
    native void   jniDetach(long db, int flags);

    native long   jniCreateTableDescriptor(long db, String table, String[] fields, int[] types, String[] referencedTables, String[] inverseFields, int[] constraintMasks);
    
    static { 
        //System.out.println("Before loading jnicli");
        Runtime.getRuntime().loadLibrary("jnicli");
        //System.out.println("After loading jnicli");
    }

    final static int TP_BOOL = 1;
    final static int TP_CHAR = 2;
    final static int TP_INT1 = 3;
    final static int TP_INT2 = 4;
    final static int TP_INT4 = 5;
    final static int TP_INT8 = 6;
    final static int TP_REAL4 = 7;
    final static int TP_REAL8 = 8;
    final static int TP_STRING = 9;
    final static int TP_DATE = 10;
    final static int TP_BOOL_ARRAY = 11;
    final static int TP_CHAR_ARRAY = 12;
    final static int TP_INT1_ARRAY = 13;
    final static int TP_INT2_ARRAY = 14;
    final static int TP_INT4_ARRAY = 15;
    final static int TP_INT8_ARRAY = 16;
    final static int TP_REAL4_ARRAY = 17;
    final static int TP_REAL8_ARRAY = 18;
    final static int TP_STRING_ARRAY = 19;

    long db;
    Hashtable classes;
    boolean   remote;
}


