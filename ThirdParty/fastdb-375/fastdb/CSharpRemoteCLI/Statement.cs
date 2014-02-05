namespace CSharpCLI { 

using System;
using System.Reflection;

/// <summary>
/// Statement class is used to prepare and execute selct statement
/// </summary>
///
public class Statement { 
    /// <summary>
    /// Cleanup unreferenced statement
    /// </summary>
    ///
    public void finalize() { 
        if (con != null) {
            close();
        }
    }

    /// <summary> 
    /// Close the statement. This method release all resource assoiated with statement
    /// at client and server. f close method will not be called, cleanup still
    /// will be performed later when garbage collector call finilize method of this
    /// object 
    /// </summary>
    ///
    public void close() {
        if (con == null) {
            throw new CliError("Statement already closed");
        }
        ComBuffer buf = new ComBuffer(Connection.CLICommand.cli_cmd_free_statement, stmtId);
        con.send(buf);
        con = null;
    }

    internal class Parameter { 
        internal Parameter          next;
        internal string             name;       
        internal Connection.CLIType type;
        internal int                ivalue;
        internal long               lvalue;
        internal float              fvalue;
        internal double             dvalue;
        internal string             svalue;
        internal Rectangle          rvalue;

        internal Parameter(string name) { 
            this.name = name;
            type = Connection.CLIType.cli_undefined;
        }
    }

    /// <summary>
    /// Set bool parameter
    /// </summary>
    /// <param name="name">name of the parameter started with <code>%</code> character</param>
    /// <param name="value">value of the parameter</param>
    ///
    public void setBool(string name, bool value) { 
        Parameter p = getParam(name);
        p.ivalue = value ? 1 : 0;
        p.type = Connection.CLIType.cli_bool;
    }

    /// <summary>
    /// Set byte parameter
    /// </summary>
    /// <param name="name">name of the parameter started with <code>%</code> character</param>
    /// <param name="value">value of the parameter</param>
    ///
    public void setByte(string name, byte value) { 
        Parameter p = getParam(name);
        p.ivalue = value;
        p.type = Connection.CLIType.cli_int4;
    }

    /// <summary>
    /// Set short parameter
    /// </summary>
    /// <param name="name">name of the parameter started with <code>%</code> character</param>
    /// <param name="value">value of the parameter</param>
    ///
    public void setShort(string name, short value) { 
        Parameter p = getParam(name);
        p.ivalue = value;
        p.type = Connection.CLIType.cli_int4;
    }

    /// <summary>
    /// Set integer parameter
    /// </summary>
    /// <param name="name">name of the parameter started with <code>%</code> character</param>
    /// <param name="value">value of the parameter</param>
    ///
    public void setInt(string name, int value) { 
        Parameter p = getParam(name);
        p.ivalue = value;
        p.type = Connection.CLIType.cli_int4;
    }

    /// <summary>
    /// Set long parameter
    /// </summary>
    /// <param name="name">name of the parameter started with <code>%</code> character</param>
    /// <param name="value">value of the parameter</param>
    ///
    public void setLong(string name, long value) { 
        Parameter p = getParam(name);
        p.lvalue = value;
        p.type = Connection.CLIType.cli_int8;
    }

    /// <summary>
    /// Set double parameter
    /// <param name="name">name of the parameter started with <code>%</code> character</param>
    /// <param name="value">value of the parameter</param>
    /// </summary>
    ///
    public void setDouble(string name, double value) { 
        Parameter p = getParam(name);
        p.dvalue = value;
        p.type = Connection.CLIType.cli_real8;
    }

    /// <summary>
    /// Set float parameter
    /// </summary>
    /// <param name="name">name of the parameter started with <code>%</code> character</param>
    /// <param name="value">value of the parameter</param>
    ///
    public void setFloat(string name, float value) { 
        Parameter p = getParam(name);
        p.fvalue = value;
        p.type = Connection.CLIType.cli_real4;
    }

    /// <summary>
    /// Set string parameter
    /// </summary>
    /// <param name="name">name of the parameter started with <code>%</code> character</param>
    /// <param name="value">value of the parameter</param>
    ///
    public void setString(string name, string value) { 
        Parameter p = getParam(name);
        p.svalue = value;
        p.type = Connection.CLIType.cli_asciiz;
    }

    /// <summary>
    /// Set reference parameter
    /// </summary>
    /// <param name="name">name of the parameter started with <code>%</code> character</param>
    /// <param name="value">value of the parameter, <code>null</code> means null reference</param>
    ///
    public void setRef(string name, Reference value) { 
        Parameter p = getParam(name);
        p.ivalue = value != null ? value.oid : 0;
        p.type = Connection.CLIType.cli_oid;
    }

    /// <summary>
    /// Set rectangle parameter
    /// </summary>
    /// <param name="name">name of the parameter started with <code>%</code> character</param>
    /// <param name="rect">value of the parameter</param>
    ///
    public void setRectangle(string name, Rectangle rect) { 
        Parameter p = getParam(name);
        p.rvalue = rect;
        p.type = Connection.CLIType.cli_rectangle;
    }


    /// <summary>
    /// Prepare (if needed) and execute select statement
    /// </summary>
    /// <returns>object set with the selected objects</returns>
    ///
    public ObjectSet fetch() {
        return fetch(false);
    }

    /// <summary>
    /// Prepare (if needed) and execute select statement
    /// Only object set returned by the select for updated statement allows
    /// update and deletion of the objects.
    /// </summary>
    /// <param name="forUpdate">if cursor is opened in for update mode</param>
    /// <returns>object set with the selected objects</returns>
    ///
    public ObjectSet fetch(bool forUpdate) {
        int i, n;
        ComBuffer buf;
        if (!prepared) { 
            buf = new ComBuffer(Connection.CLICommand.cli_cmd_prepare_and_execute, stmtId);
            n = tableDesc.nColumns;
            buf.putByte(nParams);
            buf.putByte(n);
            int len = stmtLen;
            bool addNull = false;
            if (len == 0 || stmt[len-1] != 0) { 
                addNull = true;
                len += nParams;
                buf.putShort(len+1);
            } else { 
                len += nParams;
                buf.putShort(len);
            }
            i = 0;
            Parameter p = parameters;
            do { 
                byte ch = stmt[i++];
                buf.putByte(ch);
                len -= 1;
                if (ch == '\0') { 
                    if (len != 0) { 
                        if (p.type == Connection.CLIType.cli_undefined) { 
                            throw new CliError("Unbound parameter " + p.name);
                        }
                        buf.putByte((int)p.type);
                        p = p.next;
                        len -= 1;
                    }
                }
            } while (len != 0);
            if (addNull) { 
                buf.putByte('\0');              
            }
            tableDesc.writeColumnDefs(buf);
        } else { // statement was already prepared
            buf = new ComBuffer(Connection.CLICommand.cli_cmd_execute, stmtId);
        }
        this.forUpdate = forUpdate;
        buf.putByte(forUpdate ? 1 : 0);
        for (Parameter p = parameters; p != null; p = p.next) { 
            switch (p.type) { 
              case Connection.CLIType.cli_oid:
              case Connection.CLIType.cli_int4:
                buf.putInt(p.ivalue);
                break;
              case Connection.CLIType.cli_int1:
              case Connection.CLIType.cli_bool:
                buf.putByte((byte)p.ivalue);
                break;
              case Connection.CLIType.cli_int2:
                buf.putShort((short)p.ivalue);
                break;
              case Connection.CLIType.cli_int8:
                buf.putLong(p.lvalue);
                break;
              case Connection.CLIType.cli_real4:
                buf.putFloat(p.fvalue);
                break;            
              case Connection.CLIType.cli_real8:
                buf.putDouble(p.dvalue);
                break;            
              case Connection.CLIType.cli_asciiz:
                buf.putAsciiz(p.svalue);
                break;
            }
        }
        prepared = true;
        return new ObjectSet(this, con.sendReceive(buf));
    }

    static bool FindTypeByName(Type t, object name) 
    { 
        return t.Name.Equals(name);
    }

    internal Statement(Connection con, string sql, int stmtId) {
        this.stmtId = stmtId;
        int src = 0, dst = 0, len = sql.Length;
        int p = sql.IndexOf("from");
        if (p < 0 && (p  = sql.IndexOf("FROM")) < 0) { 
            throw new CliError("Bad statment: SELECT FROM expected");
        }
        p += 5;
        while (p < len && sql[p] == ' ') {
            p += 1;
        }
        int q = p;
        while (++q < len && sql[q] != ' ');
        if (p+1 == q) { 
            throw new CliError("Bad statment: table name expected after FROM");
        }
        string tableName = sql.Substring(p, q-p);
        lock (Connection.tableHash) { 
            tableDesc = (TableDescriptor)Connection.tableHash[tableName];
            if (tableDesc == null) { 
                Type tableClass = Type.GetType(tableName);
                if (tableClass == null) { 
                    if (con.pkgs != null) 
                    {
                        foreach (string pkg in con.pkgs) { 
                            tableClass = Type.GetType(pkg + '.' + tableName);
                            if (tableClass != null) 
                            { 
                                break;
                            }
                        }
                    }
                    if (tableClass == null) 
                    { 
                        foreach (Assembly ass in AppDomain.CurrentDomain.GetAssemblies()) 
                        { 
                            foreach (Module mod in ass.GetModules()) 
                            { 
                                foreach (Type t in mod.FindTypes(new TypeFilter(FindTypeByName), tableName)) 
                                { 
                                    if (tableClass != null) 
                                    { 
                                        throw new CliError("Class " + tableName + " exists in more than one scope");
                                    }
                                    tableClass = t;
                                }
                            }
                        }
                    }
                    if (tableClass == null) { 
                        throw new CliError("Class " + tableName + " not found");
                    }
                }
                tableDesc = new TableDescriptor(tableClass);
                Connection.tableHash[tableName] = tableDesc;
            }
        }
        byte[] buf = new byte[len];
        while (src < len) { 
            char ch = sql[src];
            if (ch == '\'') {
                do {
                    do {
                        buf[dst++] = (byte)sql[src++];
                        if (src == len) { 
                            throw new CliError("Unterminated string constant in query");
                        }
                    } while (sql[src] != '\''); 
                    buf[dst++] = (byte)'\'';
                } while (++src < len && sql[src] == '\'');
            } else if (ch == '%') { 
                int begin = src;
                do {
                    if (++src == len) { 
                        break;
                    }
                    ch = sql[src];
                } while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
                         || (ch >= '0' && ch <= '9') || ch == '_');
                if (ch == '%') {
                    throw new CliError("Invalid parameter name");
                }
                Parameter param = new Parameter(sql.Substring(begin, src-begin));
                if (lastParam == null) { 
                    parameters = param;
                } else { 
                    lastParam.next = param;
                }                   
                lastParam = param;
                nParams += 1;
                buf[dst++] = (byte)'\0';
            } else { 
                buf[dst++]= (byte)sql[src++];
            }
        }
        stmt = buf;
        stmtLen = dst;
        this.con = con;
    }
                
    internal Parameter getParam(string name) {
        for (Parameter p = parameters; p != null; p = p.next) {
            if (p.name.Equals(name)) { 
                return p;
            }
        }
        throw new CliError("No such parameter");
    }

    internal byte[]          stmt;
    internal int             stmtId;
    internal int             stmtLen;
    internal Connection      con;
    internal Parameter       parameters;
    internal Parameter       lastParam;
    internal int             nParams;
    internal bool            prepared;    
    internal bool            forUpdate;
    internal TableDescriptor tableDesc;
}

    
}