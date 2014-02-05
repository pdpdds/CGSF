namespace CSharpCLI { 

/// <summary>
/// Perform connection pooling
/// </summary>
public class ConnectionPool {
    /// <summary>
    /// Make new pooled connection. If there is unused connection to this host
    /// with the same user name and password
    /// </summary>
    /// <param name="hostAddress">string with server host name</param>
    /// <param name="hostPort">integer number with server port</param>
    ///
    public Connection newConnection(string hostAddress, int hostPort)
    {
        Connection con = new Connection(this);
        con.open(hostAddress, hostPort);
        return con;
    }
    
    /// <summary>
    /// Return conection to the pool
    /// </summary>
    /// <param name="conxn">released connection</param>
    ///
    public void releaseConnection(Connection conxn) {
        lock (this) 
        { 
            conxn.next = connectionChain;
            connectionChain = conxn;
            conxn.commit();
        }
    }

    /// <summary>
    /// Physically close all opened connections
    /// </summary>
    /// 
    public void close() { 
        lock (this) 
        { 
            for (Connection conxn = connectionChain; conxn != null; conxn = conxn.next) 
            {
                conxn.pool = null;
                conxn.close();
            }
            connectionChain = null;
        }
    }

    internal Connection connectionChain;
}


}





