using System;
using System.Text;
using System.Runtime.InteropServices;

namespace FastDbNet
{
  /// <summary>
  /// Exception thrown by Gigabase/FastDB CLI implementation.
  /// </summary>
  public class CliError : Exception {
    public int ErrorCode;

    ///<summary>
    /// Constructor of the exception object.
    /// </summary>
    /// <param name="Msg">message describing the reason of the fault</param>
    public CliError(string Msg) : this((int)CLI.ErrorCode.cli_unknown_error, Msg) {}
    ///<summary>
    /// Constructor of the exception object.
    /// </summary>
    /// <param name="Code">Error code describing the reason of the fault</param>
    public CliError(int Code)   : this(Code, "") {}
    ///<summary>
    /// Constructor of the exception object.
    /// </summary>
    /// <param name="Code">Error code describing the reason of the fault</param>
    /// <param name="Msg">Message describing the reason of the fault</param>
    public CliError(int Code, string Msg): base(FormatMessage(Code, Msg)) {
      ErrorCode = Code;
    }
    private static string FormatMessage(int Code, string Msg) {
      if (Msg != "") 
        Msg += ". " + CLI.CliErrorToStr(Code);
      else
        Msg = CLI.CliErrorToStr(Code);
      return Msg;
    }
  }


  /// <summary>
  /// FastDb CLI interface.
  /// </summary>
  public class CLI {

#if GIGABASE
#if LINUX
		public const string libname = "libgigabase_r.so";
#else
	  public const string libname = "GigaBase.dll";
#endif
#else
#if LINUX
		public const string libname = "libfastdb_r.so";
#else
    public const string libname = "FastDB.dll";
#endif
#endif

    //-----------------------------------------
    // cli_result_code
    //-----------------------------------------
    public enum ErrorCode : int {
      cli_ok                         =  0,
      cli_bad_address                = -1,
      cli_connection_refused         = -2,
      cli_database_not_found         = -3,
      cli_bad_statement              = -4,
      cli_parameter_not_found        = -5,
      cli_unbound_parameter          = -6,
      cli_column_not_found           = -7,
      cli_incompatible_type          = -8,
      cli_network_error              = -9,
      cli_runtime_error              = -10,
      cli_bad_descriptor             = -11,
      cli_unsupported_type           = -12,
      cli_not_found                  = -13,
      cli_not_update_mode            = -14,
      cli_table_not_found            = -15,
      cli_not_all_columns_specified  = -16,
      cli_not_fetched                = -17,
      cli_already_updated            = -18,
      cli_table_already_exists       = -19,
      cli_not_implemented            = -20,
#if GIGABASE
  		cli_login_failed               = -21,
	  	cli_empty_parameter            = -22,
		  cli_closed_connection          = -23
#endif

      //-----------------------------------------
      // cli_error_class
      // Note: When calling CliErrorToStr in TCliErrorHandler subtract 100 from the
      //       code passed as ErrorClassCode in order to get correct description
      //-----------------------------------------
      cli_query_error                = 1  -100,
      cli_arithmetic_error           = 2  -100,
      cli_index_out_of_range_error   = 3  -100,
      cli_database_open_error        = 4  -100,
      cli_file_error                 = 5  -100,
      cli_out_of_memory_error        = 6  -100,
      cli_deadlock                   = 7  -100,
      cli_null_reference_error       = 8  -100,
      cli_lock_revoked               = 9  -100,
      cli_file_limit_exeeded         = 10 -100,

      //-----------------------------------------
      // Extended Error Codes
      //-----------------------------------------
      cli_error_loading_library      = -200,
      cli_session_not_assigned       = -201,
      cli_database_already_open      = -202,
      cli_invalid_field_size         = -203,
      cli_empty_query                = -204,
      cli_item_already_defined       = -205,
      cli_wrong_inverse_reference    = -206,
      cli_no_fields_defined          = -207,
      cli_access_violation           = -208,
      cli_unknown_error              = -999
    }

    //-----------------------------------------
    // Field Types
    //-----------------------------------------
    public enum FieldType : int {
      cli_oid                        = 0,
      cli_bool                       = 1,
      cli_int1                       = 2,
      cli_int2                       = 3,
      cli_int4                       = 4,
      cli_int8                       = 5,
      cli_real4                      = 6,
      cli_real8                      = 7,
      cli_decimal                    = 8,  //{ not supported in FastDB }
      cli_asciiz                     = 9,  //{ zero terminated string (Get/Set function can be used) }
      cli_pasciiz                    = 10, //{ pointer to zero terminated string }
      cli_cstring                    = 11, //{ not supported in FastDB }
      cli_array_of_oid               = 12,
      cli_array_of_bool              = 13,
      cli_array_of_int1              = 14,
      cli_array_of_int2              = 15,
      cli_array_of_int4              = 16,
      cli_array_of_int8              = 17,
      cli_array_of_real4             = 18,
      cli_array_of_real8             = 19,
      cli_array_of_decimal           = 20, //{ not supported in FastDB }
      cli_array_of_string            = 21,
      cli_any                        = 22, //{ not supported in FastDB }
      cli_datetime                   = 23, //{ not supported in FastDB }
      cli_autoincrement              = 24,
      cli_rectangle                  = 25, //{ not supported in FastDB }
      cli_unknown                    = 26,
      //--- Custom types not supported by the database directly -----
      cli_subst                            // Reserved for substitution variables
    };

    internal static int[] SizeOfCliType =
      new int[] {
                  Marshal.SizeOf(typeof(int)),          // cli_oid
                  Marshal.SizeOf(typeof(bool)),					// cli_bool
                  Marshal.SizeOf(typeof(sbyte)),				// cli_int1
                  Marshal.SizeOf(typeof(short)),				// cli_int2
                  Marshal.SizeOf(typeof(int)),	        // cli_int4
                  Marshal.SizeOf(typeof(long)),					// cli_int8_t
                  Marshal.SizeOf(typeof(float)),  			// cli_real4_t
                  Marshal.SizeOf(typeof(double)),				// cli_real8_t
                  0, // cli_decimal
                  0, // cli_asciiz,
                  0, // cli_pasciiz,
                  0, // cli_cstring,
                  0, // cli_array_of_oid,
                  0, // cli_array_of_bool,
                  0, // cli_array_of_int1,
                  0, // cli_array_of_int2,
                  0, // cli_array_of_int4,
                  0, // cli_array_of_int8,
                  0, // cli_array_of_real4,
                  0, // cli_array_of_real8,
                  0, // cli_array_of_decimal,
                  0, // cli_array_of_string,
                  0, // cli_any,
                  Marshal.SizeOf(typeof(double)),         // cli_datetime,
                  Marshal.SizeOf(typeof(int)),          // cli_autoincrement,
                  0, // cli_rectangle,
                  0, // cli_unknown
                  0  // ctSubst
                };

    public static string CliTypeToStr(FieldType type, bool ExtendedSyntax) {
      StringBuilder s = new StringBuilder("");
      FieldType ft = type;
      if (ExtendedSyntax && type >= CLI.FieldType.cli_array_of_oid && type <= CLI.FieldType.cli_array_of_string) {
        s.Append("array of ");
        ft = (CLI.FieldType)((int)type - (int)CLI.FieldType.cli_array_of_oid);
      }

      switch(ft) {
        case CLI.FieldType.cli_oid:
          s.Append((ExtendedSyntax)? "reference" : "(oid)"); break;
        case CLI.FieldType.cli_bool:
          s.Append((ExtendedSyntax)? "bool" : "Boolean"); break;
        case CLI.FieldType.cli_int1:
          s.Append((ExtendedSyntax)? "int1" : "Byte"); break;
        case CLI.FieldType.cli_int2:
          s.Append((ExtendedSyntax)? "int2" : "SmallInt"); break;
        case CLI.FieldType.cli_autoincrement:
        case CLI.FieldType.cli_int4:
          s.Append((ExtendedSyntax)? "int4" : "Integer"); break;
        case CLI.FieldType.cli_int8:
          s.Append((ExtendedSyntax)? "int8" : "Int64"); break;
        case CLI.FieldType.cli_real4:
          s.Append((ExtendedSyntax)? "real4" : "Single"); break;
        case CLI.FieldType.cli_datetime:
          s.Append((ExtendedSyntax)? "real8" : "DateTime"); break;
        case CLI.FieldType.cli_real8:
          s.Append((ExtendedSyntax)? "real8" : "Double"); break;
        case CLI.FieldType.cli_asciiz:
        case CLI.FieldType.cli_pasciiz:
          s.Append((ExtendedSyntax)? "string" : "String"); break;
        default:
          if (!ExtendedSyntax) 
            s.Append(Enum.GetName(typeof(CLI.FieldType), ft).Substring(4)); break;
      }
      return s.ToString();
    }

    public static bool IsArrayType(FieldType tp) { 
      return tp >= FieldType.cli_array_of_oid && tp <= FieldType.cli_array_of_real8; 
    }

    public static int CliCheck(int code, string Msg) {
      if (code < 0)
        throw new CliError(code, Msg);
      else
        return code;
    }

    public static int CliCheck(int code) {
      return CliCheck(code, "");
    }

    // translate error code to string
    public static string CliErrorToStr(int Code) {
      if (Code >= 0)
        return null;
      else
        switch (Code) {
          case (int)ErrorCode.cli_bad_address                : return "Invalid format of server URL";
          case (int)ErrorCode.cli_connection_refused         : return "Connection with server could not be established";
          case (int)ErrorCode.cli_database_not_found         : return "Database cannot be found";
          case (int)ErrorCode.cli_bad_statement              : return "Text of SQL statement is not correct";
          case (int)ErrorCode.cli_parameter_not_found        : return "Parameter was not found in statement";
          case (int)ErrorCode.cli_unbound_parameter          : return "Parameter was not specified";
          case (int)ErrorCode.cli_column_not_found           : return "No such colunm in the table";
          case (int)ErrorCode.cli_incompatible_type          : return "Conversion between application and database type is not possible";
          case (int)ErrorCode.cli_network_error              : return "Connection with server is broken";
          case (int)ErrorCode.cli_runtime_error              : return "Error during query execution";
          case (int)ErrorCode.cli_bad_descriptor             : return "Invalid statement/session description";
          case (int)ErrorCode.cli_unsupported_type           : return "Unsupported type for parameter or column";
          case (int)ErrorCode.cli_not_found                  : return "Record was not found";
          case (int)ErrorCode.cli_not_update_mode            : return "Attempt to update records selected by view only cursor";
          case (int)ErrorCode.cli_table_not_found            : return "There is no table with specified name in the database";
          case (int)ErrorCode.cli_not_all_columns_specified  : return "Insert statement doesn''t specify values for all table columns";
          case (int)ErrorCode.cli_not_fetched                : return "cli_fetch method was not called";
          case (int)ErrorCode.cli_already_updated            : return "cli_update method was invoked more than once for the same record";
          case (int)ErrorCode.cli_table_already_exists       : return "Attempt to create existing table";
          case (int)ErrorCode.cli_not_implemented            : return "Function is not implemented";
            //----- Severe Error Class Codes---------
          case (int)ErrorCode.cli_query_error                : return "Query error";
          case (int)ErrorCode.cli_arithmetic_error           : return "Arithmetic error";
          case (int)ErrorCode.cli_index_out_of_range_error   : return "Index out of range";
          case (int)ErrorCode.cli_database_open_error        : return "Database open error";
          case (int)ErrorCode.cli_file_error                 : return "File error";
          case (int)ErrorCode.cli_out_of_memory_error        : return "Out of memory";
          case (int)ErrorCode.cli_deadlock                   : return "Deadlock detected";
          case (int)ErrorCode.cli_null_reference_error       : return "Null reference";
          case (int)ErrorCode.cli_lock_revoked               : return "Lock revoked";
          case (int)ErrorCode.cli_file_limit_exeeded         : return "File limit exeeded";
            //----- Custom Error Codes---------------
          case (int)ErrorCode.cli_error_loading_library      : return "Error loading library";
          case (int)ErrorCode.cli_session_not_assigned       : return "Session not assigned or not connected";
          case (int)ErrorCode.cli_database_already_open      : return "Database already open";
          case (int)ErrorCode.cli_invalid_field_size         : return "Invalid field size";
          case (int)ErrorCode.cli_empty_query                : return "Query SQL text is not assigned";
          case (int)ErrorCode.cli_item_already_defined       : return "Field/Variable is already defined";
          case (int)ErrorCode.cli_wrong_inverse_reference    : return "Wrong inverse reference";
          case (int)ErrorCode.cli_no_fields_defined          : return "No fields defined";
          case (int)ErrorCode.cli_access_violation           : return "Access Violation";
          default:						                                 return String.Format("({0})", Code);
        }
    }

    [Flags]
      public enum CliOpenAttribute : int {
      oaReadWrite      = 0x00, //cli_open_default
      oaReadOnly			 = 0x01, //cli_open_readonly
      oaTruncate       = 0x02, //cli_open_truncate
      oaOpenConcurrent = 0x04  //cli_open_concurrent
    };

    /*=====================================================================
     * cli_open
     *     Establish connection with the server
     * Parameters:
     *     server_url - zero terminated string with server address and port,
     *                  for example "localhost:5101", "195.239.208.240:6100",...
     *     max_connect_attempts  - number of attempts to establish connection
     *     reconnect_timeout_sec - timeput in seconds between connection attempts
     *  Gigabase users:
     *  ===============
     *  user_name - user name for login
     *  password  - password for login
     *  pooled_connection - if not 0, then connection will be allocated from the connection pool
     *
     * Returns:
     *     >= 0 - connectiondescriptor to be used in all other cli calls
     *     <  0 - error code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_open(
      string ServerURL,
      int MaxConnectAttempts,
      int ReconnectTimeoutSec
#if GIGABASE
        , string UserName
        , string Password
        , bool PooledConnection
#endif
      );

    /*=====================================================================
     * cli_create
     *     Create connection to the local database
     * Parameters:
     *     databaseName - name of the database
     *     fileName - path to the database file
     *     transactionCommitDelay - trasnaction commit delay (specify 0 to disable)
     *     openAttr - mask of cli_open_attributes
     *     initDatabaseSize - initial size of the database
     *     extensionQuantum - database extension quantum
     *     initIndexSize - initial size of object index
     *     fileSizeLimit - limit for file size (0 - unlimited)
     * Returns:
     *     >= 0 - connection descriptor to be used in all other cli calls
     *     <  0 - error code as described in cli_result_code enum
     */

    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_create(
#if GIGABASE
      uint TransactionCommitDelay: Word=0;
      int  openAttr;
      int  PoolSize
#else
      string databaseName,   //[MarshalAs(UnmanagedType.LPStr)] string databaseName,
      string filePath,       //[MarshalAs(UnmanagedType.LPStr)] string filePath,
      uint   transactionCommitDelay,
      int    openAttr,
      int    initDatabaseSize,
      int    extensionQuantum,
      int    initIndexSize,
      int    fileSizeLimit
#endif
      );

    /*=====================================================================
     * cli_create_replication_node
     *     Create connection to the local database with support of replication
     * Parameters:
     *     nodeId - node identifier: 0 <= nodeId < nServers
     *     nServers - number of replication nodes (primary + standby)
     *     nodeNames - array with URLs of the nodes (address:port)
     *     databaseName - name of the database
     *     fileName - path to the database file
     *     transactionCommitDelay - trasnaction commit delay (specify 0 to disable)
     *     openAttr - mask of cli_open_attributes (to allow concurrent read access to replication node,
     *                cli_open_concurrent attribute should be set)
     *     initDatabaseSize - initial size of the database
     *     extensionQuantum - database extension quantum
     *     initIndexSize - initial size of object index
     *     fileSizeLimit - limit for file size (0 - unlimited)
     * Returns:
     *     >= 0 - connection descriptor to be used in all other cli calls
     *     <  0 - error code as described in cli_result_code enum
     */

    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_create_replication_node(
      int                nodeId,
      int                nServers,
      [In, Out] String[] nodeNames,
      string             databaseName,
      string             filePath,
      int                openAttr,
      int                initDatabaseSize,
      int                extensionQuantum,
      int                initIndexSize,
      int                fileSizeLimit);

    /*=====================================================================
      * cli_close
      *     Close session
      * Parameters:
      *     session - session descriptor returned by cli_open
      * Returns:
      *     result code as described in cli_result_code enum
      */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_close(int session);

    /*=====================================================================
     * cli_statement
     *     Specify SubSQL statement to be executed at server
     *     Binding to the parameters and columns can be established
     * Parameters:
     *     session - session descriptor returned by cli_open
     *     stmt    - zero terminated string with SubSQL statement
     * Returns:
     *     >= 0 - statement descriptor
     *     <  0 - error code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_statement(int session, string stmt);

    /*=====================================================================
     * cli_parameter
     *     Bind parameter to the statement
     * Parameters:
     *     statement  - statememt descriptor returned by cli_statement
     *     param_name - zero terminated string with parameter name
     *                  Paramter name should start with '%'
     *     var_type   - type of variable as described in CliVarType enum.
     *                  Only scalar and zero terminated string types are supported.
     *     var_ptr    - pointer to the variable
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_parameter(
      int       statement,
      string    param_name,
      int       var_type,
      IntPtr    var_ptr);

    /*=====================================================================
     * cli_column
     *     Bind extracted column of select or insert statement
     * Parameters:
     *     statement   - statememt descriptor returned by cli_statement
     *     column_name - zero terminated string with column name
     *     var_type    - type of variable as described in CliVarType enum
     *     var_len     - pointer to the variable to hold length of array variable.
     *                   This variable should be assigned the maximal length
     *                   of the array/string buffer, pointed by var_ptr.
     *                   After the execution of the statement it is assigned the
     *                   real length of the fetched array/string. If it is large
     *                   than length of the buffer, then only part of the array
     *                   will be placed in the buffer, but var_len still will
     *                   contain the actual array length.
     *     var_ptr     - pointer to the variable
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    public static extern int cli_column(
       int       statement,
       string    column_name,
       int       var_type,
       ref int   var_len,
       IntPtr var_data);

    [ StructLayout( LayoutKind.Sequential, CharSet=CharSet.Ansi )]
    public struct UnmanagedBuffer {
      public int    type;
      public int    capacity;
      public int    size;
      public bool   fetch_data;
      public IntPtr data;
    }

    /* The C# does not allow you to specify the calling convention of the callback.
     * It is just one of the C# limitations. IL, managed C++ and the runtime itself
     * supports the cdecl calling convention for delegates through
     * modopt([mscorlib]System.Runtime.CompilerServices.CallConvCdecl) on the
     * internal Invoke method of the delegate. Run ildasm on a small example in
     * managed C++ if you want to know the exact syntax.
     */

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal unsafe delegate IntPtr CliColumnSetEx(
      int       var_type, 
      IntPtr    var_ptr,
      int       len, 
      [MarshalAs(UnmanagedType.LPStr)] 
      string    column_name, 
      int       statement, 
      IntPtr    source_ptr,
      void* buffer);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal unsafe delegate IntPtr CliColumnGetEx(
      int     var_type, 
      IntPtr  var_ptr,
      ref int len, 
      string column_name, int statement,
      void* buffer);


    /*=====================================================================
     * cli_array_column
     *     Specify get/set functions for the array column
     * Parameters:
     *     statement   - statememt descriptor returned by cli_statement
     *     column_name - zero terminated string with column name  
     *     var_type    - type of variable as described in CliVarType enum
     *     var_ptr     - pointer to the variable
     *     set         - function which will be called to construct fetched 
     *                   field. It receives pointer to the variable, 
     *                   length of the fetched array and returns pointer to th 
     *                   array's elements
     *     get         - function which will be called to update the field in the 
     *                   database. Given pointer to the variable, it should return 
     *                   pointer to the array elements and store length of the
     *                   array to the variable pointer by len parameter
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal unsafe static extern int cli_array_column_ex(
      int            statement,
      string         column_name, 
      int            var_type,
      IntPtr         var_ptr,
      CliColumnSetEx SetCallback,
      CliColumnGetEx GetCallback,
      void* user_data);

    public enum QueryType: int {
      cli_view_only  = 0, 
      cli_for_update = 1
    }

    /*=====================================================================
     * cli_fetch
     *     Execute select statement.
     * Parameters:
     *     statement    - statememt descriptor returned by cli_statement
     *     queryType - not zero if fetched rows will be updated 
     * Returns:
     *     >= 0 - success, for select statements number of fetched rows is returned
     *     <  0 - error code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_fetch(int statement, QueryType queryType);

    /*=====================================================================
     * cli_insert
     *     Execute insert statement.
     * Parameters:
     *     statement  - statememt descriptor returned by cli_statement
     *     oid        - object identifier of created record. 
     * Returns:
     *     status code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_insert(int statement, ref uint oid);

    /*=====================================================================
     * cli_get_first
     *     Get first row of the selection.
     * Parameters:
     *     statement  - statememt descriptor returned by cli_statement
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_get_first(int statement);

    /*=====================================================================
     * cli_get_last
     *     Get last row of the selection.
     * Parameters:
     *     statement  - statememt descriptor returned by cli_statement
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_get_last(int statement);

    /*=====================================================================
     * cli_get_next
     *     Get next row of the selection. If get_next records is called
     *     exactly after cli_fetch function call, is will fetch the first record
     *     in selection.
     * Parameters:
     *     statement  - statememt descriptor returned by cli_statement
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_get_next(int statement);

    /*=====================================================================
     * cli_get_prev
     *     Get previous row of the selection. If get_next records is called
     *     exactly after cli_fetch function call, is will fetch the last record
     *     in selection.
     * Parameters:
     *     statement  - statememt descriptor returned by cli_statement
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_get_prev(int statement);

    /*=====================================================================
     * cli_skip
     *     Skip specified number of rows. 
     * Parameters:
     *     statement  - statememt descriptor returned by cli_statement
     *     n          - number of objects to be skipped
     *                - if "n" is positive, then this function has the same effect as
     *                     executing cli_get_next() function "n" times.
     *                - if "n" is negative, then this function has the same effect as
     *                     executing cli_get_prev() function "-n" times.
     *                - if "n"  is zero, this method just reloads current record
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_skip(int statement, int n);

    /*=====================================================================
     * cli_seek
     *    Position cursor to the record with specified OID
     * Parameters:
     *     statement   - statememt descriptor returned by cli_statement
     *     oid         - object identifier of the record to which cursor should be positioned
     * Returns:
     *     >= 0 - success, position of the record in the selection
     *     <  0 - error code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_seek(int statement, uint oid);


    /*=====================================================================
     * cli_get_oid
     *     Get object identifier of the current record
     * Parameters:
     *     statement  - statememt descriptor returned by cli_statement
     * Returns:
     *     object identifier or 0 if no object is seleected
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern uint cli_get_oid(int statement);
    
    /*=====================================================================
     * cli_update
     *     Update the current row in the selection. You have to set
     *     for_update parameter of cli_fetch to 1 in order to be able 
     *     to perform updates. Updated value of row fields will be taken
     *     from bound column variables. 
     * Parameters:
     *     statement   - statememt descriptor returned by cli_statement
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_update(int statement);

    /*=====================================================================
     * cli_remove
     *     Remove all selected records. You have to set
     *     for_update parameter of cli_fetch to 1 in order to be able 
     *     to remove records. 
     * Parameters:
     *     statement   - statememt descriptor returned by cli_statement
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_remove(int statement);

    /*=====================================================================
     * cli_free
     *     Deallocate statement and all associated data
     * Parameters:
     *     statement   - statememt descriptor returned by cli_statement
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_free(int statement);

    /*=====================================================================
     * cli_commit
     *     Commit current database transaction
     * Parameters:
     *     session - session descriptor as returned by cli_open
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_commit(int session);

    /*=====================================================================
     * cli_precommit
     *     Release all locks set by transaction. This methods allows other clients
     *     to proceed, but it doesn't flush transaction to the disk.
     * Parameters:
     *     session - session descriptor as returned by cli_open
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_precommit(int session);

    /*=====================================================================
     * cli_abort
     *     Abort current database transaction
     * Parameters:
     *     session - session descriptor as returned by cli_open
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_abort(int session);

    public enum FieldFlags: int { 
      cli_noindex       = 0, /* no indexes */
      cli_hashed        = 1, /* field should be indexed usnig hash table */
      cli_indexed       = 2  /* field should be indexed using B-Tree */
    };

    [ StructLayout( LayoutKind.Sequential, CharSet=CharSet.Ansi )]
    internal struct CliFieldDescriptor {  
      public FieldType     type;
      public FieldFlags    flags;
      public IntPtr        name;
      public IntPtr        refTableName;
      public IntPtr        inverseRefFieldName;
    };

    /*=====================================================================
     * cli_describe
     *     Describe fileds of specified table
     * Parameters:
     *     session - session descriptor as returned by cli_open
     *     table   - name of the table
     *     fields  - address of the pointer to the array of fields descriptors, 
     *               this array should be later deallocated by application by cli_free_memory()
     * Returns:
     *     >= 0 - number of fields in the table
     *     < 0  - result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern unsafe int cli_describe(
      int    session, 
      string table,
      ref void* fields);
    //[In,Out] ref CliFieldDescriptor[] fields);  // cli_field_descriptor** fields

    /*=====================================================================
     * cli_get_field_size
     *     Calculate field size
     * Parameters:
     *     fields  - array with fields descriptors obtained using cli_describe function
     *     field_no - number of the field
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_get_field_size(
      [In, Out] CliFieldDescriptor[] fields,   // cli_field_descriptor* fields
      int field_no);

    /*=====================================================================
     * cli_get_field_offset
     *     Calculate offset of the field 
     * Parameters:
     *     fields  - array with fields descriptors obtained using cli_describe function
     *     field_no - number of the field
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_get_field_offset(
      [In, Out] CliFieldDescriptor[] fields,   // cli_field_descriptor* fields
      int field_no);

    [ StructLayout( LayoutKind.Sequential, CharSet=CharSet.Ansi )]
      internal struct CliTableDescriptor {
      public IntPtr name;
    }

    /*=====================================================================
     * cli_show_tables
     *     Show all tables of specified database
     * Parameters:
     *     session - session descriptor as returned by cli_open
     *     tables  - address of the pointer to the array of tables descriptors,
     *               this array should be later deallocated by application by cli_free_memory()
     * Returns:
     *     >= 0 - number of tables in the database (Metatable is not returned/counted)
     *     < 0  - result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_show_tables(
      int session, 
      //[In, Out] CliTableDescriptor[] tables
      ref IntPtr tables);   // cli_table_descriptor** tables

    /*=====================================================================
     * cli_create_table
     *     Create new table
     * Parameters:
     *     session   - session descriptor as returned by cli_open
     *     tableName - name of new table
     *     nFields   - number of columns in the table
     *     fields    - array with table columns descriptors
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_create_table(
      int       session, 
      string    tableName, 
      int       nFields, 
      [In, Out] CliFieldDescriptor[] fields);  // cli_field_descriptor* fields

    /*=====================================================================
     * cli_alter_table
     *     Change table format
     * Parameters:
     *     session   - session descriptor as returned by cli_open
     *     tableName - name of existing table
     *     nFields   - number of columns in the table
     *     fields    - array with new table columns descriptors
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_alter_table(
      int    session, 
      string tableName, 
      int    nFields, 
      [MarshalAs(UnmanagedType.LPArray)]
      CliFieldDescriptor[] fields);  // cli_field_descriptor* fields);

    /*=====================================================================
     * cli_drop_table
     *     drop the table
     * Parameters:
     *     session   - session descriptor as returned by cli_open
     *     tableName - name of deleted table
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_drop_table(int session, string tableName);


    /*=====================================================================
     * cli_alter_index
     *     add or remove column index
     * Parameters:
     *     session   - session descriptor as returned by cli_open
     *     tableName - name of the table
     *     fieldName - name of field
     *     newFlags  - new flags of the field, if index exists for this field, but is not specified in 
     *                 <code>newFlags</code> mask, then it will be removed; if index not exists, but is 
     *                 specified in <code>newFlags</code> mask, then it will be created. *                   
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_alter_index(
      int session, string tableName, string fieldName, 
      FieldFlags newFlags);

    /*=====================================================================
     * cli_set_error_handler
     *     Set FastDB erro handler. Handler should be no-return function which perform stack unwind.
     * Parameters:
     *     session   - session descriptor as returned by cli_open
     *     handler   - error handler
     * Returns:
     *     previous handler
     */
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void CliErrorHandler(int error, 
      [MarshalAs(UnmanagedType.LPStr)] string msg, int msgarg, IntPtr context); 
    
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern CliErrorHandler cli_set_error_handler(int session, CliErrorHandler new_handler, IntPtr context);

    /*=====================================================================
     * cli_freeze
     *    Freeze cursor. Make it possible to reused cursor after commit of the current transaction.
     * Parameters:
     *     statement   - statememt descriptor returned by cli_statement
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_freeze(int statement);

    /*=====================================================================
     * cli_unfreeze
     *    Unfreeze cursor. Reuse previously frozen cursor.
     * Parameters:
     *     statement   - statememt descriptor returned by cli_statement
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_unfreeze(int statement);


    /*=====================================================================
     * cli_attach
     *    Attach thread to the database. Each thread except one opened the database should first
     *    attach to the database before any access to the database, and detach after end of the work with database
     * Parameters:
     *     session - session descriptor returned by cli_open
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_attach(int session);

    /*=====================================================================
     * cli_detach
     *    Detach thread from the database. Each thread except one opened the database should perform 
     *    attach to the database before any access to the database, and detach after end of the work with database
     * Parameters:
     *     session - session descriptor returned by cli_open
     *     detach_mode - bit mask representing detach mode
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [Flags]
      public enum CliDetachMode : int {
      cli_commit_on_detach          = 1,
      cli_destroy_context_on_detach = 2
    };

    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_detach(int session, CliDetachMode detach_mode);


    /*=====================================================================
     * cli_free_memory
     *    Free memory allocated by cli_show_tables and cli_describe
     * Parameters:
     *     session - session descriptor returned by cli_open
     *     ptr - pointer to the allocated buffer
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern unsafe void cli_free_memory(int session, void* ptr);

    [ StructLayout( LayoutKind.Sequential, CharSet=CharSet.Ansi )]
      public struct CliDatabaseMonitor {
      public int n_readers;
      public int n_writers;
      public int n_blocked_readers;
      public int n_blocked_writers;
      public int n_users;
    }; 

    /*=====================================================================
     * cli_get_database_state
     *    Obtain information about current state of the database
     * Parameters:
     *     session - session descriptor returned by cli_open
     *     monitor - pointer to the monitor structure. The folloing fields are set:
     *       n_readers: number of granted shared locks
     *       n_writers: number of granted exclusive locks
     *       n_blocked_reader: number of threads which shared lock request was blocked
     *       n_blocked_writers: number of threads which exclusive lock request was blocked
     *       n_users: number of processes openned the database
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_get_database_state(int session, ref CliDatabaseMonitor monitor);

    /*=====================================================================
     * cli_set_trace_function
     *    Set trace function which will be used to output FastDB trace messages
     * Parameters:
     *     func - pointer to trace function which receives trace message terminated with new line character
     */
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void CliTraceFunction(string msg);

    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern void cli_set_trace_function(CliTraceFunction func);

    /*=====================================================================
     * cli_prepare_query
     *     Prepare SubSQL query statement. 
     * Parameters:
     *     session - session descriptor returned by cli_open
     *     query   - query string with optional parameters. Parameters are specified
     *               as '%T' where T is one or two character code of parameter type using the same notation
     *               as in printf: %d or %i - int, %f - float or double, %ld - int8, %s - string, %p - oid...
     * Returns:
     *     >= 0 - statement descriptor
     *     <  0 - error code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_prepare_query(int session, string query);

    /* 
     * cli_execute_query
     *     Execute query previously prepared by cli_prepare_query
     * Parameters:
     *     statement - statement descriptor returned by cli_prepare_query
     *     for_update - not zero if fetched rows will be updated 
     *     record_struct - structure to receive selected record fields
     *     ...     - varying list of query parameters
     * Returns:
     *     >= 0 - success, for select statements number of fetched rows is returned
     *     <  0 - error code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_execute_query(
      int          statement, 
      QueryType    queryType, 
      IntPtr       record_struct, // void*
      params object[] list);

    /*
     * cli_insert_struct
     *     Insert new record represented as C structure
     * Parameters:
     *     session - session descriptor returned by cli_open
     *     table_name - name of the destination table
     *     record_struct - structure specifying value of record fields
     *     oid - pointer to the location to receive OID of created record (may be NULL)
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_insert_struct(
      int session, 
      string table_name, 
      IntPtr record_struct, // void*
      ref uint oid);

    /**
     * cli_lock
     *     Set exclusive database lock
     * Parameters:
     *     session - session descriptor returned by cli_open
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_lock(int session);

    /**
     * cli_backup
     *     Perform database backup
     * Parameters:
     *     session - session descriptor returned by cli_open
     *     file_name - backup file name
     *     compatify - if true then databae will be compactified during backup - 
     *                 i.e. all used objects will be placed together without holes; if false then 
     *                 backup is performed by just writting memory mapped object to the backup file.
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_backup(int session, string file_name, int compactify);
    
    /**
     * cli_backup
     *     Schedule database backup
     * Parameters:
     *     session - session descriptor returned by cli_open
     *     file_name -  path to backup file. If name ends with '?', then
     *                  each backup willbe placed in seprate file with '?' replaced with current timestamp
     *     period - period of performing backups in seconds
     * Returns:
     *     result code as described in cli_result_code enum
     */
    [DllImport(libname,
       CharSet           = CharSet.Ansi,  // We want ANSI String
       CallingConvention = CallingConvention.Cdecl)]
    internal static extern int cli_schedule_backup(int session, string file_name, int period);
  }
}
