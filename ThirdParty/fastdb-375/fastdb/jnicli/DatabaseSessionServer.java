package jnicli;

/**
 * Database session client implementation. Server side implementation of DatabaseSession interface.
 * It redirect methods to DatabaseJNI class.
 */
public class DatabaseSessionServer implements DatabaseSession {
    public long open(int accessType, String databaseName, String databasePath, long initSize, int transactionCommitDelay) {
        jni.open(accessType, databaseName, databasePath, initSize, transactionCommitDelay);
        return jni.getThreadContext();
    }
    
    public void close(long session) {
        jni.setThreadContext(session);
        jni.close();
    }

    public Cursor select(long session, Class table, String condition, int flags) {
        jni.setThreadContext(session);
        return jni.select(table, condition, flags);
    }

    public void update(long session, long oid, Object obj) {
        jni.setThreadContext(session);
        jni.update(oid, obj);
    }
        
    public long insert(long session, Object obj) {
        jni.setThreadContext(session);
        return jni.insert(obj);
    }

    public int delete(long session, Class table, String condition) {
        jni.setThreadContext(session);
        return jni.delete(table, condition);
    }

    public void commit(long session) {
        jni.setThreadContext(session);
        jni.commit();
    }

    public void rollback(long session) {
        jni.setThreadContext(session);
        jni.rollback();
    }

    public void lock(long session) {
        jni.setThreadContext(session);
        jni.lock();
    }

    public long attach() { 
        jni.attach();
        return jni.getThreadContext();
    }

    public void detach(long session, int flags) {
        jni.setThreadContext(session);
        jni.detach(flags);
    }

    public DatabaseSessionServer() { 
        jni = new DatabaseJNI(true);
    }

    private DatabaseJNI jni;
}
    
