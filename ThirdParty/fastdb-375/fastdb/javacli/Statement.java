package javacli;

import java.lang.reflect.*;

/**
 * Statement class is used to prepare and execute select statement
 */
public class Statement { 
    /**
     * Cleanup unreferenced statement
     */
    public void finalize() { 
	if (con != null) {
	    close();
	}
    }

    /** 
     * Close the statement. This method release all resource assoiated with statement
     * at client and server. f close method will not be called, cleanup still
     * will be performed later when garbage collector call finilize method of this
     * object 
     */
    public void close() {
	if (con == null) {
	    throw new CliError("Statement already closed");
	}
	ComBuffer buf = new ComBuffer(Connection.cli_cmd_free_statement, stmtId);
	con.send(buf);
	con = null;
    }

    static class Parameter { 
	Parameter next;
	String    name;	
	int       type;
	int       ivalue;
	long      lvalue;
	float     fvalue;
	double    dvalue;
	String    svalue;
	Rectangle rvalue;

	Parameter(String name) { 
	    this.name = name;
	    type = Connection.cli_undefined;
	}
    }

    /**
     * Set boolean parameter
     * @param name - name of the parameter started with <code>%</code> character
     * @param value - value of the parameter
     */
    public void setBool(String name, boolean value) { 
	Parameter p = getParam(name);
	p.ivalue = value ? 1 : 0;
	p.type = Connection.cli_bool;
    }

    /**
     * Set byte parameter
     * @param name - name of the parameter started with <code>%</code> character
     * @param value - value of the parameter
     */
    public void setByte(String name, byte value) { 
	Parameter p = getParam(name);
	p.ivalue = value;
	p.type = Connection.cli_int4;
    }

    /**
     * Set short parameter
     * @param name - name of the parameter started with <code>%</code> character
     * @param value - value of the parameter
     */
    public void setShort(String name, short value) { 
	Parameter p = getParam(name);
	p.ivalue = value;
	p.type = Connection.cli_int4;
    }

    /**
     * Set integer parameter
     * @param name - name of the parameter started with <code>%</code> character
     * @param value - value of the parameter
     */
    public void setInt(String name, int value) { 
	Parameter p = getParam(name);
	p.ivalue = value;
	p.type = Connection.cli_int4;
    }

    /**
     * Set long parameter
     * @param name - name of the parameter started with <code>%</code> character
     * @param value - value of the parameter
     */
    public void setLong(String name, long value) { 
	Parameter p = getParam(name);
	p.lvalue = value;
	p.type = Connection.cli_int8;
    }

    /**
     * Set double parameter
     * @param name - name of the parameter started with <code>%</code> character
     * @param value - value of the parameter
     */
    public void setDouble(String name, double value) { 
	Parameter p = getParam(name);
	p.dvalue = value;
	p.type = Connection.cli_real8;
    }

    /**
     * Set float parameter
     * @param name - name of the parameter started with <code>%</code> character
     * @param value - value of the parameter
     */
    public void setFloat(String name, float value) { 
	Parameter p = getParam(name);
	p.fvalue = value;
	p.type = Connection.cli_real4;
    }

    /**
     * Set string parameter
     * @param name - name of the parameter started with <code>%</code> character
     * @param value - value of the parameter
     */
    public void setString(String name, String value) { 
	Parameter p = getParam(name);
	p.svalue = value;
	p.type = Connection.cli_asciiz;
    }

    /**
     * Set reference parameter
     * @param name - name of the parameter started with <code>%</code> character
     * @param value - value of the parameter, <code>null</code> means null reference
     */
    public void setRef(String name, Reference value) { 
	Parameter p = getParam(name);
	p.ivalue = value != null ? value.oid : 0;
	p.type = Connection.cli_oid;
    }

    /**
     * Set rectangle parameter
     * @param name - name of the parameter started with <code>%</code> character
     * @param rect - value of the parameter
     */
    public void setRectangle(String name, Rectangle rect) { 
	Parameter p = getParam(name);
	p.rvalue = rect;
	p.type = Connection.cli_rectangle;
    }


    /**
     * Prepare (if needed) and execute select statement
     * @return object set with the selected objects
     */
    public ObjectSet fetch() {
	return fetch(false);
    }

    /**
     * Prepare (if needed) and execute select statement
     * Only object set returned by the select for updated statement allows
     * update and deletion of the objects.
     * @param forUpdate - if cursor is opened in for update mode
     * @return object set with the selected objects
     */
    public ObjectSet fetch(boolean forUpdate) {
	int i, n;
	ComBuffer buf;
	if (!prepared) { 
	    buf = new ComBuffer(Connection.cli_cmd_prepare_and_execute, stmtId);
	    n = tableDesc.nColumns;
	    buf.putByte(nParams);
	    buf.putByte(n);
	    int len = stmtLen;
	    boolean addNull = false;
	    if (len == 0 || stmt[len-1] != 0) { 
		addNull = true;
		len += nParams;
		buf.putShort(len+1);
	    } else { 
		len += nParams;
		buf.putShort(len);
	    }
	    i = 0;
	    Parameter p = params;
	    do { 
		byte ch = stmt[i++];
		buf.putByte(ch);
		len -= 1;
		if (ch == '\0') { 
		    if (len != 0) { 
			if (p.type == Connection.cli_undefined) { 
			    throw new CliError("Unbound parameter " + p.name);
			}
			buf.putByte(p.type);
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
	    buf = new ComBuffer(Connection.cli_cmd_execute, stmtId);
	}
	this.forUpdate = forUpdate;
	buf.putByte(forUpdate ? 1 : 0);
	for (Parameter p = params; p != null; p = p.next) { 
	    switch (p.type) { 
	      case Connection.cli_oid:
              case Connection.cli_int4:
                buf.putInt(p.ivalue);
                break;
              case Connection.cli_int1:
              case Connection.cli_bool:
                buf.putByte((byte)p.ivalue);
                break;
              case Connection.cli_int2:
                buf.putShort((short)p.ivalue);
                break;
              case Connection.cli_int8:
                buf.putLong(p.lvalue);
                break;
              case Connection.cli_real4:
                buf.putFloat(p.fvalue);
                break;            
              case Connection.cli_real8:
                buf.putDouble(p.dvalue);
                break;            
              case Connection.cli_asciiz:
                buf.putAsciiz(p.svalue);
                break;
            }
        }
        prepared = true;
        return new ObjectSet(this, con.sendReceive(buf));
    }

    protected Statement(Connection con, String sql, int stmtId) {
        this.stmtId = stmtId;
        int src = 0, dst = 0, len = sql.length();
        int p = sql.indexOf("from");
        if (p < 0 && (p  = sql.indexOf("FROM")) < 0) { 
            throw new CliError("Bad statment: SELECT FROM expected");
        }
        p += 5;
        while (p < len && sql.charAt(p) == ' ') {
            p += 1;
        }
        int q = p;
        while (++q < len && sql.charAt(q) != ' ');
        if (p+1 == q) { 
            throw new CliError("Bad statment: table name expected after FROM");
        }
        String tableName = sql.substring(p, q);
        synchronized (Connection.tableHash) { 
            tableDesc = (TableDescriptor)Connection.tableHash.get(tableName);
            if (tableDesc == null) { 
                Class tableClass = null;
                try { 
                    tableClass = Class.forName(tableName);
                } catch (ClassNotFoundException x) { 
                    if (con.pkgs != null) {
                        String pkgs[] = con.pkgs;
                        for (int i = pkgs.length; --i >= 0;) { 
                            try { 
                                tableClass = Class.forName(pkgs[i] + '.' + tableName);
                                break;
                            } catch (ClassNotFoundException x1) {}
                        }
                    }           
                    if (tableClass == null) { 
                        Package pks[] = Package.getPackages();
                        for (int i = pks.length; --i >= 0;) { 
                            try { 
                                tableClass = Class.forName(pks[i].getName() + '.' + tableName);
                                break;
                            } catch (ClassNotFoundException x1) {}
                        }
                    }
                    if (tableClass == null) { 
                        throw new CliError("Class " + tableName + " not found");
                    }
                }
                tableDesc = new TableDescriptor(tableClass);
                Connection.tableHash.put(tableName, tableDesc);
            }
        }
        byte buf[] = new byte[len];
        while (src < len) { 
            char ch = sql.charAt(src);
            if (ch == '\'') {
                do {
                    do {
                        buf[dst++] = (byte)sql.charAt(src++);
                        if (src == len) { 
                            throw new CliError("Unterminated string constant in query");
                        }
                    } while (sql.charAt(src) != '\'');  
                    buf[dst++] = (byte)'\'';
                } while (++src < len && sql.charAt(src) == '\'');
            } else if (ch == '%') { 
                int begin = src;
                do {
                    if (++src == len) { 
                        break;
                    }
                    ch = sql.charAt(src);
                } while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
                         || (ch >= '0' && ch <= '9') || ch == '_');
                if (ch == '%') {
                    throw new CliError("Invalid parameter name");
                }
                Parameter param = new Parameter(sql.substring(begin, src));
                if (lastParam == null) { 
                    params = param;
                } else { 
                    lastParam.next = param;
                }                   
                lastParam = param;
                nParams += 1;
                buf[dst++] = (byte)'\0';
            } else { 
                buf[dst++]= (byte)sql.charAt(src++);
            }
        }
        stmt = buf;
        stmtLen = dst;
        this.con = con;
    }
                
    protected Parameter getParam(String name) {
        for (Parameter p = params; p != null; p = p.next) {
            if (p.name.equals(name)) { 
                return p;
            }
        }
        throw new CliError("No such parameter");
    }

    byte[]          stmt;
    int             stmtId;
    int             stmtLen;
    Connection      con;
    Parameter       params;
    Parameter       lastParam;
    int             nParams;
    Field           columns;
    int             nColumns;
    boolean         prepared;    
    boolean         forUpdate;
    TableDescriptor tableDesc;
}

    
