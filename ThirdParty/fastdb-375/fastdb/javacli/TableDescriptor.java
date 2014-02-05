package javacli;

import java.lang.reflect.*;
import java.util.Date;

class TableDescriptor { 
    Field[]     columns;
    int         nColumns;
    int[]       types;
    Class       cls;
    Constructor constructor;
    boolean     autoincrement;

    static final Object[] bypassFlag = { new Boolean(true) };
    static Method         setBypass;
    static Class          stringType;
    static Class          dateType;
    static final Class[]  constructorProfile = new Class[0];
    static final Object[] constructorParameters = new Object[0];

    static { 
	try { 
	    // setAccessible() method is available only in JDK 1.2
	    Class c = Class.forName("java.lang.reflect.AccessibleObject");
	    Class[] param = { Boolean.TYPE };
	    setBypass = c.getMethod("setAccessible", param);
	} catch(Exception x) {}

        try { 
	    stringType = Class.forName("java.lang.String");
	    dateType = Class.forName("java.util.Date");
	} catch(Exception x) {}
    }

    TableDescriptor(Class tableClass) { 
        Class c;
	int i, n;

        for (c = tableClass, n = 0; c != null; c = c.getSuperclass()) { 
            Field[] classFields = c.getDeclaredFields();            
            for (i = 0; i < classFields.length; i++) {
                if ((classFields[i].getModifiers() & (Modifier.STATIC|Modifier.TRANSIENT)) == 0) {
                    n += 1;
                }
            }
        } 

	columns = new Field[n];
	types = new int[n];
        nColumns = n;
        cls = tableClass;

        try { 
            constructor = tableClass.getDeclaredConstructor(constructorProfile);
        } catch(Exception x) {
            throw new CliError("Default constructor is not accessible in class " + tableClass.getName());
        }
        if (setBypass != null) { 
            try { 
                setBypass.invoke(constructor, bypassFlag);
            } catch(Exception x) {}
        }
        for (c = tableClass, n = 0; c != null; c = c.getSuperclass()) { 
            Field[] classFields = c.getDeclaredFields();            
            for (i = 0; i < classFields.length; i++) {
                Field f = classFields[i];
                if ((f.getModifiers() & (Modifier.STATIC|Modifier.TRANSIENT)) == 0) {               
                    if (setBypass != null) { 
                        try { 
                            setBypass.invoke(f, bypassFlag);
                        } catch(Exception x) {
                            throw new CliError("Field '" + f.getName() + "' is not accessible in class "
                                               + tableClass.getName());
                        }
                    }
                    Class type = f.getType();
                    int cliType;
                    if (type == Byte.TYPE) { 
                        cliType = Connection.cli_int1;
                    } else if (type == Short.TYPE) { 
                        cliType = Connection.cli_int2;
                    } else if (type == Integer.TYPE) { 
                        if ((f.getModifiers() & Modifier.VOLATILE) != 0) { 
                            cliType = Connection.cli_autoincrement;
                            autoincrement = true;
                        } else { 
                            cliType = Connection.cli_int4;
                        }
                    } else if (type == Boolean.TYPE) { 
                        cliType = Connection.cli_bool;
                    } else if (type == Long.TYPE) { 
                        cliType = Connection.cli_int8;
                    } else if (type == Float.TYPE) { 
                        cliType = Connection.cli_real4;
                    } else if (type == Double.TYPE) { 
                        cliType = Connection.cli_real8;
                    } else if (type == Reference.TYPE) { 
                        cliType = Connection.cli_oid;
                    } else if (type == Rectangle.TYPE) { 
                        cliType = Connection.cli_rectangle;
                    } else if (type == stringType) { 
                        cliType = Connection.cli_asciiz;
                    } else if (type == dateType) { 
                        cliType = Connection.cli_datetime;
                    } else if (type.isArray()) { 
                        type = type.getComponentType();
                        if (type == Byte.TYPE) { 
                            cliType = Connection.cli_array_of_int1;
                        } else if (type == Short.TYPE) { 
                            cliType = Connection.cli_array_of_int2;
                        } else if (type == Integer.TYPE) { 
                            cliType = Connection.cli_array_of_int4;
                        } else if (type == Boolean.TYPE) { 
                            cliType = Connection.cli_array_of_bool;
                        } else if (type == Long.TYPE) { 
                            cliType = Connection.cli_array_of_int8;
                        } else if (type == Float.TYPE) { 
                            cliType = Connection.cli_array_of_real4;
                        } else if (type == Double.TYPE) { 
                            cliType = Connection.cli_array_of_real8;
                        } else if (type == Reference.TYPE) { 
                            cliType = Connection.cli_array_of_oid;
                        } else if (type == stringType) { 
                            cliType = Connection.cli_array_of_string;
                        } else { 
                            throw new CliError("Unsupported array type " + type.getName());
                        }                   
                    } else { 
                        throw new CliError("Unsupported field type " + type.getName());
                    }               
                    columns[n] = f;
                    types[n++] = cliType;
                }
            }
        }
    }   

    void writeColumnDefs(ComBuffer buf) { 
        for (int i = 0, n = nColumns; i < n; i++) { 
            buf.putByte(types[i]);
            buf.putAsciiz(columns[i].getName());
        }
    }

    void writeColumnValues(ComBuffer buf, Object obj) { 
        int i, j, n, len;
        try { 
            for (i = 0, n = nColumns; i < n; i++) { 
                switch (types[i]) { 
                  case Connection.cli_int1:
                    buf.putByte(columns[i].getByte(obj));
                    break;
                  case Connection.cli_int2:
                    buf.putShort(columns[i].getShort(obj));
                    break;
                  case Connection.cli_int4:
                    buf.putInt(columns[i].getInt(obj));
                    break;
                  case Connection.cli_int8:
                    buf.putLong(columns[i].getLong(obj));
                    break;
                  case Connection.cli_real4:
                    buf.putFloat(columns[i].getFloat(obj));
                    break;
                  case Connection.cli_real8:
                    buf.putDouble(columns[i].getDouble(obj));
                    break;
                  case Connection.cli_bool:
                    buf.putByte(columns[i].getBoolean(obj) ? 1 : 0);
                    break;
                  case Connection.cli_oid:
                    Reference ref = (Reference)columns[i].get(obj);
                    buf.putInt(ref != null ? ref.oid : 0);
                    break;
                  case Connection.cli_rectangle:
                    Rectangle rect = (Rectangle)columns[i].get(obj);
                    if (rect == null) { 
                        rect = new Rectangle();
                    }
                    buf.putRectangle(rect);
                    break;
                  case Connection.cli_asciiz:
                    buf.putString((String)columns[i].get(obj)); 
                    break;
                  case Connection.cli_datetime:
                    buf.putInt((int)(((Date)columns[i].get(obj)).getTime() / 1000));
                    break;
                  case Connection.cli_array_of_int1:
                    buf.putByteArray((byte[])columns[i].get(obj));
                    break;
                  case Connection.cli_array_of_int2:
                    { 
                        short[] arr = (short[])columns[i].get(obj);
                        len = arr == null ? 0 : arr.length;
                        buf.putInt(len);
                        for (j = 0; j < len; j++) { 
                            buf.putShort(arr[j]);
                        }
                        break;
                    }
                  case Connection.cli_array_of_int4:
                    { 
                        int[] arr = (int[])columns[i].get(obj);
                        len = arr == null ? 0 : arr.length;
                        buf.putInt(len);
                        for (j = 0; j < len; j++) { 
                            buf.putInt(arr[j]);
                        }
                        break;
                    }
                  case Connection.cli_array_of_int8:
                    { 
                        long[] arr = (long[])columns[i].get(obj);
                        len = arr == null ? 0 : arr.length;
                        buf.putInt(len);
                        for (j = 0; j < len; j++) { 
                            buf.putLong(arr[j]);
                        }
                        break;
                    }
                  case Connection.cli_array_of_real4:
                    { 
                        float[] arr = (float[])columns[i].get(obj);
                        len = arr == null ? 0 : arr.length;
                        buf.putInt(len);
                        for (j = 0; j < len; j++) { 
                            buf.putFloat(arr[j]);
                        }
                        break;
                    }
                  case Connection.cli_array_of_real8:
                    { 
                        double[] arr = (double[])columns[i].get(obj);
                        len = arr == null ? 0 : arr.length;
                        buf.putInt(len);
                        for (j = 0; j < len; j++) { 
                            buf.putDouble(arr[j]);
                        }
                        break;
                    }
                  case Connection.cli_array_of_bool:
                    { 
                        boolean[] arr = (boolean[])columns[i].get(obj);
                        len = arr == null ? 0 : arr.length;
                        buf.putInt(len);
                        for (j = 0; j < len; j++) { 
                            buf.putByte(arr[j] ? 1 : 0);
                        }
                        break;
                    }
                  case Connection.cli_array_of_oid:
                    { 
                        Reference[] arr = (Reference[])columns[i].get(obj);
                        len = arr == null ? 0 : arr.length;
                        buf.putInt(len);
                        for (j = 0; j < len; j++) { 
                            buf.putInt(arr[j] != null ? arr[j].oid : 0);
                        }
                        break;
                    }
                  case Connection.cli_array_of_string:
                    { 
                        String[] arr = (String[])columns[i].get(obj);
                        len = arr == null ? 0 : arr.length;
                        buf.putInt(len);
                        for (j = 0; j < len; j++) { 
                            buf.putAsciiz(arr[j]);
                        }
                        break;
                    }
                  case Connection.cli_autoincrement:
                    break;
                  default:
                    throw new CliError("Unsupported type " + types[i]);
                }
            }   
        } catch (Exception x) { 
            x.printStackTrace();
            throw new CliError("Object access error: " + x);
        }
    }

    Object readObject(ComBuffer buf) { 
        try { 
            Object obj = constructor.newInstance(constructorParameters);
            int i, j, n, len;
            for (i = 0, n = nColumns; i < n; i++) { 
                int type = buf.getByte();
                if (type != types[i]) { 
                    throw new CliError("Unexpected type of column: " + type 
                                       + " instead of " + types[i]);
                }
                switch (types[i]) { 
                  case Connection.cli_int1:
                    columns[i].setByte(obj, buf.getByte());
                    break;
                  case Connection.cli_int2:
                    columns[i].setShort(obj, buf.getShort());
                    break;
                  case Connection.cli_int4:
                  case Connection.cli_autoincrement:
                    columns[i].setInt(obj, buf.getInt());
                    break;
                  case Connection.cli_int8:
                    columns[i].setLong(obj, buf.getLong());
                    break;
                  case Connection.cli_real4:
                    columns[i].setFloat(obj, buf.getFloat());
                    break;
                  case Connection.cli_real8:
                    columns[i].setDouble(obj, buf.getDouble());
                    break;
                  case Connection.cli_bool:
                    columns[i].setBoolean(obj, buf.getByte() != 0);
                    break;
                  case Connection.cli_oid:
                    { 
                        int oid = buf.getInt();
                        columns[i].set(obj, (oid != 0) ? new Reference(oid) : null);
                        break;
                    } 
                  case Connection.cli_rectangle:
                    columns[i].set(obj, buf.getRectangle());
                    break;
                  case Connection.cli_asciiz:
                    columns[i].set(obj, buf.getString());
                    break;
                  case Connection.cli_datetime:
                    columns[i].set(obj, new Date(((long)buf.getInt() & 0xFFFFFFFFL)*1000));
                    break;
                  case Connection.cli_array_of_int1:
                    { 
                        len = buf.getInt();
                        byte[] arr = new byte[len];
                        for (j = 0; j < len; j++) { 
                            arr[j] = buf.getByte();
                        }
                        columns[i].set(obj, arr);
                        break;
                    }
                  case Connection.cli_array_of_int2:
                    { 
                        len = buf.getInt();
                        short[] arr = new short[len];
                        for (j = 0; j < len; j++) { 
                            arr[j] = buf.getShort();
                        }
                        columns[i].set(obj, arr);
                        break;
                    }
                  case Connection.cli_array_of_int4:
                    { 
                        len = buf.getInt();
                        int[] arr = new int[len];
                        for (j = 0; j < len; j++) { 
                            arr[j] = buf.getInt();
                        }
                        columns[i].set(obj, arr);
                        break;
                    }
                  case Connection.cli_array_of_int8:
                    { 
                        len = buf.getInt();
                        long[] arr = new long[len];
                        for (j = 0; j < len; j++) { 
                            arr[j] = buf.getLong();
                        }
                        columns[i].set(obj, arr);
                        break;
                    }
                  case Connection.cli_array_of_real4:
                    { 
                        len = buf.getInt();
                        float[] arr = new float[len];
                        for (j = 0; j < len; j++) { 
                            arr[j] = buf.getFloat();
                        }
                        columns[i].set(obj, arr);
                        break;
                    }
                  case Connection.cli_array_of_real8:
                    { 
                        len = buf.getInt();
                        double[] arr = new double[len];
                        for (j = 0; j < len; j++) { 
                            arr[j] = buf.getDouble();
                        }
                        columns[i].set(obj, arr);
                        break;
                    }
                  case Connection.cli_array_of_bool:
                    { 
                        len = buf.getInt();
                        boolean[] arr = new boolean[len];
                        for (j = 0; j < len; j++) { 
                            arr[j] = buf.getByte() != 0;
                        }
                        columns[i].set(obj, arr);
                        break;
                    }
                  case Connection.cli_array_of_oid:
                    { 
                        len = buf.getInt();
                        Reference[] arr = new Reference[len];
                        for (j = 0; j < len; j++) { 
                            int oid = buf.getInt();
                            arr[j] = oid != 0 ? new Reference(oid) : null;
                        }
                        columns[i].set(obj, arr);
                        break;
                    }
                  case Connection.cli_array_of_string:
                    { 
                        len = buf.getInt();
                        String[] arr = new String[len];
                        for (j = 0; j < len; j++) { 
                            arr[j] = buf.getAsciiz();
                        }
                        columns[i].set(obj, arr);
                        break;
                    }
                  default:
                    throw new CliError("Unsupported type " + types[i]);
                }
            }   
            return obj;
        } catch (Exception x) { 
            throw new CliError("Object access error: " + x);
        }
    }
        
}





