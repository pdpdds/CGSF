namespace CSharpCLI { 

using System;

/// <summary>
/// Set of objects returned by select. This class is similar with JDBC ResultSet
/// class and allows navigation though the selected objects in orward or backward 
/// direction 
/// </summary>
///
public class ObjectSet { 
    /// <summary>
    /// Get first fetched object
    /// </summary>
    /// <returns>first object in the set or null if no objects were selected</returns>
    ///
    public object getFirst() { 
        return getObject(Connection.CLICommand.cli_cmd_get_first, 0);
    }

    /// <summary>
    /// Get last fetched object
    /// </summary>
    /// <returns>last object in the set or null if no objects were selected</returns>
    ///
    public object getLast() {
        return getObject(Connection.CLICommand.cli_cmd_get_last, 0);
    }

    /// <summary>
    /// Get next fetched object
    /// </summary>
    /// <returns>next object in the set or null if current object is thelast one in the
    /// set or no objects were selected</returns>
    ///
    public object getNext() { 
        return getObject(Connection.CLICommand.cli_cmd_get_next, 0);
    }

    /// <summary>
    /// Get previous fetched object
    /// </summary>
    /// <returns>previous object in the set or null if the current object is the first
    /// one in the set or no objects were selected</returns>
    ///
    public object getPrev() { 
        return getObject(Connection.CLICommand.cli_cmd_get_prev, 0);
    }


    /// <summary>
    /// Skip specified number of objects. 
    /// <UL>
    /// <LI>if <code>n</code> is positive, then this method has the same effect as
    /// executing getNext() mehod <code>n</code> times.</LI>
    /// <LI>if <code>n</code> is negative, then this method has the same effect of 
    /// executing getPrev() mehod <code>-n</code> times.</LI> 
    /// <LI>if <code>n</code> is zero, this method has no effect</LI>    
    /// </UL>
    /// </summary>
    /// <param name="n">number of objects to be skipped</param>
    /// <returns>object <code>n</code> positions relative to the current position</returns>
    ///
    Object skip(int n) { 
        return getObject(Connection.CLICommand.cli_cmd_skip, n);
    }

    /// <summary>
    /// Get reference to the current object
    /// </summary>
    /// <returns>return reference to the current object or null if no objects were 
    /// selected</returns>
    ///
    public Reference getRef() { 
        return currOid != 0 ? new Reference(currOid) : null;
    }

    /// <summary>
    /// Update the current object in the set. Changes made in the current object 
    /// are saved in the database
    /// </summary>
    ///
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
        ComBuffer buf = new ComBuffer(Connection.CLICommand.cli_cmd_update, stmt.stmtId);
        stmt.tableDesc.writeColumnValues(buf, currObj);
        stmt.con.sendReceive(buf);
    }

    /// <summary>
    /// Remove all selected objects. 
    /// All objects in the object set are removed from the database.
    /// </summary>
    ///
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
        stmt.con.sendReceive(Connection.CLICommand.cli_cmd_remove, stmt.stmtId);
    }

    /// <summary>
    /// Get the number of objects in the object set.
    /// </summary>
    /// <returns>number of the selected objects</returns>
    ///
    public int size() { 
        return nObjects;
    }

    /// <summary>
    /// Close object set. Any followin operation with this object set will raise an
    /// exception.
    /// </summary>
    ///
    public void close() { 
        if (stmt == null) { 
            throw new CliError("ObjectSet was aleady closed");
        }
        stmt = null;
    }

    internal object getObject(Connection.CLICommand cmd, int n) { 
        if (stmt == null) { 
            throw new CliError("ObjectSet was aleady closed");
        }
        if (stmt.con == null) { 
            throw new CliError("Statement was closed");
        }
        ComBuffer buf = new ComBuffer(cmd, stmt.stmtId);
        if (cmd == Connection.CLICommand.cli_cmd_skip) { 
            buf.putInt(n);
        }
        stmt.con.send(buf);
        buf.reset(4);
        stmt.con.receive(buf, 4);
        int len = buf.getInt();
        if (len == (int)Connection.CLIStatus.cli_not_found) { 
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
    
    internal ObjectSet(Statement stmt, int nObjects) { 
        this.stmt = stmt;
        this.nObjects = nObjects;
    }

    internal Statement stmt;
    internal int       nObjects;
    internal int       currOid;
    internal object    currObj;
    internal bool      updated;
}

}
