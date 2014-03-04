namespace CSharpCLI { 

using System;
using System.Reflection;


/// <summary>
/// Class representing references between persistent objects
/// </summary>
///
public class Reference { 
    internal int oid;
   
    /// <summary>
    /// Get object identifier
    /// </summary>
    ///
    public int getOid() { 
        return oid;
    }

    /// <summary>
    /// Comparison method for references
    /// </summary>
    ///
    public bool equals(Object obj) { 
        return obj is Reference && ((Reference)obj).oid == oid;
    }

    /// <summary>
    /// Hash code method for references
    /// </summary>
    ///
    public override int GetHashCode() { 
        return oid;
    }

    /// <summary>
    /// String representation of reference
    /// </summary>
    ///    
    public override string ToString() { 
        return "$" + oid;
    }

    ///
    /// <summary>
    /// Constructor of reference from integer
    /// </summary>
    /// <param name="oid">referenced object identifier</param>
    ///    
    public Reference(int oid) { 
        this.oid = oid;
    }
}

}