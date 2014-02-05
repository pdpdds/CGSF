package jnicli;

import java.lang.reflect.*;
import java.util.*;

class ClassDescriptor {
    Class    cls;
    int[]    types;
    Field[]  fields;
    long     td;
    Constructor constructor;

    static final Object[] bypassFlag = { new Boolean(true) };
    static Method         setBypass;
    static final Class[]  constructorProfile = new Class[0];
    static final Object[] constructorParameters = new Object[0];

    static { 
	try { 
	    // setAccessible() method is available only in JDK 1.2
	    Class c = Class.forName("java.lang.reflect.AccessibleObject");
	    Class[] param = { Boolean.TYPE };
	    setBypass = c.getMethod("setAccessible", param);
	} catch(Exception x) {}
    }

    Object newInstance() throws Exception { 
        return constructor.newInstance(constructorParameters);
    }

    ClassDescriptor(Class c) { 
        this.cls = c;
        Vector fieldList = new Vector();
        try { 
            buildFieldList(c, fieldList);
        } catch (Exception x) { 
            throw new CliException(x.getMessage());
        }
        fields = new Field[fieldList.size()];
        fieldList.copyInto(fields);
        types = new int[fields.length];
        for (int i = 0; i < fields.length; i++) { 
            Field f = fields[i];
            if (setBypass != null) { 
                try { 
                    setBypass.invoke(f, bypassFlag);
                } catch(Exception x) {}
            }
            Class type = f.getType();
            int cliType;
            if (type == Byte.TYPE) { 
                cliType = DatabaseJNI.TP_INT1;
            } else if (type == Short.TYPE) { 
                cliType = DatabaseJNI.TP_INT2;
            } else if (type == Integer.TYPE) { 
                cliType = DatabaseJNI.TP_INT4;
            } else if (type == Boolean.TYPE) { 
                cliType = DatabaseJNI.TP_BOOL;
            } else if (type == Character.TYPE) { 
                cliType = DatabaseJNI.TP_CHAR;
            } else if (type == Long.TYPE) { 
                cliType = DatabaseJNI.TP_INT8;
            } else if (type == Float.TYPE) { 
                cliType = DatabaseJNI.TP_REAL4;
            } else if (type == Double.TYPE) { 
                cliType = DatabaseJNI.TP_REAL8;
            } else if (type == String.class) { 
                cliType = DatabaseJNI.TP_STRING;
            } else if (type == Date.class) { 
                cliType = DatabaseJNI.TP_DATE;
            } else if (type.isArray()) { 
                type = type.getComponentType();
                if (type == Boolean.TYPE) { 
                    cliType = DatabaseJNI.TP_BOOL_ARRAY;
                } else if (type == Character.TYPE) { 
                    cliType = DatabaseJNI.TP_CHAR_ARRAY;
                } else if (type == Byte.TYPE) { 
                    cliType = DatabaseJNI.TP_INT1_ARRAY;
                } else if (type == Short.TYPE) { 
                    cliType = DatabaseJNI.TP_INT2_ARRAY;
                } else if (type == Integer.TYPE) { 
                    cliType = DatabaseJNI.TP_INT4_ARRAY;
                } else if (type == Long.TYPE) { 
                    cliType = DatabaseJNI.TP_INT8_ARRAY;
                } else if (type == Float.TYPE) { 
                    cliType = DatabaseJNI.TP_REAL4_ARRAY;
                } else if (type == Double.TYPE) { 
                    cliType = DatabaseJNI.TP_REAL8_ARRAY;
                } else if (type == String.class) { 
                    cliType = DatabaseJNI.TP_STRING_ARRAY;
                } else { 
                    throw new CliException("Unsupported array type " + type.getName());
                }                   
            } else { 
                throw new CliException("Unsupported field type " + type.getName());
            }               
            types[i] = cliType;            
        }
        try { 
            constructor = c.getDeclaredConstructor(constructorProfile);
        } catch(Exception x) {
            throw new CliException("Default constructor is not accessible in class " + c.getName());
        }
        if (setBypass != null) { 
            try {
                setBypass.invoke(constructor, bypassFlag);
            } catch (Exception x) {}
        }
    }

    void buildFieldList(Class c, Vector list) throws Exception {
        Class superclass = c.getSuperclass();
        if (superclass != null) { 
            buildFieldList(superclass, list);
        }
        Field[] flds = c.getDeclaredFields();
        for (int i = 0; i < flds.length; i++) { 
            Field f = flds[i];
            if ((f.getModifiers() & (Modifier.STATIC|Modifier.TRANSIENT)) == 0) {
                list.addElement(f);
            }
        }
    }
    
    String getConstraints() throws Exception { 
        StringBuffer buf = new StringBuffer();
        getConstraints(cls, buf);
        return buf.toString();
    }

    void getConstraints(Class c, StringBuffer buf) throws Exception { 
        Class superclass = c.getSuperclass();
        if (superclass != null) { 
            getConstraints(superclass, buf);
        }
        try { 
            Field f = c.getDeclaredField("CONSTRAINTS");
            if (((f.getModifiers() & Modifier.STATIC) != 0)) {            
                if (setBypass != null) { 
                    try {
                        setBypass.invoke(f, bypassFlag);
                    } catch (Exception x) {}
                }
                String constraints = (String)f.get(null);
                if (constraints != null) { 
                    buf.append(constraints);
                    buf.append(',');
                }
            }
        } catch (NoSuchFieldException x) { 
        }
    }

    String[] getFieldNames() { 
        String names[] = new String[fields.length];
        for (int i = 0; i < names.length; i++) { 
            names[i] = fields[i].getName();
        }
        return names;
    }
}