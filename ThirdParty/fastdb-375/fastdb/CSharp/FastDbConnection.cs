using System;
using System.Text;
using System.Collections;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace FastDbNet
{
  //-------------------------------------------------------------------------
  public class FastDbConnection: IDisposable {
    public static readonly int DefaultInitDatabaseSize  = 4*1024*1024; // Default initial db size (number of objects)
    public static readonly int DefaultInitIndexSize     = 512*1024;    // Default initial index size (number of objects)
    public static readonly int DefaultExtensionQuantum  = 4*1024*1024; // Quantum of extension of allocated memory
    public static readonly int MaxParallelSearchThreads = 64;          // Maximal number of threads which can be spawned to perform parallel sequentila search
    public static readonly int DefaultDatabasePort      = 6010;
    public static readonly int DefReconnectTimeoutSec   = 120;         // Reconnect timeout seconds

    /// <summary>
    /// Create a FastDb connection (without opening a database).
    /// </summary>
    /// <param name="DatabaseName">Database name</param>
    public FastDbConnection(string DatabaseName) {
      this.dbName = DatabaseName;
      this.dbPath = DatabaseName + ".fdb";
    }

    /// <summary>
    /// Destroy FastDb connection, close the database, and free session resources.
    /// </summary>
    ~FastDbConnection()                  { Dispose(false); }
    
    /// <summary>
    /// Create table given its structure.
    /// </summary>
    /// <param name="TableName">Table name</param>
    /// <param name="fields">Table fields</param>
    /// <returns>Return code (int)CLI.ErrorCode</returns>
    public int CreateTable(string TableName, FastDbFields fields) {
      CLI.CliFieldDescriptor[] flds = (CLI.CliFieldDescriptor[])Array.CreateInstance(typeof(CLI.CliFieldDescriptor), fields.Count);
      //(CLI.CliFieldDescriptor[])aFlds.ToArray(typeof(CLI.CliFieldDescriptor));

      for(int i=0; i<fields.Count; i++) {
        flds[i].type                = fields[i].Type; 
        flds[i].flags               = fields[i].Flags;
        flds[i].name                = Marshal.StringToHGlobalAnsi(fields[i].Name);
        flds[i].refTableName        = Marshal.StringToHGlobalAnsi(fields[i].RefTable);
        flds[i].inverseRefFieldName = Marshal.StringToHGlobalAnsi(fields[i].InvRefField);
      }
      
      int rc = CLI.cli_create_table(session, TableName, fields.Count, flds);

      for(int i=0; i < fields.Count; i++) {
        Marshal.FreeCoTaskMem(flds[i].name);
        Marshal.FreeCoTaskMem(flds[i].refTableName);
        Marshal.FreeCoTaskMem(flds[i].inverseRefFieldName);
      }
      if (rc < 0 && rc != (int)CLI.ErrorCode.cli_table_already_exists) CLI.CliCheck(rc);
      return rc;
    }

    /// <summary>
    /// Name of the database
    /// </summary>
    public string DatabaseName      { get { return dbName; }            set { CheckConnection(false); dbName = value; } }
    /// <summary>
    /// Path to the database file.
    /// </summary>
    public string DatabasePath      { get { return dbPath; }            set { CheckConnection(false); dbPath = value; } }
    /// <summary>
    /// Initial database size.
    /// </summary>
    public int    InitDbSize        { get { return initDbSize; }        set { CheckConnection(false); initDbSize = value; } }
    /// <summary>
    /// Initial database index size.
    /// </summary>
    public int    InitIdxSize       { get { return initIdxSize; }       set { CheckConnection(false); initIdxSize = value; } }
    /// <summary>
    /// Memory extention quantum size
    /// </summary>
    public int    ExtensionQuantum  { get { return extQuantum;    }     set { CheckConnection(false); extQuantum    = value; } }
    /// <summary>
    /// Maximum allowed size of the database file.  0 = unlimited.
    /// </summary>
    public int    FileSizeLimit     { get { return fileSizeLimit; }     set { CheckConnection(false); fileSizeLimit = value; } }
    /// <summary>
    /// Number of attempts to establish connection
    /// </summary>
    public int    MaxConnectRetries { get { return maxConnectRetries; } set { CheckConnection(false); maxConnectRetries = value; } }
    /// <summary>
    /// Timeout in seconds between connection attempts
    /// </summary>
    public int    ReconnectTimeout  { get { return reconnectTimeout;  } set { CheckConnection(false); reconnectTimeout  = value; } } 
    /// <summary>
    /// If true, Open() creates a replicated node.  Defaults to false.
    /// </summary>
    public bool   EnableReplication { get { return enableReplication; } set { CheckConnection(false); enableReplication = value; } } 
    /// <summary>
    /// Trasnaction commit delay (specify 0 to disable).
    /// </summary>
    public uint   TransCommitDelay  { get { return transCommitDelay; }  set { CheckConnection(false); transCommitDelay = value; } } 
    /// <summary>
    /// Node identifier: 0 ... NodeNames.Length  (only relevant for a replicated database).
    /// </summary>
    public int    NodeID            { get { return nodeID; }            set { CheckConnection(false); nodeID = value; } } 
    /// <summary>
    /// Names of the replicated nodes (only relevant for a replicated database).
    /// </summary>
    public string[] NodeNames       { get { return nodeNames; }         set { CheckConnection(false); nodeNames = value; } } 
    /// <summary>
    /// Internal session handle
    /// </summary>
    public int    Session           { get { return session; } }
    /// <summary>
    /// Controls automated calls to Attach()/Detach() methods.  Disabled by default.
    /// </summary>
    public bool   Threaded          { get { return threaded; } set { threaded = value; } } 
    /// <summary>
    /// Attributes used to open database. <seealso cref="CLI.CliOpenAttribute"/>
    /// </summary>
    public CLI.CliOpenAttribute OpenAttributes { 
      get { return openAttributes; } 
      set { CheckConnection(false); openAttributes = value; } 
    }

    /// <summary>
    /// Open local database.
    /// </summary>
    public void Open()                      { this.Open(true, "", 0); }
    public void Open(string Host, int Port) { this.Open(false, Host, Port); }

    /// <summary>
    /// Commit transaction and write changed data to disk.
    /// </summary>
    public void Commit()    { CLI.CliCheck(CLI.cli_commit(session)); }
    /// <summary>
    /// Commit transaction without writing changed data to disk.
    /// </summary>
    public void PreCommit() { CLI.CliCheck(CLI.cli_precommit(session)); }
    /// <summary>
    /// Roolback current transaction.
    /// </summary>
    public void Rollback()  { CLI.CliCheck(CLI.cli_abort(session)); }
    /// <summary>
    /// Close database connection.
    /// </summary>
    public void Close()     
    { 
      for(int i=commands.Count-1; i >= 0; --i)
        ((FastDbCommand)commands[i]).Free();
      CLI.CliCheck(CLI.cli_close(session)); 
      session = -1; 
    }

    /// <summary>
    /// List tables in the database.
    /// </summary>
    /// <returns>A string array of table names</returns>
    public unsafe string[] ListTables() {
      bool dummy = false;
      return ListTables("", ref dummy);
    }

    /// <summary>
    /// Checks if a table exists in the database.
    /// </summary>
    /// <param name="TableName">Name of the table to check for existence</param>
    /// <returns>true - table exists.</returns>
    public bool TableExists(string TableName) {
      bool exists = false;
      ListTables(TableName, ref exists);
      return exists;
    }

    public FastDbFields DescribeTable(string TableName) { 
      return this.DescribeTable(TableName, true); }
    
    /// <summary>
    /// Describes a table given its name.
    /// </summary>
    /// <param name="TableName">Name of the table to describe</param>
    /// <param name="RaiseError">If true, an error check will be performed (default: true).</param>
    /// <returns>A collection of fields fetched from the database's table.</returns>
    public unsafe FastDbFields DescribeTable(string TableName, bool RaiseError) {
      FastDbFields fields = new FastDbFields();
      void* p = null;

      int rc = CLI.cli_describe(session, TableName, ref p);
      if (RaiseError) CLI.CliCheck(rc);
      if (rc > 0) {
        try {
          CLI.CliFieldDescriptor* fld = (CLI.CliFieldDescriptor*)p;
          for(int i=0; i<rc; i++, fld++) {
            Debug.Assert(fld->name != IntPtr.Zero, "Field name is a null pointer!");
            string s  = Marshal.PtrToStringAnsi(fld->name);
            string sr = (fld->refTableName == IntPtr.Zero) ? null : Marshal.PtrToStringAnsi(fld->refTableName);
            string si = (fld->inverseRefFieldName == IntPtr.Zero) ? null : Marshal.PtrToStringAnsi(fld->inverseRefFieldName);
            fields.Add(s, fld->type, fld->flags, sr, si);
          }
        }
        finally {
          CLI.cli_free_memory(session, p);
        }
      }
      return fields;
    }

    /// <summary>
    /// Drop a table from the database
    /// </summary>
    /// <param name="TableName">Name of the table</param>
    public void DropTable(string TableName) {
      CLI.CliCheck(CLI.cli_drop_table(session, TableName));
    }

    /// <summary>
    /// Alter index on a field
    /// </summary>
    /// <param name="TableName">Name of the table</param>
    /// <param name="FieldName">Name of the field</param>
    /// <param name="NewFlags">New index types.</param>
    public void AlterIndex(string TableName, string FieldName, CLI.FieldFlags NewFlags) {
      CLI.CliCheck(CLI.cli_alter_index(session, TableName, FieldName, NewFlags));
    }


    /// <summary>
    /// Create a new SQL command in this connection. <seealso cref="CLI.FastDbCommand"/>
    /// </summary>
    /// <param name="sql">SQL text representing a command</param>
    /// <returns>FastDbCommand object to be used for executing the SQL command</returns>
    public FastDbCommand CreateCommand(string sql) {
      lock(typeof(FastDbConnection)) {
        int n = commands.Add(new FastDbCommand(this, sql));
        return (FastDbCommand)commands[n];
      }
    }

    internal void RemoveCommand(FastDbCommand command) {
      lock(typeof(FastDbConnection)) {
        commands.Remove(command);
      }
    }

    /// <summary>
    /// Attach current thread to the database. Each thread except one opened the database 
    /// should first attach to the database before any access to the database, 
    /// and detach after end of the work with database.
    /// </summary>
    public void Attach() {
      CLI.CliCheck(CLI.cli_attach(session));
    }

    public void Detach() { 
      Detach(CLI.CliDetachMode.cli_commit_on_detach | CLI.CliDetachMode.cli_destroy_context_on_detach);
    }

    /// <summary>
    /// Detach current thread from the database. Each thread except one opened the database 
    /// should perform attach to the database before any access to the database, 
    /// and detach after end of the work with database
    /// <seealso cref="CLI.CliDetachMode"/>
    /// </summary>
    /// <param name="mode">Optional parameter indicating the detach action.</param>
    public void Detach(CLI.CliDetachMode mode) {
      CLI.CliCheck(CLI.cli_detach(session, mode));
    }

    /// <summary>
    /// Set exclusive database lock
    /// </summary>  
    public void Lock() {
      CLI.CliCheck(CLI.cli_lock(session));
    }

    /// <summary>
    /// Perform database backup
    /// </summary>  
    /// <param name="filePath">backup file path</param>
    /// <param name="compactify">if true then databae will be compactified during backup - 
    /// i.e. all used objects will be placed together without holes; if false then 
    ///  backup is performed by just writting memory mapped object to the backup file.</param>    
    public void Backup(string filePath, bool compactify) {
      CLI.CliCheck(CLI.cli_backup(session, filePath, compactify ? 1 : 0));
    }

    /// <summary>
    /// Schedule database backup
    /// </summary>  
    /// <param name="filePath">path to backup file. If name ends with '?', then
    /// each backup willbe placed in seprate file with '?' replaced with current timestamp</param>
    /// <param name=" period">period of performing backups in seconds</param>    
    public void ScheduleBackup(string filePath, int period) {
      CLI.CliCheck(CLI.cli_schedule_backup(session, filePath, period));
    }

    /// <summary>
    /// Extract a DDL of a table
    /// </summary>
    /// <param name="TableName">Name of a table</param>
    /// <returns>A string representing the table's DDL.</returns>
    public string ExtractTableDDL(string TableName) {
      FastDbFields flds = DescribeTable(TableName);
      StringBuilder result = new StringBuilder("create table "+TableName+" (\n");

      int nLen = 0;
      for(int i=0; i<flds.Count; i++) nLen = (nLen > flds[i].Name.Length) ? nLen : flds[i].Name.Length;
      for(int i=0; i<flds.Count; i++) {
        result.AppendFormat("\t{0} {1}{2}", flds[i].Name.PadRight(nLen, ' '), CLI.CliTypeToStr(flds[i].Type, true),
          (flds[i].RefTable == null) ? "" : " to "+flds[i].RefTable);
        result.Append((i==(flds.Count-1)) ? "" : ",\n");
      }
      result.Append(");\n");
      string IDX_STR = "create {0} on {1}.{2};\n";
      for(int i=0; i<flds.Count; i++) {
        if (Enum.IsDefined(flds[i].Flags.GetType(), CLI.FieldFlags.cli_hashed))
          result.AppendFormat(IDX_STR, "hash",  TableName, flds[i].Name);
        if (Enum.IsDefined(flds[i].Flags.GetType(), CLI.FieldFlags.cli_indexed))
          result.AppendFormat(IDX_STR, "index",  TableName, flds[i].Name);
      }
      return result.ToString();
    }

    /// <summary>
    /// Extracts the metadata of the entire FastDB database, and stores it to a file
    /// </summary>
    /// <param name="FileName">FileName where the DDL is to be saved.</param>
    public void SaveDDLtoFile(string FileName) {
      System.IO.StreamWriter writer = System.IO.File.CreateText(FileName);
      try {
        string[] tables = ListTables();
        writer.WriteLine("open '{0}';", dbName);
        foreach (string table in tables)
          writer.Write(ExtractTableDDL(table));
        writer.WriteLine("commit;");
        writer.WriteLine("exit;");
      }
      finally {
        writer.Close();
      }
    }


    /// <summary>
    /// This method implements IDisposable.  It takes this object off 
    /// the Finalization queue to prevent finalization code for this 
    /// object from executing a second time.
    /// </summary>
    public void Dispose() {
      Dispose(true);
      GC.SuppressFinalize(this);
    }

    /// <summary>
    /// This method executes by a user's call, or by the runtime.
    /// </summary>
    /// <param name="disposing">If disposing equals true, the method has been called directly
    /// or indirectly by a user's code. Managed and unmanaged resources
    /// can be disposed. If disposing equals false, the method has been called by the 
    /// runtime from inside the finalizer and you should not reference 
    /// other objects. Only unmanaged resources can be disposed.</param>
    protected virtual void Dispose(bool disposing) {
      if(this.session != -1) {   // Check to see if Dispose has already been called.
        if(disposing) {} // Dispose managed resources here.
        Close();         // Release unmanaged resources. 
      }
    }

    protected void CheckConnection(bool IsConnected) {
      if ((IsConnected) ? session == -1 : session != -1)
        throw new CliError("The session is " + ((IsConnected) ? "connected" : "not connected"));
    }

    private static void SessionErrorHandler(int error, 
      [MarshalAs(UnmanagedType.LPStr)] string msg, int msgarg, IntPtr UserData) {
      //Debug.Assert(UserData != IntPtr.Zero, "UserData must be assigned FastDbSession value!");
      //int handle; Marshal.Copy(UserData, handle, 0, 1);
      // This procedure must raise an error to unwind the stack
      throw new CliError(error-100, msg+String.Format(" ({0})", msgarg));
    }

    private unsafe string[] ListTables(string TableName, ref bool tableExists) {
      IntPtr p = new IntPtr(0);
      int rc = CLI.CliCheck(CLI.cli_show_tables(session, ref p));
      ArrayList tables = new ArrayList(rc);
      if (rc > 0) {
        try {
          CLI.CliTableDescriptor* table = (CLI.CliTableDescriptor*)p.ToPointer();
          tableExists = false;
          for(int i=0; i < rc; i++, table++) {
            string s = Marshal.PtrToStringAnsi(table->name);
            if (String.Compare(s, TableName, true) == 0) tableExists = true;
            tables.Add(s);
          }
        } 
        finally {
          CLI.cli_free_memory(session, p.ToPointer());
        }
      }
      return (string[])tables.ToArray(typeof(string));
    }

    private void Open(bool isLocal, string Host, int Port) {
      CheckConnection(false);

      if (!isLocal)
        session = CLI.cli_open(String.Format("{0}:{1}", Host, Port), maxConnectRetries, reconnectTimeout);
      else
        session = (int)CLI.ErrorCode.cli_bad_address;

      if (session != (int)CLI.ErrorCode.cli_bad_address) 
        throw new CliError(session, "cli_open failed");
      else {
        if (enableReplication) 
          session = 
            CLI.CliCheck(CLI.cli_create_replication_node(
            nodeID,
            nodeNames.Length,
            nodeNames,
            dbName,
            dbPath,
            (int)openAttributes,
            initDbSize,
            extQuantum,
            initIdxSize,
            fileSizeLimit), "cli_create_replication_node failed");
        else
          session = 
            CLI.CliCheck(CLI.cli_create(dbName,
            dbPath,
            transCommitDelay,
            (int)openAttributes,
            initDbSize,
            extQuantum,
            initIdxSize,
            fileSizeLimit), "cli_create failed");
      }

      sessionThreadID = System.Threading.Thread.CurrentThread.GetHashCode();
      IntPtr dummy    = IntPtr.Zero;
      errorHandler = new CLI.CliErrorHandler(SessionErrorHandler);
      CLI.cli_set_error_handler(session, errorHandler, dummy);
    }

    private string    dbName;
    private string    dbPath;
    private int       session            = -1;
    private int       initDbSize         = DefaultInitDatabaseSize;
    private int       initIdxSize        = DefaultInitIndexSize;
    private int       extQuantum         = DefaultExtensionQuantum;
    private int       fileSizeLimit      = 0;
    private uint      transCommitDelay   = 0;
    private CLI.CliOpenAttribute openAttributes = CLI.CliOpenAttribute.oaReadWrite;
    private int       sessionThreadID    = -1;
    private int       maxConnectRetries  = 0;
    private int       reconnectTimeout   = DefReconnectTimeoutSec;
    private bool      enableReplication  = false;
    private CLI.CliErrorHandler errorHandler;

    private int       nodeID             = 0;
    private string[]  nodeNames          = new string[] {};
    private bool      threaded           = false;
    private ArrayList commands           = new ArrayList();
  }
}
