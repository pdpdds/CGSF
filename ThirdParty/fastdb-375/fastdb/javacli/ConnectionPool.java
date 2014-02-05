package javacli;

/** 
 * Perform connection pooling
 */
public class ConnectionPool {
   /**
    *  Make new pooled connection. If there is unused connection to this host
    *  with the same user name and password
    *  @param hostAddress string with server host name
    *  @param hostPort    integer number with server port
    */
    public synchronized Connection newConnection(String hostAddress, int hostPort) 
    {
        Connection con = new Connection(this);
        con.open(hostAddress, hostPort);
        return con;
    }
    
   /**
    * Return conection to the pool
    *  @param conxn released connection
    */
    public synchronized void releaseConnection(Connection conxn) {
        conxn.next = connectionChain;
        connectionChain = conxn;
        conxn.commit();
    }

   /**
    * Physically close all opened connections
    */
    public synchronized void close() { 
        for (Connection conxn = connectionChain; conxn != null; conxn = conxn.next) {
             conxn.pool = null;
             conxn.close();
        }
        connectionChain = null;
    }

    protected Connection connectionChain;
}



