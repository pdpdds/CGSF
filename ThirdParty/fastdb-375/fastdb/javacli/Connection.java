package javacli;

import java.util.Hashtable;
import java.io.*;
import java.util.Date;
import java.util.HashMap;
import java.lang.reflect.*;
import java.net.*;

/** 
 * FastDB interface responsible for managing connection with FastDB server.
 * Also it implements insert commit, rollback operations 
 */
public class Connection {
   /**
    *  Open onnection with server
    *  @param hostAddress string with server host name
    *  @param hostPort    integer number with server port
    */    
    public void open(String hostAddress, int hostPort) { 
	address = hostAddress;
	port = hostPort;
	try { 
	    socket = null;
	    if (hostAddress.equals("localhost")) { 
		try { 
		    socket = new LocalSocket(hostAddress, hostPort);
		} catch (Throwable x) {}
	    } 
	    if (socket == null) { 
		socket = new Socket(hostAddress, hostPort);	
	    }
	    try { 
		socket.setTcpNoDelay(true);
	    } catch (NoSuchMethodError er) {}
	    
	    in = socket.getInputStream();
	    out = socket.getOutputStream();
	} catch (IOException x) { 
	    throw new CliError("Faield to open database: " + x);
	}
    }

   /**
    * Add package name to the list of registered package. The name of the each registered 
    * package will be appended to the table name when Java class for the database table is 
    located.
    * @param pkName - name of the package
    */
    public void addPackage(String pkName) { 
	if (pkgs == null) { 
	    pkgs = new String[1];
	} else { 
	    String np[] = new String[pkgs.length+1];
	    System.arraycopy(pkgs, 0, np, 0, pkgs.length);
	    pkgs = np;
	}
	pkgs[pkgs.length-1] = pkName;
    }

   /**
    * Close connection with server
    */
    public void close() { 
	if (socket == null) { 
	    throw new CliError("Session is not opened");
	}	
	if (pool != null) { 
	    pool.releaseConnection(this);
	    return;
	}
	ComBuffer buf = new ComBuffer(cli_cmd_close_session);
	send(buf);
	try { 
	    socket.close();
	} catch (IOException x) { 
	    socket = null;
	    throw new CliError("Close failed: " + x);
	}
	socket = null;
    }
    
   /**
    * Create select statement. 
    * @param sql - SubSQL select statement with parameters. Paameters should be
    * started with <code>%</code> character. Each used paramter should be set
    * before execution of the statement.
    */
    public Statement  createStatement(String sql) { 
	if (socket == null) { 
	    throw new CliError("Session is not opened");
	}
	return new Statement(this, sql, ++nStatements);
    }

    /**
     * Commit current transaction
     */
    public void commit() { 
	if (socket == null) { 
	    throw new CliError("Session is not opened");
	}
        sendReceive(cli_cmd_commit, 0);
	
    }
    
    /**
     * Release all locks set by the current transaction
     */
    public void precommit() { 
	if (socket == null) { 
	    throw new CliError("Session is not opened");
	}
        sendReceive(cli_cmd_precommit, 0);
	
    }
    
    /**
     * Rollback curent transaction. All changes made by current transaction are lost.
     */
    public void rollback() { 
	if (socket == null) { 
	    throw new CliError("Session is not opened");
	}
	sendReceive(cli_cmd_abort, 0);
    }

    /**
     * Insert object in the database. There is should be table in the database with
     * name equal to the full class name of the inserted object (comparison is 
     * case sensitive). FastDB will store to the database all non-static and 
     * non-transient fields from the class. All Java primitive types, 
     * <code>java.lang.String</code>, arrays of primitive types or strings, <code>java.util.Date</code>
     * are supported by FastDB. If <code>int</code> field is marked as <code>volatile</code>, it
     * is assumed to be autoincremented field - unique value to this field is assigned automatically 
     * by FastDB. 
     * 
     * @param obj - object to be inserted inthe database
     * @return reference to th inserted object
     */
    public Reference insert(Object obj) { 
	if (socket == null) { 
	    throw new CliError("Session is not opened");
	}
	Class clazz = obj.getClass();
	String className = clazz.getName();
	className = className.substring(className.lastIndexOf('.')+1);
	TableDescriptor tableDesc;
	synchronized (tableHash) { 
	    tableDesc = (TableDescriptor)tableHash.get(className);
	    if (tableDesc == null) { 
		tableDesc = new TableDescriptor(clazz);
		tableHash.put(className, tableDesc);
	    }
	}
	ComBuffer buf = new ComBuffer(cli_cmd_prepare_and_insert);
	buf.putAsciiz("insert into " + className);
	buf.putByte(tableDesc.nColumns);
	tableDesc.writeColumnDefs(buf);
	tableDesc.writeColumnValues(buf, obj);
	send(buf);
	receive(buf, 12);
	int rc = buf.getInt();
	if (rc == cli_ok) { 
	    int rowid = buf.getInt();
	    int oid = buf.getInt();
	    if (tableDesc.autoincrement) { 
		try { 
		    for (int i = tableDesc.nColumns; --i >= 0;) { 
			if (tableDesc.types[i] == Connection.cli_autoincrement) { 
			    tableDesc.columns[i].setInt(obj, rowid);
			}
		    }
		} catch (IllegalAccessException x) { 
		    throw new CliError("Failed to assigned value to autoincremented field: " + x);
		}
	    }
	    return (oid != 0) ? new Reference(oid) : null;
	} else { 
	    throw new CliError("Insert object operation failed with status " + rc);
	}
    }

    /**
     * Create table matching specified class.  Name of the table is equal
     * to class name without any package prefixes. This table will include columns
     * corresponsinf to all non-static and non-transient fields of specified class and base classes
     * of this class. 
     * @param cls Java class for which table should be created.
     * @return <code>true</code> if table sucessfully created, <code>false</code> if table already exists
     * @exception throws CliError exception in case of all other errors (except table already exists)
     */
    public boolean createTable(Class cls) 
    {
        return createTable(cls, null);
    }


    /**
     * Create table matching specified class.  Name of the table is equal
     * to class name without any package prefixes. This table will include columns
     * corresponsinf to all non-static and non-transient fields of specified class and base classes
     * of this class. 
     * @param cls Java class for which table should be created.
     * @param referenceMap map to provide names of referenced tables for reference fields.
     * Key of this map is field name.
     * @return <code>true</code> if table sucessfully created, <code>false</code> if table already exists
     * @exception throws CliError exception in case of all other errors (except table already exists)
     */
    public boolean createTable(Class cls, HashMap referenceMap) 
    {
        int i;
	if (socket == null) { 
	    throw new CliError("Session is not opened");
	}
        Class c;
        int nColumns = 0;
        for (c = cls; c != null; c = c.getSuperclass()) { 
            Field[] classFields = c.getDeclaredFields();
            for (i = 0; i < classFields.length; i++) { 
                if ((classFields[i].getModifiers() & (Modifier.TRANSIENT|Modifier.STATIC)) == 0) { 
                    nColumns += 1;
                }
            }
        }
	ComBuffer buf = new ComBuffer(cli_cmd_create_table);
        String tableName = cls.getName();
        tableName = tableName.substring(tableName.lastIndexOf('.')+1);
	buf.putAsciiz(tableName);
        buf.putByte(nColumns);
        for (c = cls; c != null; c = c.getSuperclass()) { 
            Field[] classFields = c.getDeclaredFields();
            for (i = 0; i < classFields.length; i++) { 
                Field f = classFields[i];
                if ((classFields[i].getModifiers() & (Modifier.TRANSIENT|Modifier.STATIC)) == 0) { 
                    Class type = f.getType();
                    int cliType;
                    String refTableName = "";
                    if (type == Byte.TYPE) { 
                        cliType = Connection.cli_int1;
                    } else if (type == Short.TYPE) { 
                        cliType = Connection.cli_int2;
                    } else if (type == Integer.TYPE) { 
                        if ((f.getModifiers() & Modifier.VOLATILE) != 0) { 
                            cliType = Connection.cli_autoincrement;
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
                        if (referenceMap == null || 
                            (refTableName = (String)referenceMap.get(classFields[i].getName())) == null)
                        {
                            throw new CliError("Rererenced table should be specified for reference field");
                        }
                        cliType = Connection.cli_oid;
                    } else if (type == Rectangle.TYPE) { 
                        cliType = Connection.cli_rectangle;
                    } else if (type == String.class) { 
                        cliType = Connection.cli_asciiz;
                    } else if (type == Date.class) { 
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
                            if (referenceMap == null || 
                                (refTableName = (String)referenceMap.get(classFields[i].getName())) == null)
                            {
                                throw new CliError("Rererenced table should be specified for reference field");
                            }
                        } else if (type == String.class) { 
                            cliType = Connection.cli_array_of_string;
                        } else { 
                            throw new CliError("Unsupported array type " + type.getName());
                        }		    
                    } else { 
                        throw new CliError("Unsupported field type " + type.getName());
                    }		    
                    buf.putByte(cliType);
                    buf.putByte(0);
                    buf.putAsciiz(f.getName());
                    buf.putAsciiz(refTableName);
                    buf.putAsciiz("");
                }
            }
        }
        send(buf);
        receive(buf, 4);
        int rc = buf.getInt();
        if (rc == cli_table_already_exists) { 
            return false;
        } else if (rc < 0) { 
            throw new CliError("Create table failed with status: " + rc);
        }
        return true;
    }

    /**
     * Drop table corresponding to the specified class. 
     * @param cls Java class for which should be created. Name of the table is the same
     * as name of the class without any package prefixes.
     */

    public void dropTable(Class cls) 
    { 
        String tableName = cls.getName();
        tableName = tableName.substring(tableName.lastIndexOf('.')+1);
        dropTable(tableName);
    }

    /**
     * Drop specified table
     * @param tableName name of the table
     */
    public void dropTable(String tableName) 
    { 
        ComBuffer buf = new ComBuffer(cli_cmd_create_table);
        buf.putAsciiz(tableName);
        sendReceive(buf);
    }
       
    /**
     * Add case sensitive index
     * @param tableName name of the table
     * @param fieldName name of the field
     */
    public void addIndex(String tableName, String fieldName) {
        addIndex(tableName, fieldName, false);
    }

    /**
     * Add index
     * @param tableName name of the table
     * @param fieldName name of the field
     * @param caseInsensitive attribute specifying whether index is case insensitive or not
     */
    public void addIndex(String tableName, String fieldName, boolean caseInsensitive) {
        ComBuffer buf = new ComBuffer(cli_cmd_alter_index);
        buf.putAsciiz(tableName);
        buf.putAsciiz(fieldName);
        buf.putByte(caseInsensitive ? cli_indexed|cli_case_insensitive : cli_indexed);
        sendReceive(buf);
    }

    /**
     * Drop index
     * @param tableName name of the table
     * @param fieldName name of the field
     */
    public void dropIndex(String tableName, String fieldName) { 
        ComBuffer buf = new ComBuffer(cli_cmd_alter_index);
        buf.putAsciiz(tableName);
        buf.putAsciiz(fieldName);
        buf.putByte(0);
        sendReceive(buf);
    }
        

    public Connection() {}

    Connection(ConnectionPool pool) { 
        this.pool = pool;
    }

    protected int sendReceive(int cmd, int stmtId) { 
        return sendReceive(new ComBuffer(cmd, stmtId));
    }
    
    protected int sendReceive(ComBuffer buf) { 
        send(buf);
        receive(buf, 4);
        int rc = buf.getInt();
        if (rc < 0) { 
            throw new CliError("Request failed with status " + rc);
        }
        return rc;
    }
        

    protected void send(ComBuffer com) { 
        try { 
            com.end();
            out.write(com.buf, 0, com.pos);
        } catch (IOException x) { 
            throw new CliError("Communication error: " + x);
        }
    }

    protected void receive(ComBuffer com, int len) { 
        com.reset(len);
        int offs = 0;
        try { 
            while (len > 0) { 
                int rc = in.read(com.buf, offs, len);
                if (rc < 0) { 
                    throw new CliError("Failed to read response");
                } 
                len -= rc;
                offs += rc;
            }
        } catch (IOException x) { 
            throw new CliError("Communication error: " + x);
        }
    }
            

    protected Socket         socket;
    protected InputStream    in;  // socket input stream
    protected OutputStream   out; // socket output stream

    protected ConnectionPool pool;
    protected Connection     next;

    protected String         address; 
    protected int            port; 

    protected String[]       pkgs;

    int                      nStatements;

    static Hashtable         tableHash = new Hashtable();

    /* command codes */
    protected static final int cli_cmd_close_session = 0;
    protected static final int cli_cmd_prepare_and_execute = 1;
    protected static final int cli_cmd_execute = 2;
    protected static final int cli_cmd_get_first = 3;
    protected static final int cli_cmd_get_last = 4;
    protected static final int cli_cmd_get_next = 5;
    protected static final int cli_cmd_get_prev = 6;
    protected static final int cli_cmd_free_statement = 7;
    protected static final int cli_cmd_abort = 8;
    protected static final int cli_cmd_commit = 9;
    protected static final int cli_cmd_update = 10;
    protected static final int cli_cmd_remove = 11;
    protected static final int cli_cmd_remove_current = 12;
    protected static final int cli_cmd_insert = 13;
    protected static final int cli_cmd_prepare_and_insert = 14;
    protected static final int cli_cmd_describe_table = 15;
    protected static final int cli_cmd_show_tables = 16;
    protected static final int cli_cmd_precommit = 17;
    protected static final int cli_cmd_skip = 18;
    protected static final int cli_cmd_create_table = 19;
    protected static final int cli_cmd_drop_table = 20;
    protected static final int cli_cmd_alter_index = 21;
    protected static final int cli_cmd_freeze = 22;
    protected static final int cli_cmd_unfreeze = 23;
    protected static final int cli_cmd_seek = 24;
    protected static final int cli_cmd_alter_table = 25;
    
    /* status codes */
    public static final int cli_ok = 0;
    public static final int cli_bad_address = -1;
    public static final int cli_connection_refused = -2;
    public static final int cli_database_not_found = -3;
    public static final int cli_bad_statement = -4;
    public static final int cli_parameter_not_found = -5;
    public static final int cli_unbound_parameter = -6;
    public static final int cli_column_not_found = -7;
    public static final int cli_incompatible_type = -8;
    public static final int cli_network_error = -9;
    public static final int cli_runtime_error = -10;
    public static final int cli_closed_statement = -11;
    public static final int cli_unsupported_type = -12;
    public static final int cli_not_found = -13;
    public static final int cli_not_update_mode = -14;
    public static final int cli_table_not_found = -15;
    public static final int cli_not_all_columns_specified = -16;
    public static final int cli_not_fetched = -17;
    public static final int cli_already_updated = -18;
    public static final int cli_table_already_exists = -19; 
    public static final int cli_not_implemented = -20;       
    public static final int cli_login_failed = -21;
    public static final int cli_empty_parameter = -22;
    public static final int cli_closed_connection = -23;
    
    /* CLI types */
    protected static final int cli_oid =  0;
    protected static final int cli_bool = 1;
    protected static final int cli_int1 = 2;
    protected static final int cli_int2 = 3;
    protected static final int cli_int4 = 4;
    protected static final int cli_int8 = 5;
    protected static final int cli_real4 = 6;
    protected static final int cli_real8 = 7;
    protected static final int cli_decimal = 8;
    protected static final int cli_asciiz = 9;
    protected static final int cli_pasciiz = 10;
    protected static final int cli_cstring = 11;
    protected static final int cli_array_of_oid =  12;
    protected static final int cli_array_of_bool = 13;
    protected static final int cli_array_of_int1 = 14;
    protected static final int cli_array_of_int2 = 15;
    protected static final int cli_array_of_int4 = 16;
    protected static final int cli_array_of_int8 = 17;
    protected static final int cli_array_of_real4 = 18;
    protected static final int cli_array_of_real8 = 19;
    protected static final int cli_array_of_decimal = 20;
    protected static final int cli_array_of_string = 21;
    protected static final int cli_any =           22;
    protected static final int cli_datetime  =     23;
    protected static final int cli_autoincrement  = 24;
    protected static final int cli_rectangle =     25;
    protected static final int cli_undefined =     26;

    /* CLI fields qualifiers */
    protected static final int cli_hashed           = 1;
    protected static final int cli_indexed          = 2;
    protected static final int cli_case_insensitive = 4;
}
    





