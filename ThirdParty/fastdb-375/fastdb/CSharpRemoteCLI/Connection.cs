namespace CSharpCLI { 

using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Collections;
using System.Reflection;

/// <summary> 
/// Gigabase interface responsible for managing connection with GigaBASE server.
/// Also it implements insert commit, rollback operations 
/// </summary>
public class Connection {
    /// <summary>
    /// Open onnection with server
    /// </summary>
    /// <param name="hostAddress">string with server host name</param>
    /// <param name="hostPort">integer number with server port</param>
    ///   
    public void open(String hostAddress, int hostPort) { 
	address = hostAddress;
	port = hostPort;
	socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream,
			    ProtocolType.Tcp);
	foreach (IPAddress ip in Dns.Resolve(hostAddress).AddressList) 
	{ 
	    try 
	    {
		socket.Connect(new IPEndPoint(ip, hostPort));	
		break;
	    } 
	    catch (SocketException) {}
	}
	socket.SetSocketOption(SocketOptionLevel.Tcp, SocketOptionName.NoDelay, 1);
    }

    /// <summary>
    /// Add package name to the list of registered package. The name of the each registered 
    /// package will be appended to the table name when Java class for the database table is 
    /// located.
    /// </summary>
    /// <param name="pkName">name of the package</param>
    ///
    public void addPackage(string pkName) { 
	if (pkgs == null) { 
	    pkgs = new string[1];
	} else { 
	    string[] np = new string[pkgs.Length+1];
	    System.Array.Copy(pkgs, 0, np, 0, pkgs.Length);
	    pkgs = np;
	}
	pkgs[pkgs.Length-1] = pkName;
    }

    /// <summary>
    /// Close connection with server
    /// </summary>
    ///
    public void close() { 
	if (socket == null) { 
	    throw new CliError("Session is not opened");
	}	
	if (pool != null) { 
	    pool.releaseConnection(this);
	    return;
	}
	ComBuffer buf = new ComBuffer(CLICommand.cli_cmd_close_session);
	send(buf);
    	socket.Close();
	socket = null;
    }
    
    ///
    /// <summary>
    /// Create select statement. 
    /// </summary>
    /// <param name="sql">SubSQL select statement with parameters. Paameters should be
    /// started with <code>%</code> character. Each used paramter should be set
    /// before execution of the statement</param>
    /// 
    public Statement  createStatement(string sql) { 
	if (socket == null) { 
	    throw new CliError("Session is not opened");
	}
	return new Statement(this, sql, ++nStatements);
    }

    /// <summary>
    /// Commit current transaction
    /// </summary>
    ///
    public void commit() { 
	if (socket == null) { 
	    throw new CliError("Session is not opened");
	}
        sendReceive(CLICommand.cli_cmd_commit, 0);
	
    }
    
    /// <summary>
    /// Release all locks set by the current transaction
    /// </summary>
    ///
    public void precommit() { 
	if (socket == null) { 
	    throw new CliError("Session is not opened");
	}
        sendReceive(CLICommand.cli_cmd_precommit, 0);
	
    }
    
    /// <summary>
    /// Rollback curent transaction.Al changes made by current transaction are lost.
    /// </summary>
    ///
    public void rollback() { 
	if (socket == null) { 
	    throw new CliError("Session is not opened");
	}
	sendReceive(CLICommand.cli_cmd_abort, 0);
    }

    /// <summary>
    /// Insert object in the database. There is should be table in the database with
    /// name equal to the full class name of the inserted object (comparison is 
    /// case sensitive). GigaBASE will store to the database all non-static and 
    /// non-transient fields from the class. All Java primitive types, 
    /// <code>java.lang.string</code>, arrays of primitive types or strings, <code>java.util.Date</code>
    /// are supported by GigaBASE. If <code>int</code> field is marked as <code>volatile</code>, it
    /// is assumed to be autoincremented field - unique value to this field is assigned automatically 
    /// by GigaBASE. 
    /// </summary>
    /// <param name="obj">object to be inserted inthe database</param>
    /// <returns>reference to the inserted object</returns>
    ///
    public Reference insert(object obj) { 
	if (socket == null) { 
	    throw new CliError("Session is not opened");
	}
	Type clazz = obj.GetType();
	string className = clazz.Name;
	TableDescriptor tableDesc;
	lock (tableHash) { 
	    tableDesc = (TableDescriptor)tableHash[className];
	    if (tableDesc == null) { 
		tableDesc = new TableDescriptor(clazz);
		tableHash[className] = tableDesc;
	    }
	}
	ComBuffer buf = new ComBuffer(CLICommand.cli_cmd_prepare_and_insert);
	buf.putAsciiz("insert into " + className);
	buf.putByte(tableDesc.nColumns);
	tableDesc.writeColumnDefs(buf);
	tableDesc.writeColumnValues(buf, obj);
	send(buf);
	receive(buf, 12);
	CLIStatus rc = (CLIStatus)buf.getInt();
	if (rc == CLIStatus.cli_ok) { 
	    int rowid = buf.getInt();
	    int oid = buf.getInt();
	    if (tableDesc.autoincrement) { 
		for (int i = tableDesc.nColumns; --i >= 0;) { 
		    if (tableDesc.types[i] == CLIType.cli_autoincrement) { 
			tableDesc.columns[i].SetValue(obj, rowid);
		    }
		}
	    }
	    return (oid != 0) ? new Reference(oid) : null;
	} else { 
	    throw new CliError("Insert object operation failed with status " + rc);
	}
    }

    /// <summary>
    /// Create table matching specified class.  Name of the table is equal
    /// to class name without any package prefixes. This table will include columns
    /// corresponsinf to all non-static and non-transient fields of specified class and base classes
    /// of this class. 
    /// </summary>
    /// <param name="cls">Java class for which table should be created</param>
    /// <returns><code>true</code> if table sucessfully created, <code>false</code> 
    /// if table already exists, throws <code>CliError</code> exception in case of all other errors</returns>
    ///
    public bool createTable(Type cls) 
    {
        int i;
	if (socket == null) { 
	    throw new CliError("Session is not opened");
	}
        Type c;
        int nColumns = 0;
        for (c = cls; c != null; c = c.BaseType) { 
            FieldInfo[] classFields = c.GetFields(BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.DeclaredOnly);
            for (i = 0; i < classFields.Length; i++) { 
                if (!classFields[i].IsStatic && !classFields[i].IsNotSerialized) { 
                    nColumns += 1;
                }
            }
        }
	ComBuffer buf = new ComBuffer(CLICommand.cli_cmd_create_table);
        String tableName = cls.Name;
	buf.putAsciiz(tableName);
        buf.putByte(nColumns);
        for (c = cls; c != null; c = c.BaseType) { 
            FieldInfo[] classFields = c.GetFields(BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.DeclaredOnly);
            for (i = 0; i < classFields.Length; i++) { 
	        FieldInfo f = classFields[i];
                String refTableName = "";
                String inverseRefName = "";
                if (!f.IsStatic && !f.IsNotSerialized) {                
                    Type type = f.FieldType;
                    Connection.CLIType cliType;
                    if (type == typeof(byte)) { 
                        cliType = Connection.CLIType.cli_int1;
                    } else if (type == typeof(short)) { 
                        cliType = Connection.CLIType.cli_int2;
                    } else if (type == typeof(int)) {                   
                        if (f.GetCustomAttributes(typeof(AutoincrementAttribute), false).Length > 0) {
                            cliType = Connection.CLIType.cli_autoincrement;
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
                        object[] attribs = f.GetCustomAttributes(typeof(ReferencesAttribute), false);
                        if (attribs.Length == 0) {      
                            throw new CliError("Rererences attribute should be specified for rererence field");
                        }
                        ReferencesAttribute ra = (ReferencesAttribute)attribs[0];
                        if (ra.refTableName == null) { 
                            throw new CliError("Name of referenced table should be specified");
                        }
                        refTableName = ra.refTableName;
                        if (ra.inverseRefName != null) { 
                            inverseRefName = ra.inverseRefName;
                        }
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
                            object[] attribs = f.GetCustomAttributes(typeof(ReferencesAttribute), false);
                            if (attribs.Length == 0) {      
                                throw new CliError("Rererences attribute should be specified for rererence field");
                            }
                            ReferencesAttribute ra = (ReferencesAttribute)attribs[0];
                            if (ra.refTableName == null) { 
                                throw new CliError("Name of referenced table should be specified");
                            }
                            refTableName = ra.refTableName;
                            if (ra.inverseRefName != null) { 
                                inverseRefName = ra.inverseRefName;
                            }
                        } else if (type == typeof(string)) { 
                            cliType = Connection.CLIType.cli_array_of_string;
                        } else { 
                            throw new CliError("Unsupported array type " + type.Name);
                        }               
                    } else { 
                        throw new CliError("Unsupported field type " + type.Name);
                    }                   
                    buf.putByte((int)cliType);
                    buf.putByte(0);
                    buf.putAsciiz(f.Name);
                    buf.putAsciiz(refTableName);
                    buf.putAsciiz(inverseRefName);
                }
            }
        }
        send(buf);
        receive(buf, 4);
        int rc = buf.getInt();
        if (rc == (int)CLIStatus.cli_table_already_exists) { 
            return false;
        } else if (rc < 0) { 
            throw new CliError("Create table failed with status: " + rc);
        }
        return true;
    }

    /// <summary>
    /// Drop table corresponding to the specified class. 
    /// </summary>
    /// <param name="cls">Java class for which should be created. Name of the table is the same
    /// as name of the class without any package prefixes</param>
    ///
    public void dropTable(Type cls) 
    { 
        String tableName = cls.Name;
        dropTable(tableName);
    }

    /// <summary>
    /// Drop specified table
    /// </summary>
    /// <param name="tableName">name of the table</param>
    ///
    public void dropTable(String tableName) 
    { 
	ComBuffer buf = new ComBuffer(CLICommand.cli_cmd_drop_table);
	buf.putAsciiz(tableName);
	sendReceive(buf);
    }
       
    /// <summary>
    /// Add case sensitive index
    /// </summary>
    /// <param name="tableName">name of the table</param>
    /// <param name="fieldName">name of the field</param>
    ///
    public void addIndex(String tableName, String fieldName) {
        addIndex(tableName, fieldName, false);
    }

    /// <summary>
    /// Add case sensitive index
    /// </summary>
    /// <param name="tableName">name of the table</param>
    /// <param name="fieldName">name of the field</param>
    /// <param name="caseInsensitive">attribute specifying whether index is case insensitive or not</param>
    ///
    public void addIndex(String tableName, String fieldName, bool caseInsensitive) {
	ComBuffer buf = new ComBuffer(CLICommand.cli_cmd_alter_index);
	buf.putAsciiz(tableName);
	buf.putAsciiz(fieldName);
        buf.putByte(caseInsensitive 
                    ? (int)(CLIFieldQualifier.cli_indexed|CLIFieldQualifier.cli_case_insensitive) 
                    : (int)CLIFieldQualifier.cli_indexed);
	sendReceive(buf);
    }

    /// <summary>
    /// Drop index
    /// </summary>
    /// <param name="tableName">name of the table</param>
    /// <param name="fieldName">name of the field</param>
    ///
    public void dropIndex(String tableName, String fieldName) { 
        ComBuffer buf = new ComBuffer(CLICommand.cli_cmd_alter_index);
        buf.putAsciiz(tableName);
        buf.putAsciiz(fieldName);
        buf.putByte(0);
        sendReceive(buf);
    }
    /// <summary>
    /// Default constructor
    /// </summary>
    public Connection() {}

    internal Connection(ConnectionPool pool) { 
        this.pool = pool;
    }

    internal int sendReceive(CLICommand cmd, int stmtId) { 
        return sendReceive(new ComBuffer(cmd, stmtId));
    }
    
    internal int sendReceive(ComBuffer buf) { 
        send(buf);
        receive(buf, 4);
        int rc = buf.getInt();
        if (rc < 0) { 
            throw new CliError("Request failed with status " + rc);
        }
        return rc;
    }
        

    internal void send(ComBuffer com) { 
        int rc;
        com.end();
        for (int size = com.pos, offs = 0; size != 0; size -= rc, offs += rc) { 
            int chunk = size > MaxSendParcelSize ? MaxSendParcelSize : size;            
            rc = socket.Send(com.buf, offs, chunk, SocketFlags.None);
            if (rc <= 0) { 
                throw new CliError("Socket write failed");
            }
        }                
    }

    internal void receive(ComBuffer com, int len) { 
        com.reset(len);
        int offs = 0;
        while (len > 0) { 
            int rc = socket.Receive(com.buf, offs, len, SocketFlags.None);
            if (rc < 0) { 
                throw new CliError("Failed to read response");
            } 
            len -= rc;
            offs += rc;
        }
    }
            

    internal Socket         socket;

    internal ConnectionPool pool;
    internal Connection     next;

    internal string         address; 
    internal int            port; 

    internal string[]       pkgs;

    internal int            nStatements;

    internal static Hashtable tableHash = new Hashtable();

    /* command codes */
    internal enum CLICommand { 
        cli_cmd_close_session,
        cli_cmd_prepare_and_execute,
        cli_cmd_execute,
        cli_cmd_get_first,
        cli_cmd_get_last,
        cli_cmd_get_next,
        cli_cmd_get_prev,
        cli_cmd_free_statement,
        cli_cmd_abort,
        cli_cmd_commit,
        cli_cmd_update,
        cli_cmd_remove,
        cli_cmd_remove_current,
        cli_cmd_insert,
        cli_cmd_prepare_and_insert,
        cli_cmd_describe_table,
        cli_cmd_show_tables,
        cli_cmd_precommit,
        cli_cmd_skip,
        cli_cmd_create_table,
        cli_cmd_drop_table, 
        cli_cmd_alter_index,
        cli_cmd_freeze,
        cli_cmd_unfreeze,
        cli_cmd_seek,
        cli_cmd_alter_table,
        cli_cmd_last
    }
    
    /* status codes */
    internal enum CLIStatus { 
        cli_ok = 0,
        cli_bad_address = -1,
        cli_connection_refused = -2,
        cli_database_not_found = -3,
        cli_bad_statement = -4,
        cli_parameter_not_found = -5,
        cli_unbound_parameter = -6,
        cli_column_not_found = -7,
        cli_incompatible_type = -8,
        cli_network_error = -9,
        cli_runtime_error = -10,
        cli_bad_descriptor = -11,
        cli_unsupported_type = -12,
        cli_not_found        = -13,
        cli_not_update_mode  = -14,
        cli_table_not_found  = -15,
        cli_not_all_columns_specified = -16,
        cli_not_fetched = -17, 
        cli_already_updated = -18, 
        cli_table_already_exists = -19, 
        cli_not_implemented = -20,       
        cli_login_failed = -21,
        cli_empty_parameter = -22,
        cli_closed_connection = -23
    }
    
    /* CLI types */
    internal enum CLIType {  
        cli_oid,
        cli_bool,
        cli_int1,
        cli_int2,
        cli_int4,
        cli_int8,
        cli_real4,
        cli_real8,
        cli_decimal,
        cli_asciiz,
        cli_pasciiz,
        cli_cstring,
        cli_array_of_oid,
        cli_array_of_bool,
        cli_array_of_int1,
        cli_array_of_int2,
        cli_array_of_int4,
        cli_array_of_int8,
        cli_array_of_real4,
        cli_array_of_real8,
        cli_array_of_decimal,
        cli_array_of_string,
        cli_any,
        cli_datetime,
        cli_autoincrement,
        cli_rectangle,
        cli_undefined
    }

    /* CLI fields qualifiers */
    internal enum CLIFieldQualifier {  
        cli_hashed           = 1, 
        cli_indexed          = 2, 
        cli_case_insensitive = 4
    }
}
    

}



