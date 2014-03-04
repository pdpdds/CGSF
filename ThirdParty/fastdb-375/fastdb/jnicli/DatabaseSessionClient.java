package jnicli;

/**
 * Database session client implementation. This class implements Database interface using
 * proxy object to access remote database server. The role of this class is to keep session context.
 */
public class DatabaseSessionClient implements Database {
    public void open(int accessType, String databaseName, String databasePath, long initSize, int transactionCommitDelay) {
        session = proxy.open(accessType, databaseName, databasePath, initSize, transactionCommitDelay);
    }
    
    public void close() {
        proxy.close(session);
    }

    public Cursor select(Class table, String condition, int flags) {
        return proxy.select(session, table, condition, flags);
    }

    public void update(long oid, Object obj) {
        proxy.update(session, oid, obj);
    }
        
    public long insert(Object obj) {
        return proxy.insert(session, obj);
    }

    public int delete(Class table, String condition) {
        return proxy.delete(session, table, condition);
    }

    public void commit() {
        proxy.commit(session);
    }

    public void rollback() {
        proxy.rollback(session);
    }

    public void lock() {
        proxy.lock(session);
    }

    public void attach() { 
        session = proxy.attach();
    }

    public void detach(int flags) {
        proxy.detach(session, flags);
    }

    public DatabaseSessionClient(DatabaseSession proxy) {
        this.proxy = proxy;
    }
    
    private long session;
    private DatabaseSession proxy;
}
    
