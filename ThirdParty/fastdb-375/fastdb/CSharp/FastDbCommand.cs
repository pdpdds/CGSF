using System;
using System.Collections;
using System.Text.RegularExpressions;

namespace FastDbNet
{
  //-------------------------------------------------------------------------
  /// <summary>
  /// A class that implements a select/insert query that can be executed in 
  /// FastDb database.  A new command is created using the following code:
  /// <code>
  /// FastDbCommand command = connection.CreateCommand("select * from Table");
  /// </code>
  /// <seealso cref="FastDbConnection.CreateCommand"/>
  /// </summary>
  public class FastDbCommand: IDisposable {
    private FastDbConnection connection;
    private int     statement;
    private string  sql;
    private bool    described;
    private bool    sql_changed;
    private bool    bof;
    private bool    eof;
    private int     rec_no;
    private string  table_name;
    private bool    read_only;
    private int     row_count;
    private FastDbFields     fields;
    private FastDbParameters vars;

    internal FastDbCommand(FastDbConnection connection, string sql): this(connection) {
      this.sql = sql;
    }

    internal FastDbCommand(FastDbConnection connection) {
      this.connection = connection;
      statement   = -1;
      sql         = "";
      described   = false;
      sql_changed = false;
      bof         = false;
      eof         = false;
      rec_no      = -1;
      table_name  = "";
      read_only   = true;
      row_count   = 0;
      fields      = new FastDbFields();
      vars        = new FastDbParameters();
    }
    ~FastDbCommand() { Dispose(false); }

    public void Free() { this.Free(true); }
    public void Free(bool CheckError) {
      if (statement == -1) return;
      if (connection.Threaded) connection.Detach();
      connection.RemoveCommand(this);
      int rc;
      if (connection.Session == -1)
        rc = (int)CLI.ErrorCode.cli_ok;
      else
        rc = CLI.cli_free(statement);
      statement = -1;
      if (CheckError) CLI.CliCheck(rc, "cli_free failed");
      Fields.Clear();
      Parameters.Clear();
      table_name = "";
      described  = false;
    }

    /// <summary>
    /// A reference to the FastDb connection where this command is defined.
    /// </summary>
    public FastDbConnection Connection { get { return connection; } } 
    /// <summary>
    /// Internal statement handle.
    /// </summary>
    public int              Statement  { get { return statement; } } 
    /// <summary>
    /// A container of fields defined in current command.
    /// </summary>
    public FastDbFields     Fields     { get { return fields; } } 
    /// <summary>
    /// A container of parameters defined in current command.
    /// </summary>
    public FastDbParameters Parameters { get { return vars; } } 
    /// <summary>
    /// An SQL string representing a query against the database.
    /// </summary>
    public string           SQL        { get { return sql; } set { sql = value; sql_changed = true; } }
    /// <summary>
    /// A property indicating whether current query is read-only or updatable.
    /// </summary>
    public bool             ReadOnly   { get { return read_only; } }
    /// <summary>
    /// If true - the cursor is at the beginning of the record-set.
    /// </summary>
    public bool             Bof        { get { return bof; } }
    /// <summary>
    /// If true - the cursor is at the end of the record-set.
    /// </summary>
    public bool             Eof        { get { return eof; } }
    /// <summary>
    /// Current number of record within the record-set: 0 ... RowCount
    /// </summary>
    public int              RecNo      { get { return rec_no; } }
    /// <summary>
    /// Total number of records within the record-set.
    /// </summary>
    public int              RowCount      { get { return row_count; } }
    /// <summary>
    /// Name of the table parsed out of the SQL property.
    /// </summary>
    public string           TableName  { get { return table_name; } }

    private bool IsSelectQuery() { return String.Compare(sql.Substring(0, 6), "select", true) == 0; }

    private string GetTableName() {
      if (!sql_changed && table_name != "") return table_name;
      if (sql == "")    return "";

      // The tablename is the first identifier after FROM
      Match m = Regex.Match(sql, @"insert\s+into\s+([a-zA-Z0-9_]+)\s*");
      if (m.Success) return table_name = m.Groups[1].ToString();
      m = Regex.Match(sql, @"select\s+\*?\s*from\s+([a-zA-Z0-9_]+)\s*");
      if (m.Success) 
        return table_name = m.Groups[1].ToString();
      else
        throw new CliError((int)CLI.ErrorCode.cli_table_not_found);
    }

    /// <summary>
    /// Fetch definition of fields from the database.  SQL property must be assigned
    /// </summary>
    public void Describe() {
      if (described && !sql_changed) return;
      else if (statement != -1) Free(false);

      if (SQL == "") throw new CliError("SQL not assigned!");
      string s = GetTableName();
      if (SQL == "") throw new CliError("Error parsing table name from the SQL!");

      if (fields.Count == 0) 
        fields.Assign(connection.DescribeTable(GetTableName()));
      statement = CLI.CliCheck(CLI.cli_statement(connection.Session, sql), "cli_statement failed");
      fields.Bind(statement);
      vars.Bind(statement);
      described = true;
      sql_changed = false;
    }

    public int  Execute()               { return this.Execute(false, true); }
    public int  Execute(bool Updatable) { return this.Execute(Updatable, true); }

    /// <summary>
    /// Executes an SQL command agains a FastDB connection.
    /// </summary>
    /// <param name="Updatable">Determines if records can be updated (optional, default: false).</param>
    /// <param name="FetchFirst">Instructs to fetch the first record (optional, default: false). 
    /// If false, the user must call First() method after Execute().</param>
    /// <returns>Number of records in the record-set fetched from the database.</returns>
    public int  Execute(bool Updatable, bool FetchFirst) {
      if (connection.Threaded) connection.Attach();
      Describe();
      read_only = !Updatable;
      row_count = 
        CLI.CliCheck(
        CLI.cli_fetch(statement, (Updatable) ? CLI.QueryType.cli_for_update : CLI.QueryType.cli_view_only));
      rec_no = (FetchFirst) ? 0 : -1;
      eof    = row_count <= 0;
      if (FetchFirst && !eof) First();
      else bof = FetchFirst;
      return row_count;
    }

    public Cursor Select() { return Select(false); }
    public Cursor Select(bool Updatable) {
      Execute(Updatable, false);
      return new Cursor(this);
    }

    /// <summary>
    /// Inserts a record in the database.  Requires the SQL to be in the form: 
    /// "insert into TableName".
    /// </summary>
    /// <returns>An OID of the newly inserted record.</returns>
    public uint Insert() {
      uint oid = 0;
      if (connection.Threaded) connection.Attach();
      Describe();
      CLI.CliCheck(CLI.cli_insert(statement, ref oid));
      if (connection.Threaded) connection.Detach();
      return oid;
    }

    /// <summary>
    /// Updates a record in the database.  Requires the SQL to be in the form: 
    /// "select * from TableName ...", and the command must be updatable:
    /// command.Execute(true).
    /// </summary>
    public void Update() { 
      if (read_only) 
        throw new CliError((int)CLI.ErrorCode.cli_not_update_mode); 
      CLI.CliCheck(CLI.cli_update(statement)); 
    }
    /// <summary>
    /// Deletes all records in the current record-set returned by the select statement.
    /// Requires the SQL to be in the form: 
    /// "select * from TableName ...", and the command must be updatable:
    /// command.Execute(true);
    /// command.Delete();
    /// </summary>
    public void Delete()   { CLI.CliCheck(CLI.cli_remove(statement)); }
    /// <summary>
    /// Freeze cursor. Make it possible to reuse cursor after commit of 
    /// the current transaction.
    /// </summary>
    public void Freeze()   { CLI.CliCheck(CLI.cli_freeze(statement)); }
    /// <summary>
    /// Unfreeze cursor. Reuse previously frozen cursor.
    /// </summary>
    public void UnFreeze() { CLI.CliCheck(CLI.cli_unfreeze(statement)); }
    /// <summary>
    /// Return the OID of the currently fetched record in the cursor.
    /// </summary>
    public uint OID        { get { return CLI.cli_get_oid(statement); } }

    /// <summary>
    /// Refresh the selected record the cursor points to.
    /// </summary>
    public void RefreshRecord() { CLI.CliCheck(CLI.cli_skip(statement, 0)); }
    /// <summary>
    /// Determines if the current command has an open cursor.
    /// </summary>
    /// <returns>true - the current query is open.</returns>
    public bool IsOpen()        { return statement != -1; }
    /// <summary>
    /// Go to the first row of the current record-set.
    /// </summary>
    public void First()         { CLI.CliCheck(CLI.cli_get_first(statement)); rec_no = 0; }
    /// <summary>
    /// Go to the last row of the current record-set.
    /// </summary>
    public void Last()          { CLI.CliCheck(CLI.cli_get_last(statement)); rec_no = row_count; }
    
    /// <summary>
    /// Go to the next row of the current record-set.
    /// </summary>
    public bool Next() { 
      int n = CLI.cli_get_next(statement); 
      eof = n != (int)CLI.ErrorCode.cli_ok;
      bof = false;
      if (!eof) rec_no++;
      return !eof;
    }
    
    /// <summary>
    /// Go to the previous row of the current record-set.
    /// </summary>
    public bool Prev() { 
      int n = CLI.cli_get_prev(statement); 
      bof = n != (int)CLI.ErrorCode.cli_ok;
      eof = false;
      if (!bof) rec_no--;
      return !bof;
    }

    public void Skip(int Records) { Skip(Records, false); }
    /// <summary>
    /// Skip N rows of the current record-set.
    /// </summary>
    /// <param name="Records">Number of records to skip</param>
    /// <param name="RefreshOnNoSkip">Optional parameter (default: false). If true, forces a refresh when parameter Records=0.</param>
    public void Skip(int Records, bool RefreshOnNoSkip) {
      if ((Records == 0) && !RefreshOnNoSkip) return;
      int n = CLI.cli_skip(statement, Records);

      if (n == (int)CLI.ErrorCode.cli_not_found) {
        if (Records >= 0) {
          eof = true;
          bof = false;
          rec_no = row_count;
        }
        else {
          bof = true;
          eof = false;
          rec_no = 0;
        }
      }
      else {
        CLI.CliCheck(n, "cli_skip failed");
        rec_no += Records;
      }
    }

    /// <summary>
    /// Seeks for a given OID in the current record-set.
    /// </summary>
    /// <param name="oid">OID to search for.</param>
    /// <returns>Position of the record in the selection.</returns>
    public int  Seek(uint oid) {
      return rec_no = CLI.CliCheck(CLI.cli_seek(statement, oid));
    }

    /// <summary>
    /// Clears the current command, empties fields/parameters, and frees unmanaged resources.
    /// </summary>
    public void Clear() { Free(); }

    /// <summary>
    /// This class is used to iterate over a record set using foreach() statement.
    /// Example:
    /// <code>
    /// FastDbCommand command = connection.CreateCommand("select * from persons");
    /// foreach(FastDbCommand.Cursor cur in command.Select()) {
    ///   foreach(FastDbField f in cur.Fields) 
    ///     Console.WriteLine(f.asString);
    /// }
    /// </code>
    /// </summary>
    public class Cursor: IEnumerable {
      FastDbCommand command;
      public Cursor(FastDbCommand command) {
        this.command = command;
      }
      public FastDbFields  Fields   { get { return command.Fields; } }
      public FastDbCommand Command  { get { return command; } }

      #region IEnumerable Members

      IEnumerator IEnumerable.GetEnumerator() { return GetEnumerator(); }
      public Iterator GetEnumerator()         { return new Iterator(this); }

      /// <summary>
      /// This class implements IEnumerator interface. Use its methods within a 
      /// foreach() loop.  
      /// </summary>
      public class Iterator: IEnumerator {
        #region Private Fields
        Cursor cursor;
        #endregion

        #region Constructors
        public Iterator(Cursor cursor) {
          this.cursor = cursor;
        }
        #endregion

        #region Methods
        public void Reset()         { cursor.command.First(); }
        public bool MoveNext()      { 
          if (cursor.Command.RecNo == -1) { 
            Reset(); 
            return !cursor.command.Eof;
          } else { 
            return cursor.command.Next(); 
          }
        }
        public Cursor Current { get { return cursor; } }
        // The current property on the IEnumerator interface:
        object IEnumerator.Current  { get { return Current; } }
        #endregion
      }
      #endregion
    }

    #region IDisposable Members
    // Implement IDisposable.
    // This method is not virtual. A derived class should not be able to override this method.
    public void Dispose() {
      Dispose(true);
      // Take yourself off the Finalization queue 
      // to prevent finalization code for this object
      // from executing a second time.
      GC.SuppressFinalize(this);
    }

    // Dispose(bool disposing) executes in two distinct scenarios.
    // If disposing equals true, the method has been called directly
    // or indirectly by a user's code. Managed and unmanaged resources
    // can be disposed.
    // If disposing equals false, the method has been called by the 
    // runtime from inside the finalizer and you should not reference 
    // other objects. Only unmanaged resources can be disposed.
    protected virtual void Dispose(bool disposing) {
      if(this.statement != -1) {   // Check to see if Dispose has already been called.
        if(disposing) {} // Dispose managed resources here.
        Free();          // Release unmanaged resources. 
      }
    }
    #endregion
  }
}
