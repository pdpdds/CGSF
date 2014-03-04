package jnicli;

/**
 * This class is used as interface to local or remote database (through RMI).
 * In case of local connections you should create instance of DatabaseJNI class directly.
 * In case of remote connection, it is necessary to create instance of DatabaseSessionClient
 * class and path reference to the stub object implementing DatabaseSession interface.<p>
 * Methods of this interface are mostly one-to-one mapped to methods of C++ dbDatabase class.
 * In case of remote application, it should either open or create new database at server using Database.open method,
 * either attach to the existing database using Database.attach method. In case of local application,
 * Database.attach should be used by all application threads except one opened the database.<p>
 * All database access errors are reported by means of CliException. In case of remote database there are
 * also can be exceptions related with server communication.
 */
public interface Database { 
    static final int READ_ONLY         = 0;
    static final int READ_WRITE        = 1;
    static final int CONCURRENT_READ   = 2; 
    static final int CONCURRENT_UPDATE = 3;
    /**
     * Open database
     * @param accessType database access type: one of READ_ONLY, READ_WRITE, CONCURRENT_READ or CONCURRENT_UPDATE
     * @param databaseName database name
     * @param databasePath path to the database file
     * @param initSize initial size of database 
     * @param transactionCommitDelay transaction commit delay (specify 0 to disable)
     */
    void open(int accessType, String databaseName, String databasePath, long initSize, int transactionCommitDelay);
    
    /**
     * Close database
     */
    void close();


    /**
     * Select flag FOR_UPDATE: make it possible to update selected records
     */
    static final int FOR_UPDATE = 1;
    /**
     * Select flag AUTOCOMMIT: automatically commit current transaction after completion of selection
     */    
    static final int AUTOCOMMIT = 2;

    /**
     * Select objects from table matching condition
     * @param table class corresponding to the table
     * @param condition SubSQL condition (to select all records pass empty string)
     * @param flags combination of FOR_UPDATE and AUTOCOMMIT flags
     * @return cursor for selected objects. Cursor can be either incremental, when records are extracted on 
     * demand or prefetched. Prefetched cursor is used for remote database and when AUTOCOMMIT flag is set.
     * Prefetched cursor doesn't allow in place updates.
     */
    Cursor select(Class table, String condition, int flags);
    

    /**
     * Update object with soecified OID
     * @param oid OID of updated object
     * @param obj updated object
     */
    void update(long oid, Object obj);

    /**
     * Insert new object in the database. Object will be inserted in the table with the same name
     * as object class
     * @param obj inserted object
     * @return OID of inserted object
     */
    long insert(Object obj);

    /**
     * Delete all objects from the table matching specified condition
     * @param table class corresponding to the table
     * @param condition SubSQL condition (to delete all records pass empty string)
     * @return number of deleted objects
     */
    int delete(Class table, String condition);

    /**
     * Commit current transaction. 
     * Transaction is implicitly started when any database operation is perfromed.
     */
    void commit();

    /**
     * Commit current transaction
     * Transaction is implicitly started when any database operation is perfromed.
     */
    void rollback();

    /**
     * Lock database in exclusive mode.
     * Lock is automatically released when transaction is committed or aborted
     */
    void lock();

    /**
     * Attach thread to the database. All threads except the one opened the database should
     * invoke this method before start working with database
     */
    void attach();

    static final int CLI_DETACH_COMMIT = 1;
    static final int CLI_DETACH_DESTROY_CONTEXT = 2;
    static final int CLI_DETACH_DEFAULT=CLI_DETACH_COMMIT|CLI_DETACH_DESTROY_CONTEXT;
    /**
     * Detach thread to the database. All threads except the one opened the database should
     * invoke this method before start working with database
     * @param flags combination of CLI_DETACH_COMMIT and CLI_DETACH_DESTROY_CONTEXT flags
     */
    void detach(int flags);
}