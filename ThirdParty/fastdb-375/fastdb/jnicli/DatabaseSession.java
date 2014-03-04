package jnicli;

/**
 * Database session interface. This interface is used for communication with remote database server using RMI.
 */
public interface DatabaseSession { 
    /**
     * Open database
     * @param accessType database access type: one of READ_ONLY, READ_WRITE, CONCURRENT_READ or CONCURRENT_UPDATE
     * @param databaseName database name
     * @param databasePath path to the database file
     * @param initSize initial size of database 
     * @param transactionCommitDelay transaction commit delay (specify 0 to disable)
     * @return database session identifier
     */
    long open(int accessType, String databaseName, String databasePath, long initSize, int transactionCommitDelay);

    
    /**
     * Close database
     * @param session session identifier
     */
    void close(long session);

    /**
     * Select objects from table matching condition
     * @param session session identifier
     * @param table class corresponding to the table
     * @param condition SubSQL condition (to select all records pass empty string)
     * @param flags combination of FOR_UPDATE and AUTOCOMMIT flags
     * @return cursor for selected objects
     */
    Cursor select(long session, Class table, String condition, int flags);
    

    /**
     * Update object with soecified OID
     * @param session session identifier
     * @param oid OID of updated object
     * @param obj updated object
     */
    void update(long session, long oid, Object obj);

    /**
     * Insert new object in the database. Object will be inserted in the table with the same name
     * as object class
     * @param session session identifier
     * @param obj inserted object
     * @return OID of inserted object
     */
    long insert(long session, Object obj);

    /**
     * Delete all objects from the table matching specified condition
     * @param session session identifier
     * @param table class corresponding to the table
     * @param condition SubSQL condition (to delete all records pass empty string)
     * @return number of deleted objects
     */
    int delete(long session, Class table, String condition);

    /**
     * Commit current transaction. 
     * Transaction is implicitly started when any database operation is perfromed.
     * @param session session identifier
     */
    void commit(long session);

    /**
     * Commit current transaction
     * Transaction is implicitly started when any database operation is perfromed.
     * @param session session identifier
     */
    void rollback(long session);

    /**
     * Lock database in exclusive mode.
     * Lock is automatically released when transaction is committed or aborted
     * @param session session identifier
     */
    void lock(long session);

    /**
     * Attach thread to the database. All threads except the one opened the database should
     * invoke this method before start working with database
     * @return database session identifier
     */
    long attach();

    /**
     * Detach thread to the database. All threads except the one opened the database should
     * invoke this method before start working with database
     * @param flags combination of CLI_DETACH_COMMIT and CLI_DETACH_DESTROY_CONTEXT flags
     * @param session session identifier
     */
    void detach(long session, int flags);
}