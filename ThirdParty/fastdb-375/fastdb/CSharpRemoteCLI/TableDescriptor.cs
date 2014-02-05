namespace CSharpCLI { 

using System;
using System.Reflection;


internal class TableDescriptor { 
    internal FieldInfo[]          columns;
    internal int                  nColumns;
    internal Connection.CLIType[] types;
    internal Type                 cls;
    internal ConstructorInfo      constructor;
    internal bool                 autoincrement;

    static Type[]   constructorProfile = new Type[0];
    static object[] constructorParameters = new object[0];


    internal TableDescriptor(Type tableClass) { 
	int i, n;
        Type c;

        for (c = tableClass, n = 0; c != null; c = c.BaseType) { 
            FieldInfo[] classFields = c.GetFields(BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.DeclaredOnly);
            for (i = 0; i < classFields.Length; i++) {
                FieldInfo f = classFields[i];
	        if (!f.IsStatic && !f.IsNotSerialized) { 		    
                    n += 1;
                }
            }
        }
	columns = new FieldInfo[n];
	types = new Connection.CLIType[n];
        nColumns = n;
	cls = tableClass;

	constructor = tableClass.GetConstructor(constructorProfile);

        for (c = tableClass, n = 0; c != null; c = c.BaseType) { 
            FieldInfo[] classFields = c.GetFields(BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.DeclaredOnly);
            for (i = 0; i < classFields.Length; i++) {
	        FieldInfo f = classFields[i];
                if (!f.IsStatic && !f.IsNotSerialized) {                
                    columns[n] = f;
                    Type type = f.FieldType;
                    Connection.CLIType cliType;
                    if (type == typeof(byte)) { 
                        cliType = Connection.CLIType.cli_int1;
                    } else if (type == typeof(short)) { 
                        cliType = Connection.CLIType.cli_int2;
                    } else if (type == typeof(int)) {                   
                        if (f.GetCustomAttributes(typeof(AutoincrementAttribute), false).Length > 0) {
                            cliType = Connection.CLIType.cli_autoincrement;
                            autoincrement = true;
                        } 
                        else { 
                            cliType = Connection.CLIType.cli_int4;
                        }
                    } else if (type == typeof(bool)) { 
                        cliType = Connection.CLIType.cli_bool;
                    } else if (type == typeof(long)) { 
                        cliType = Connection.CLIType.cli_int8;
                    } else if (type == typeof(float)) { 
                        cliType = Connection.CLIType.cli_real4;
                    } else if (type == typeof(double)) { 
                        cliType = Connection.CLIType.cli_real8;
                    } else if (type == typeof(Reference)) { 
                        cliType = Connection.CLIType.cli_oid;
                    } else if (type == typeof(Rectangle)) { 
                        cliType = Connection.CLIType.cli_rectangle;
                    } else if (type == typeof(string)) { 
                        cliType = Connection.CLIType.cli_asciiz;
                    } else if (type == typeof(DateTime)) { 
                        cliType = Connection.CLIType.cli_datetime;
                    } else if (type.IsArray) { 
                        type = type.GetElementType();
                        if (type == typeof(byte)) { 
                            cliType = Connection.CLIType.cli_array_of_int1;
                        } else if (type == typeof(short)) { 
                            cliType = Connection.CLIType.cli_array_of_int2;
                        } else if (type == typeof(int)) { 
                            cliType = Connection.CLIType.cli_array_of_int4;
                        } else if (type == typeof(bool)) { 
                            cliType = Connection.CLIType.cli_array_of_bool;
                        } else if (type == typeof(long)) { 
                            cliType = Connection.CLIType.cli_array_of_int8;
                        } else if (type == typeof(float)) { 
                            cliType = Connection.CLIType.cli_array_of_real4;
                        } else if (type == typeof(double)) { 
                            cliType = Connection.CLIType.cli_array_of_real8;
                        } else if (type == typeof(Reference)) { 
                            cliType = Connection.CLIType.cli_array_of_oid;
                        } else if (type == typeof(string)) { 
                            cliType = Connection.CLIType.cli_array_of_string;
                        } else { 
                            throw new CliError("Unsupported array type " + type.Name);
                        }               
                    } else { 
                        throw new CliError("Unsupported field type " + type.Name);
                    }                   
                    types[n++] = cliType;
                }
            }
        }
    }   

    internal void writeColumnDefs(ComBuffer buf) { 
        for (int i = 0, n = nColumns; i < n; i++) { 
            buf.putByte((int)types[i]);
            buf.putAsciiz(columns[i].Name);
        }
    }

    internal void writeColumnValues(ComBuffer buf, Object obj) { 
        int i, j, n, len;
        for (i = 0, n = nColumns; i < n; i++) 
        { 
            switch (types[i]) 
            { 
                case Connection.CLIType.cli_int1:
                    buf.putByte((byte)columns[i].GetValue(obj));
                    break;
                case Connection.CLIType.cli_int2:
                    buf.putShort((short)columns[i].GetValue(obj));
                    break;
                case Connection.CLIType.cli_int4:
                    buf.putInt((int)columns[i].GetValue(obj));
                    break;
                case Connection.CLIType.cli_int8:
                    buf.putLong((long)columns[i].GetValue(obj));
                    break;
                case Connection.CLIType.cli_real4:
                    buf.putFloat((float)columns[i].GetValue(obj));
                    break;
                case Connection.CLIType.cli_real8:
                    buf.putDouble((double)columns[i].GetValue(obj));
                    break;
                case Connection.CLIType.cli_bool:
                    buf.putByte((bool)columns[i].GetValue(obj) ? 1 : 0);
                    break;
                case Connection.CLIType.cli_oid:
                    Reference r = (Reference)columns[i].GetValue(obj);
                    buf.putInt(r != null ? r.oid : 0);
                    break;
                case Connection.CLIType.cli_rectangle:
                    Rectangle rect = (Rectangle)columns[i].GetValue(obj);
                    if (rect == null) 
                    { 
                        rect = new Rectangle();
                    }
                    buf.putRectangle(rect);
                    break;
                case Connection.CLIType.cli_asciiz:
                    buf.putString((string)columns[i].GetValue(obj)); 
                    break;
                case Connection.CLIType.cli_datetime:
                    buf.putInt((int)(((DateTime)columns[i].GetValue(obj)).Ticks / 1000000));
                    break;
                case Connection.CLIType.cli_array_of_int1:
                    buf.putByteArray((byte[])columns[i].GetValue(obj));
                    break;
                case Connection.CLIType.cli_array_of_int2:
                { 
                    short[] arr = (short[])columns[i].GetValue(obj);
                    len = arr == null ? 0 : arr.Length;
                    buf.putInt(len);
                    for (j = 0; j < len; j++) 
                    { 
                        buf.putShort(arr[j]);
                    }
                    break;
                }
                case Connection.CLIType.cli_array_of_int4:
                { 
                    int[] arr = (int[])columns[i].GetValue(obj);
                    len = arr == null ? 0 : arr.Length;
                    buf.putInt(len);
                    for (j = 0; j < len; j++) 
                    { 
                        buf.putInt(arr[j]);
                    }
                    break;
                }
                case Connection.CLIType.cli_array_of_int8:
                { 
                    long[] arr = (long[])columns[i].GetValue(obj);
                    len = arr == null ? 0 : arr.Length;
                    buf.putInt(len);
                    for (j = 0; j < len; j++) 
                    { 
                        buf.putLong(arr[j]);
                    }
                    break;
                }
                case Connection.CLIType.cli_array_of_real4:
                { 
                    float[] arr = (float[])columns[i].GetValue(obj);
                    len = arr == null ? 0 : arr.Length;
                    buf.putInt(len);
                    for (j = 0; j < len; j++) 
                    { 
                        buf.putFloat(arr[j]);
                    }
                    break;
                }
                case Connection.CLIType.cli_array_of_real8:
                { 
                    double[] arr = (double[])columns[i].GetValue(obj);
                    len = arr == null ? 0 : arr.Length;
                    buf.putInt(len);
                    for (j = 0; j < len; j++) 
                    { 
                        buf.putDouble(arr[j]);
                    }
                    break;
                }
                case Connection.CLIType.cli_array_of_bool:
                { 
                    bool[] arr = (bool[])columns[i].GetValue(obj);
                    len = arr == null ? 0 : arr.Length;
                    buf.putInt(len);
                    for (j = 0; j < len; j++) 
                    { 
                        buf.putByte(arr[j] ? 1 : 0);
                    }
                    break;
                }
                case Connection.CLIType.cli_array_of_oid:
                { 
                    Reference[] arr = (Reference[])columns[i].GetValue(obj);
                    len = arr == null ? 0 : arr.Length;
                    buf.putInt(len);
                    for (j = 0; j < len; j++) 
                    { 
                        buf.putInt(arr[j] != null ? arr[j].oid : 0);
                    }
                    break;
                }
                case Connection.CLIType.cli_array_of_string:
                { 
                    string[] arr = (string[])columns[i].GetValue(obj);
                    len = arr == null ? 0 : arr.Length;
                    buf.putInt(len);
                    for (j = 0; j < len; j++) 
                    { 
                        buf.putAsciiz(arr[j]);
                    }
                    break;
                }
                case Connection.CLIType.cli_autoincrement:
                    break;
                default:
                    throw new CliError("Unsupported type " + types[i]);
            }
        }
    }

    internal object readObject(ComBuffer buf) 
    { 
        Object obj = constructor.Invoke(constructorParameters);
        int i, j, n, len;
        for (i = 0, n = nColumns; i < n; i++) 
        { 
            Connection.CLIType type = (Connection.CLIType)buf.getByte();
            if (type != types[i]) 
            { 
                throw new CliError("Unexpected type of column: " + type 
                    + " instead of " + types[i]);
            }
            switch (types[i]) 
            { 
                case Connection.CLIType.cli_int1:
                    columns[i].SetValue(obj, buf.getByte());
                    break;
                case Connection.CLIType.cli_int2:
                    columns[i].SetValue(obj, buf.getShort());
                    break;
                case Connection.CLIType.cli_int4:
                case Connection.CLIType.cli_autoincrement:
                    columns[i].SetValue(obj, buf.getInt());
                    break;
                case Connection.CLIType.cli_int8:
                    columns[i].SetValue(obj, buf.getLong());
                    break;
                case Connection.CLIType.cli_real4:
                    columns[i].SetValue(obj, buf.getFloat());
                    break;
                case Connection.CLIType.cli_real8:
                    columns[i].SetValue(obj, buf.getDouble());
                    break;
                case Connection.CLIType.cli_bool:
                    columns[i].SetValue(obj, buf.getByte() != 0);
                    break;
                case Connection.CLIType.cli_oid:
                { 
                    int oid = buf.getInt();
                    columns[i].SetValue(obj, (oid != 0) ? new Reference(oid) : null);
                    break;
                } 
                case Connection.CLIType.cli_rectangle:
                    columns[i].SetValue(obj, buf.getRectangle());
                    break;
                case Connection.CLIType.cli_asciiz:
                    columns[i].SetValue(obj, buf.getString());
                    break;
                case Connection.CLIType.cli_datetime:
                    columns[i].SetValue(obj, new DateTime(((long)buf.getInt() & 0xFFFFFFFFL)*1000000));
                    break;
                case Connection.CLIType.cli_array_of_int1:
                { 
                    len = buf.getInt();
                    byte[] arr = new byte[len];
                    for (j = 0; j < len; j++) 
                    { 
                        arr[j] = buf.getByte();
                    }
                    columns[i].SetValue(obj, arr);
                    break;
                }
                case Connection.CLIType.cli_array_of_int2:
                { 
                    len = buf.getInt();
                    short[] arr = new short[len];
                    for (j = 0; j < len; j++) 
                    { 
                        arr[j] = buf.getShort();
                    }
                    columns[i].SetValue(obj, arr);
                    break;
                }
                case Connection.CLIType.cli_array_of_int4:
                { 
                    len = buf.getInt();
                    int[] arr = new int[len];
                    for (j = 0; j < len; j++) 
                    { 
                        arr[j] = buf.getInt();
                    }
                    columns[i].SetValue(obj, arr);
                    break;
                }
                case Connection.CLIType.cli_array_of_int8:
                { 
                    len = buf.getInt();
                    long[] arr = new long[len];
                    for (j = 0; j < len; j++) 
                    { 
                        arr[j] = buf.getLong();
                    }
                    columns[i].SetValue(obj, arr);
                    break;
                }
                case Connection.CLIType.cli_array_of_real4:
                { 
                    len = buf.getInt();
                    float[] arr = new float[len];
                    for (j = 0; j < len; j++) 
                    { 
                        arr[j] = buf.getFloat();
                    }
                    columns[i].SetValue(obj, arr);
                    break;
                }
                case Connection.CLIType.cli_array_of_real8:
                { 
                    len = buf.getInt();
                    double[] arr = new double[len];
                    for (j = 0; j < len; j++) 
                    { 
                        arr[j] = buf.getDouble();
                    }
                    columns[i].SetValue(obj, arr);
                    break;
                }
                case Connection.CLIType.cli_array_of_bool:
                { 
                    len = buf.getInt();
                    bool[] arr = new bool[len];
                    for (j = 0; j < len; j++) 
                    { 
                        arr[j] = buf.getByte() != 0;
                    }
                    columns[i].SetValue(obj, arr);
                    break;
                }
                case Connection.CLIType.cli_array_of_oid:
                { 
                    len = buf.getInt();
                    Reference[] arr = new Reference[len];
                    for (j = 0; j < len; j++) 
                    { 
                        int oid = buf.getInt();
                        arr[j] = oid != 0 ? new Reference(oid) : null;
                    }
                    columns[i].SetValue(obj, arr);
                    break;
                }
                case Connection.CLIType.cli_array_of_string:
                { 
                    len = buf.getInt();
                    string[] arr = new string[len];
                    for (j = 0; j < len; j++) 
                    { 
                        arr[j] = buf.getAsciiz();
                    }
                    columns[i].SetValue(obj, arr);
                    break;
                }
                default:
                    throw new CliError("Unsupported type " + types[i]);
            }
        }       
        return obj;
    }
        
}


}


