package javacli;

/**
 * Class representing references between persistent objects
 */
public class Reference { 
    int oid;
   
    /**
     * Get object identifier
     */
    public int getOid() { 
        return oid;
    }

    /**
     * Comparison method for references
     */
    public boolean equals(Object obj) { 
        return obj instanceof Reference && ((Reference)obj).oid == oid;
    }

    /**
     * Hash code method for references
     */
    public int hashCode() { 
        return oid;
    }

    static Class TYPE;

    static { 
        try { 
            TYPE = Class.forName("javacli.Reference");
        } catch (ClassNotFoundException x) { 
        }
    }

    public String toString() { 
        return "$" + oid;
    }

    public Reference(int oid) { 
        this.oid = oid;
    }
}
