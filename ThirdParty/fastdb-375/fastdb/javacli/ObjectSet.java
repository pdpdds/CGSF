package javacli;

/**
 * Set of objects returned by select. This class is similar with JDBC ResultSet
 * class and allows navigation though the selected objects in orward or backward 
 * direction 
 */
public class ObjectSet { 
    /**
     * Get first fetched object
     * @return first object in the set or null if no objects were selected
     */
    public Object getFirst() { 
        return getObject(Connection.cli_cmd_get_first, 0);
    }

    /**
     * Get last fetched object
     * @return last object in the set or null if no objects were selected
     */
    public Object getLast() {
        return getObject(Connection.cli_cmd_get_last, 0);
    }

    /**
     * Get next fetched object
     * @return next object in the set or null if current object is thelast one in the
     * set or no objects were selected
     */
    public Object getNext() { 
        return getObject(Connection.cli_cmd_get_next, 0);
    }

    /**
     * Get previous fetched object
     * @return previous object in the set or null if the current object is the first
     * one in the set or no objects were selected
     */
    public Object getPrev() { 
        return getObject(Connection.cli_cmd_get_prev, 0);
    }


    /**
     * Skip specified number of objects. 
     * <UL>
     * <LI>if <code>n</code> is positive, then this method has the same effect as
     * executing getNext() mehod <code>n</code> times.
     * <LI>if <code>n</code> is negative, then this method has the same effect of 
     * executing getPrev() mehod <code>-n</code> times.     
     * <LI>if <code>n</code> is zero, this method has no effect     
     * </UL>
     * @param n number of objects to be skipped
     * @return object <code>n</code> positions relative to the current position
     */ 
    Object skip(int n) { 
        return getObject(Connection.cli_cmd_skip, n);
    }

    /**
     * Get reference to the current object
     * @return return reference to the current object or null if no objects were 
     * selected
     */
    public Reference getRef() { 
        return currOid != 0 ? new Reference(currOid) : null;
    }

    /**
     * Update the current object in the set. Changes made in the current object 
     * are saved in the database
     */
    public void update() { 
        if (stmt == null) { 
            throw new CliError("ObjectSet was aleady closed");
        }
        if (stmt.con == null) { 
            throw new CliError("Statement was closed");
        }
        if (currOid == 0) { 
            throw new CliError("No object was selected");
        }
        if (!stmt.forUpdate) { 
            throw new CliError("Updates not allowed");
        }
        if (updated) { 
            throw new CliError("Record was already updated");
        }
        updated = true;
        ComBuffer buf = new ComBuffer(Connection.cli_cmd_update, stmt.stmtId);
        stmt.tableDesc.writeColumnValues(buf, currObj);
        stmt.con.sendReceive(buf);
    }

    /**
     * Remove all selected objects. 
     * All objects in the object set are removed from the database.
     */
    public void removeAll() { 
        if (stmt == null) { 
            throw new CliError("ObjectSet was aleady closed");
        }
        if (stmt.con == null) { 
            throw new CliError("Statement was closed");
        }
        if (!stmt.forUpdate) { 
            throw new CliError("Updates not allowed");
        }
        stmt.con.sendReceive(Connection.cli_cmd_remove, stmt.stmtId);
    }

    /**
     * Get the number of objects in the object set.
     * @return number of the selected objects
     */
    public int size() { 
        return nObjects;
    }

    /**
     * Close object set. Any followin operation with this object set will raise an
     * exception.
     */
    public void close() { 
        if (stmt == null) { 
            throw new CliError("ObjectSet was aleady closed");
        }
        stmt = null;
    }

    protected Object getObject(int cmd, int n) { 
        if (stmt == null) { 
            throw new CliError("ObjectSet was aleady closed");
        }
        if (stmt.con == null) { 
            throw new CliError("Statement was closed");
        }
        ComBuffer buf = new ComBuffer(cmd, stmt.stmtId);
        if (cmd == Connection.cli_cmd_skip) { 
            buf.putInt(n);
        }
        stmt.con.send(buf);
        buf.reset(4);
        stmt.con.receive(buf, 4);
        int len = buf.getInt();
        if (len == Connection.cli_not_found) { 
            return null;
        } else if (len <= 0) { 
            throw new CliError("Failed to get object");
        }
        buf.reset(len-4);
        stmt.con.receive(buf, len-4);
        currOid = buf.getInt();
        if (currOid == 0) { 
            return null;
        }
        updated = false;
        return currObj = stmt.tableDesc.readObject(buf);
    }
    
    protected ObjectSet(Statement stmt, int nObjects) { 
        this.stmt = stmt;
        this.nObjects = nObjects;
    }

    protected Statement stmt;
    protected int       nObjects;
    protected int       currOid;
    protected Object    currObj;
    protected boolean   updated;
}


