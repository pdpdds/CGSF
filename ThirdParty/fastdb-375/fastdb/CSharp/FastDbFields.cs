using System;
using System.Text;
using System.Collections;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace FastDbNet
{
  //-------------------------------------------------------------------------
  /// <summary>
  /// FastDbBuffer encapsulates a binding between a memory buffer and an 
  /// object used to get/set data in the buffer.
  /// </summary>
  public class FastDbBuffer: IDisposable {
    public static readonly int MIN_CAPACITY = ALIGN(30);

    internal static int ALIGN(int size) { return size + size % Marshal.SizeOf(typeof(Int64)); }

    public IntPtr buffer;
    public string name;
    public int    flags;  // cli_field_flags 
    public int    bound_to_statement = -1;

    // Make sure the delegate stays alive while in unmanaged code
    internal unsafe static CLI.CliColumnGetEx doGetColumn = new CLI.CliColumnGetEx(GetColumn);
    internal unsafe static CLI.CliColumnSetEx doSetColumn = new CLI.CliColumnSetEx(SetColumn);

    private bool disposed;

    public unsafe FastDbBuffer(string name, CLI.FieldType var_type, CLI.FieldFlags idx_flags) {
      buffer            = Marshal.AllocCoTaskMem(Marshal.SizeOf(typeof(CLI.UnmanagedBuffer)));
      ((CLI.UnmanagedBuffer*)buffer)->fetch_data = true;
      ((CLI.UnmanagedBuffer*)buffer)->type       = (int)var_type;
      ((CLI.UnmanagedBuffer*)buffer)->size       = CLI.SizeOfCliType[(int)var_type];
      ((CLI.UnmanagedBuffer*)buffer)->capacity   = MIN_CAPACITY;
      ((CLI.UnmanagedBuffer*)buffer)->data       = Marshal.AllocCoTaskMem(MIN_CAPACITY);

      this.name  = name; //Marshal.StringToHGlobalAnsi(name);
      this.flags = (int)idx_flags;

      disposed = false;
    }

    // Use C# destructor syntax for finalization code.
    // This destructor will run only if the Dispose method does not get called.
    // It gives the base class the opportunity to finalize.
    ~FastDbBuffer() {
      // Do not re-create Dispose clean-up code here.
      // Calling Dispose(false) is optimal in terms of
      // readability and maintainability.
      Dispose(false);
    }

    internal unsafe static IntPtr GetColumn(int var_type, IntPtr var_ptr, ref int len, 
      string column_name, int statement, 
      void* user_data) 
    {
      CLI.UnmanagedBuffer* buffer = (CLI.UnmanagedBuffer*)user_data;
      if (CLI.IsArrayType((CLI.FieldType)var_type)) 
        len = buffer->size / CLI.SizeOfCliType[var_type - (int)CLI.FieldType.cli_array_of_oid];
      else if (var_type == (int)CLI.FieldType.cli_asciiz || var_type == (int)CLI.FieldType.cli_pasciiz) 
        len = buffer->size-1;
      else 
        len = buffer->size;

      return buffer->data;
    }

    internal unsafe static IntPtr SetColumn(int var_type, 
      IntPtr var_ptr, int len, string column_name,
      int statement, IntPtr data_ptr, 
      void* user_data) 
    {
      CLI.UnmanagedBuffer* buffer = (CLI.UnmanagedBuffer*)user_data;
      if (var_type == (int)CLI.FieldType.cli_asciiz || var_type == (int)CLI.FieldType.cli_pasciiz) {
        SetBufferTypeAndSize(buffer, (CLI.FieldType)var_type, len, true);
        return buffer->data;
      }
      else if (CLI.IsArrayType((CLI.FieldType)var_type)) {
        if (buffer->fetch_data) {
          SetBufferTypeAndSize(buffer, (CLI.FieldType)var_type, 
            len*CLI.SizeOfCliType[(int)var_type - (int)CLI.FieldType.cli_array_of_oid], true);
          return buffer->data;
        }
        else
          return IntPtr.Zero;        // FastDB won't fetch a field if we return nil
      }
      else // sanity check
        throw new CliError("Unsupported type: "+Enum.GetName(typeof(CLI.FieldType), buffer->type));
    }

    protected unsafe static void SetBufferTypeAndSize(CLI.UnmanagedBuffer* buf, 
      CLI.FieldType NewType, int NewSize, bool TypeCheck) 
    {
      int n;

      if (!TypeCheck || CLI.IsArrayType(NewType))
        n = NewSize;
      else if (NewType == CLI.FieldType.cli_asciiz || NewType == CLI.FieldType.cli_pasciiz)
        n = NewSize+1;
      else
        n = CLI.SizeOfCliType[(int)NewType];

      if (n > buf->capacity) {
          buf->data = Marshal.ReAllocCoTaskMem(buf->data, ALIGN(n));
          buf->capacity = n;
      }

      buf->size = n;

      if (buf->type != (int)NewType)
        buf->type = (int)NewType;
    }

    /// <summary>
    /// Use Assign() method to copy the content of on FastDbBuffer to another.
    /// </summary>
    /// <param name="var">is the source FastDbBuffer to be copied from.</param>
    public unsafe virtual void Assign(FastDbBuffer var) {
      this.name = var.Name;
      this.Capacity = var.Capacity;
      this.bound_to_statement = var.StatementID;
      this.FetchData = var.FetchData;
      CopyBufferData(var.Type, var.Size, ((CLI.UnmanagedBuffer*)var.buffer.ToPointer())->data);
    }

    internal unsafe void CopyBufferData(CLI.FieldType type, int size, IntPtr data) 
    {
      SetBufferTypeAndSize((CLI.UnmanagedBuffer*)buffer.ToPointer(), type, size, true);
      Int64* pend  = (Int64*)((byte*)data.ToPointer() + ALIGN(size));
      Int64* pfrom = (Int64*)data.ToPointer();
      Int64* pto   = (Int64*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer();
      while(pfrom < pend)
        *pto++ = *pfrom++;
      // for(int i=0; i < size; i++)
      //   *pto++ = *pfrom++;
      // Note: buffers are always aligned to 8 bytes, so we can simply copy by 
      // iterating over 8 byte integers.
      //if (type == CLI.FieldType.cli_asciiz || type == CLI.FieldType.cli_pasciiz)
      //  *pto = (byte)'\0';
    }

    /// <summary>
    /// Bind() abstract method binds a FastDbBuffer to a FastDb statement obtained by calling
    /// cli_statement() API function.  The descendants are requiered to override this method.
    /// </summary>
    /// <param name="StatementID">integer value representing a FastDB statement</param>
    internal unsafe virtual void Bind(int StatementID) {} // bound_to_statement = StatementID; }
    /// <summary>
    /// Unbind() clears the buffer binding to a statement.
    /// </summary>
    internal virtual unsafe void UnBind() { bound_to_statement = -1; }  // Note: since 0 is a valid statement we initialize this to a negative value

    /// <summary>
    /// Returns the name of this field/parameter.
    /// </summary>
    public unsafe string         Name         { get { return name; } }
    /// <summary>
    /// Capacity controls internally allocated memory for the buffer.  
    /// By default minimum size of the buffer is 30 bytes, however, if you are
    /// reading/writing large arrays or string values, you may consider setting 
    /// the Capacity field to a larger value in order to minimize memory reallocations.
    /// </summary>
    public unsafe int            Capacity     { get { return ((CLI.UnmanagedBuffer*)buffer.ToPointer())->capacity; } set { ((CLI.UnmanagedBuffer*)buffer.ToPointer())->capacity = value; } }
    /// <summary>
    /// Determines the size in bytes of current value in the buffer.
    /// </summary>
    public unsafe int            Size         { get { return ((CLI.UnmanagedBuffer*)buffer.ToPointer())->size; } }
    /// <summary>
    /// Returns the type of the value in the buffer.
    /// </summary>
    public unsafe CLI.FieldType  Type         { get { return (CLI.FieldType)((CLI.UnmanagedBuffer*)buffer.ToPointer())->type; } }   
    /// <summary>
    /// Contains index flags indicating the types of indexes (hash/T-tree) on this field.
    /// </summary>
    public unsafe CLI.FieldFlags Flags        { get { return (CLI.FieldFlags)flags; } }  
    /// <summary>
    /// Internal statement's ID that this buffer is bound to. 
    /// </summary>
    public unsafe int            StatementID  { get { return bound_to_statement; } }
    /// <summary>
    /// For array fields FetchData controls whether to copy the field's content
    /// to the buffer when a cursor is moved to another record.  By setting this 
    /// value to false, it may increase performance on queries that don't need to use
    /// the content of the array field. 
    /// </summary>
    public unsafe bool           FetchData    { get { return ((CLI.UnmanagedBuffer*)buffer.ToPointer())->fetch_data; } set { ((CLI.UnmanagedBuffer*)buffer.ToPointer())->fetch_data = value; } }
      
    public bool   asBoolean {get {return (bool)getValue(typeof(bool));}     set {setValue(value);}}  
    public Int16  asInt16   {get {return (Int16)getValue(typeof(Int16));}   set {setValue(value);}}  
    public int    asInteger {get {return (int)getValue(typeof(int));}       set {setValue(value);}}  
    public uint   asOID     {get {return (uint)getValue(typeof(uint));}     set {setValue(value);}}  
    public Int64  asInt64   {get {return (Int64)getValue(typeof(Int64));}   set {setValue(value);}}  
    public double asDouble  {get {return (double)getValue(typeof(double));} set {setValue(value);}}
    public string asString  {get {return (string)getValue(typeof(string));} set {setValue(value);}}
    public byte[] asByteArray {get {return (byte[])getValue(typeof(byte[]));} set {setValue(value);}}

    protected unsafe virtual Object getValue(Type t) {
      switch((CLI.FieldType)((CLI.UnmanagedBuffer*)buffer.ToPointer())->type) {
        case CLI.FieldType.cli_bool: 
        case CLI.FieldType.cli_int1:    return Convert.ChangeType(*(sbyte*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer(), t);
        case CLI.FieldType.cli_int2:    return Convert.ChangeType(*(short*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer(), t);
        case CLI.FieldType.cli_oid:     return Convert.ChangeType(*(uint*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer(), t);
        case CLI.FieldType.cli_int4:    return Convert.ChangeType(*(int*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer(), t);
        case CLI.FieldType.cli_int8:    return Convert.ChangeType(*(Int64*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer(), t);
        case CLI.FieldType.cli_real4:   return Convert.ChangeType(*(Single*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer(), t);
        case CLI.FieldType.cli_datetime:
        case CLI.FieldType.cli_real8:   return Convert.ChangeType(*(double*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer(), t);
        case CLI.FieldType.cli_asciiz:
        case CLI.FieldType.cli_pasciiz: return Convert.ChangeType(Marshal.PtrToStringAnsi(((CLI.UnmanagedBuffer*)buffer.ToPointer())->data), t);
        case CLI.FieldType.cli_array_of_int1:
            if (t == typeof(byte[])) { 
                int len = Size;
                byte[] arr = new byte[len];
                byte* src = (byte*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer();
                for (int i = 0; i < len; i++) {
                    arr[i] = *src++;
                }
                return arr;
            } else { 
                throw new CliError("getValue: Unsupported conversion type! "+Enum.GetName(typeof(CLI.FieldType), ((CLI.UnmanagedBuffer*)buffer.ToPointer())->type));
            }
        default:                        throw new CliError("getValue: Unsupported type!"+Enum.GetName(typeof(CLI.FieldType), ((CLI.UnmanagedBuffer*)buffer.ToPointer())->type));
      }
    }

    protected unsafe void setValue(Object Value) {
      switch((CLI.FieldType)((CLI.UnmanagedBuffer*)buffer.ToPointer())->type) {
        case CLI.FieldType.cli_oid:
          *(uint*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer() = Convert.ToUInt32(Value);
          break;
        case CLI.FieldType.cli_int4:
          *(int*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer() = Convert.ToInt32(Value);
          break;
        case CLI.FieldType.cli_bool:
        case CLI.FieldType.cli_int1:
          *(sbyte*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer() = Convert.ToSByte(Value);
          break;
        case CLI.FieldType.cli_int2:
          *(Int16*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer() = Convert.ToInt16(Value);
          break;
        case CLI.FieldType.cli_int8:
          *(Int64*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer() = Convert.ToInt64(Value);
          break;
        case CLI.FieldType.cli_real4:
          *(Single*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer() = Convert.ToSingle(Value);
          break;
        case CLI.FieldType.cli_datetime:
        case CLI.FieldType.cli_real8:
          *(double*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer() = Convert.ToDouble(Value);
          break;
        case CLI.FieldType.cli_asciiz:
        case CLI.FieldType.cli_pasciiz:
          //byte[] bytes = Encoding.Convert(Encoding.Unicode, Encoding.ASCII, Encoding.Unicode.GetBytes(Value.ToString()));
          //cli_ex_buffer_resize(buffer, buffer.type, bytes.Length, true);
          //buffer.data = Marshal.AllocCoTaskMem( bytes.Length );
          //Marshal.
          //Marshal.Copy(bytes, 0, buffer.data, bytes.Length);
          //fixed(char* p = &bytes[0]) 
          //{
          string s   = Value.ToString();
          IntPtr str = Marshal.StringToHGlobalAnsi(s);
          try {
            CopyBufferData((CLI.FieldType)((CLI.UnmanagedBuffer*)buffer.ToPointer())->type, s.Length, str);
          } 
          finally {
            Marshal.FreeCoTaskMem(str);
          }
          break;
        case CLI.FieldType.cli_array_of_int1:
            if (Value is byte[]) { 
                byte[] arr = (byte[])Value;
                int len = arr.Length;
                SetBufferTypeAndSize((CLI.UnmanagedBuffer*)buffer.ToPointer(), CLI.FieldType.cli_array_of_int1, len, false);
                byte* dst = (byte*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer();
                for (int i = 0; i < len; i++) {
                   *dst++ = arr[i];
                }
                break;
            } else { 
                throw new CliError("getValue: Unsupported conversion type! "+Enum.GetName(typeof(CLI.FieldType), ((CLI.UnmanagedBuffer*)buffer.ToPointer())->type));
            }
        default:
          throw new CliError("Unsupported type: "+Enum.GetName(typeof(CLI.FieldType), (CLI.FieldType)((CLI.UnmanagedBuffer*)buffer.ToPointer())->type));
      }
    }

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
    protected unsafe virtual void Dispose(bool disposing) {
      // Check to see if Dispose has already been called.
      if(!this.disposed) {
        // If disposing equals true, dispose managed resources.
        if(disposing) {  // Dispose managed resources here.
          Marshal.FreeCoTaskMem(((CLI.UnmanagedBuffer*)buffer.ToPointer())->data);
          ((CLI.UnmanagedBuffer*)buffer.ToPointer())->data = (IntPtr)0;
          Marshal.FreeCoTaskMem(buffer);
        }
        // Release unmanaged resources. 
        //CLI.cli_ex_buffer_free(buffer);
        disposed = true;
      }
    }
    #region IEnumerable Members

    public IEnumerator GetEnumerator() {
      // TODO:  Add FastDbBuffer.GetEnumerator implementation
      return null;
    }

    #endregion
  }


  //-------------------------------------------------------------------------
  /// <summary>
  /// FastDbField implements a class that automatically manages the memory 
  /// associated with a field belonging to a database cursor.
  /// </summary>
  public class FastDbField: FastDbBuffer {
    public string RefTable;
    public string InvRefField;

    /// <summary>
    /// Field's constructor.
    /// </summary>
    /// <param name="name">Field name</param>
    /// <param name="field_type">Field type</param>
    /// <param name="idx_flags">Index types (hash/T-tree)</param>
    /// <param name="ref_table">Reference table name for inverse reference fields</param>
    /// <param name="inv_ref_field">Inverse reference field name</param>
    public FastDbField(string name, CLI.FieldType field_type, CLI.FieldFlags idx_flags, 
      string ref_table, string inv_ref_field) :
      base(name, field_type, idx_flags) {
      this.RefTable = ref_table;
      this.InvRefField = inv_ref_field;
    }

    /// <summary>
    /// Copy field's content to the current field
    /// </summary>
    /// <param name="field">Source field to copy from</param>
    public override void Assign(FastDbBuffer field) {
      Debug.Assert(field is FastDbField, "Cannot assign " + field.GetType().Name + " type!");
      base.Assign(field);
      this.RefTable = ((FastDbField)field).RefTable;
      this.InvRefField = ((FastDbField)field).InvRefField;
    }

    /// <summary>
    /// Is true if the field is of array type.
    /// </summary>
    public bool IsArray { get { return CLI.IsArrayType(Type); } }

    /// <summary>
    /// Returns the number of array elements for array type fields
    /// </summary>
    public unsafe int ArraySize {
      get {
        if (CLI.IsArrayType(this.Type)) {
          return Size / CLI.SizeOfCliType[(int)this.Type - (int)CLI.FieldType.cli_array_of_oid];
        } else 
          return 0;
      }
      set {
        Debug.Assert(CLI.IsArrayType(this.Type), "Cannot set array size on non-array fields: "+Enum.GetName(typeof(CLI.FieldType), Type));
        int n = CLI.SizeOfCliType[(int)this.Type - (int)CLI.FieldType.cli_array_of_oid] * value;
        if (Size != n)
          SetBufferTypeAndSize((CLI.UnmanagedBuffer*)buffer.ToPointer(), this.Type, n, true);
      }
    }

    public bool   ArrayAsBoolean(int idx) { return (bool)getArrayValue(typeof(bool),     idx);}
    public Int16  ArrayAsInt16(int idx)   { return (Int16)getArrayValue(typeof(Int16),   idx);}  
    public uint   ArrayAsOID(int idx)     { return (uint)getArrayValue(typeof(uint),     idx);}    
    public Int64  ArrayAsInt64(int idx)   { return (Int64)getArrayValue(typeof(Int64),   idx);}  
    public double ArrayAsDouble(int idx)  { return (double)getArrayValue(typeof(double), idx);}
    public string ArrayAsString(int idx)  { return (string)getArrayValue(typeof(string), idx);}

    protected unsafe Object getArrayValue(Type t, int idx) {
      switch((CLI.FieldType)((CLI.UnmanagedBuffer*)buffer.ToPointer())->type) {
        case CLI.FieldType.cli_array_of_bool:  
        case CLI.FieldType.cli_array_of_int1:  return Convert.ChangeType(*((sbyte*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer()+sizeof(sbyte)*idx), t);
        case CLI.FieldType.cli_array_of_int2:  return Convert.ChangeType(*((short*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer()+sizeof(short)*idx), t);
        case CLI.FieldType.cli_array_of_oid:   return Convert.ChangeType(*((uint*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer()+sizeof(uint)*idx), t);
        case CLI.FieldType.cli_array_of_int4:  return Convert.ChangeType(*((int*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer()+sizeof(int)*idx), t);
        case CLI.FieldType.cli_array_of_int8:  return Convert.ChangeType(*((Int64*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer()+sizeof(Int64)*idx), t);
        case CLI.FieldType.cli_array_of_real4: return Convert.ChangeType(*((Single*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer()+sizeof(Single)*idx), t);
        case CLI.FieldType.cli_array_of_real8: return Convert.ChangeType(*((double*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer()+sizeof(double)*idx), t);
        default:                               throw new CliError("Unsupported type!");
      }
    }

    /// <summary>
    /// This method adds functionality to the base class that allows to get 
    /// the content of an array field as string.
    /// </summary>
    /// <param name="t"></param>
    /// <returns></returns>
    protected override unsafe Object getValue(Type t) {
        if (t == typeof(string)) {
            switch((CLI.FieldType)((CLI.UnmanagedBuffer*)buffer.ToPointer())->type) {
                case CLI.FieldType.cli_array_of_int1:
                case CLI.FieldType.cli_array_of_int2:
                case CLI.FieldType.cli_array_of_int4:
                case CLI.FieldType.cli_array_of_int8:
                case CLI.FieldType.cli_array_of_oid:
                case CLI.FieldType.cli_array_of_real4:
                case CLI.FieldType.cli_array_of_real8:
                  StringBuilder s = new StringBuilder("{");
                  for(int i=0; i < ArraySize; i++) {
                    s.Append(ArrayAsString(i));
                    s.Append( (i == ArraySize-1) ? "" : "," );
                  }
                  s.Append("}");
                  return s.ToString();
            }
        }
        return base.getValue(t);
    }
  
    /// <summary>
    /// Set a value of an array element. <seealso cref="CLI.FieldType"/>
    /// </summary>
    /// <param name="idx">Element index</param>
    /// <param name="Value">Element's new value</param>
    public unsafe void SetArrayValue(int idx, Object Value) {
            Debug.Assert(idx >= 0 && idx < ArraySize, "Array index " + idx + " out of bounds!");

            switch((CLI.FieldType)((CLI.UnmanagedBuffer*)buffer.ToPointer())->type) {
              case CLI.FieldType.cli_array_of_oid:
                *((uint*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer()+sizeof(uint)*idx) = Convert.ToUInt32(Value);
                break;
              case CLI.FieldType.cli_array_of_int4:
                *((int*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer()+sizeof(int)*idx) = Convert.ToInt32(Value);
                break;
              case CLI.FieldType.cli_array_of_bool:
              case CLI.FieldType.cli_array_of_int1:
                *((sbyte*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer()+sizeof(sbyte)*idx) = Convert.ToSByte(Value);
                break;
              case CLI.FieldType.cli_array_of_int2:
                *((Int16*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer()+sizeof(Int16)*idx) = Convert.ToInt16(Value);
                break;
              case CLI.FieldType.cli_array_of_int8:
                *((Int64*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer()+sizeof(Int64)*idx) = Convert.ToInt64(Value);
                break;
              case CLI.FieldType.cli_array_of_real4:
                *((Single*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer()+sizeof(Single)*idx) = Convert.ToSingle(Value);
                break;
              case CLI.FieldType.cli_array_of_real8:
                *((double*)((CLI.UnmanagedBuffer*)buffer.ToPointer())->data.ToPointer()+sizeof(double)*idx) = Convert.ToDouble(Value);
                break;
              default:
                throw new CliError("Unsupported type!");
            }
          }

    internal unsafe override void Bind(int StatementID) {
      int res;
      base.Bind(StatementID);
      
      if (bound_to_statement == StatementID) 
        return;
      else
        bound_to_statement = StatementID;

      //IntPtr pname = Marshal.StringToHGlobalAnsi(Name);
      try {
        if (CLI.IsArrayType(Type) || Type == CLI.FieldType.cli_asciiz || Type == CLI.FieldType.cli_pasciiz)
          res = CLI.cli_array_column_ex(StatementID, name,
            (Type == CLI.FieldType.cli_pasciiz) ? (int)CLI.FieldType.cli_asciiz : (int)Type,
            ((CLI.UnmanagedBuffer*)buffer.ToPointer())->data, 
            doSetColumn, doGetColumn, (CLI.UnmanagedBuffer*)buffer.ToPointer());
        else
          res = CLI.cli_column(StatementID, name, (int)Type, ref ((CLI.UnmanagedBuffer*)buffer.ToPointer())->size, ((CLI.UnmanagedBuffer*)buffer.ToPointer())->data);
        CLI.CliCheck(res);
      }
      finally {
        //Marshal.FreeCoTaskMem(name);
      }
    }
  }


  //-------------------------------------------------------------------------
  /// <summary>
  /// FastDbParameter implements a class that automatically manages the memory
  /// associated with a parameter associated with a database statement.
  /// </summary>
  public class FastDbParameter: FastDbBuffer {
    /// <summary>
    /// Constructor that creates an instance of the FastDbField.  You would 
    /// normally not use it directly, but by invoking: <see cref="FastDbFields.Add"/> method.
    /// </summary>
    /// <param name="name">The name of the field.</param>
    /// <param name="var_type">The type of the field.</param>
    /// <param name="capacity">Optional capacity of the field's buffer.</param>
    public FastDbParameter(string name, CLI.FieldType var_type): this(name, var_type, 0) {}
    public FastDbParameter(string name, CLI.FieldType var_type, int capacity) : 
      base(name, var_type, CLI.FieldFlags.cli_noindex) {}

    public override void Assign(FastDbBuffer var) {
      Debug.Assert(var is FastDbParameter, "Cannot assign " + var.GetType().Name + " type!");
      base.Assign(var);
    }

    internal unsafe override void Bind(int StatementID) {
      int res;
      base.Bind(StatementID);
      
      if (bound_to_statement == StatementID) 
        return;
      else
        bound_to_statement = StatementID;

      if (CLI.IsArrayType(Type))
        res = (int)CLI.ErrorCode.cli_not_implemented;  // Array variables are not supported!
      else {
        string s = "%" + Name;
        res = CLI.cli_parameter(StatementID, s, (int)Type, ((CLI.UnmanagedBuffer*)buffer.ToPointer())->data);
      }
      CLI.CliCheck(res);
    }
  }

  
  //-------------------------------------------------------------------------
  /// <summary>
  /// FastDbCollection is the base class for FastDbFields and FastDbParameters.  It 
  /// implements a collection of fields/parameters used to store 
  /// FastDbField/FastDbParameter objects.
  /// </summary>
  public abstract class FastDbCollection: IEnumerable {
    protected ArrayList items = new ArrayList();
    
    /// <summary>
    /// Find an item by index.
    /// </summary>
    protected FastDbBuffer this[int index]   { get { return (FastDbBuffer)items[index]; } }

    /// <summary>
    /// Find an item by name.
    /// </summary>
    protected FastDbBuffer this[string name] { 
      get { 
        for(int i=0; i<items.Count; i++) 
          if (String.Compare(name, ((FastDbBuffer)items[i]).Name, true) == 0)
            return (FastDbBuffer)items[i];
        throw new CliError("Parameter \"" + name + "\" not found!");
      } 
    }

    protected abstract FastDbBuffer Add(FastDbBuffer item);

    /// <summary>
    /// Get a count of items in the collection.
    /// </summary>
    public int  Count   { get { return items.Count; } }
    /// <summary>
    /// Clear the collection by removing all items.
    /// </summary>
    public void Clear() { UnBind(); items.Clear(); }
    /// <summary>
    /// Copy the contents of one collection to another.
    /// </summary>
    /// <param name="items">Source collection to copy items from.</param>
    public virtual void Assign(FastDbCollection items) {
      Clear();
      for(int i=0; i<items.Count; ++i) this.Add(items[i]);
    }

    /// <summary>
    /// Bind all fields/Parameters in the collection to a statement.
    /// </summary>
    /// <param name="statement"></param>

    internal void Bind(int statement) {
      for(int i=0; i<items.Count; ++i) this[i].Bind(statement);
    }
    /// <summary>
    /// Unbind all fields from a statement.
    /// </summary>
    internal void UnBind() {
      for(int i=0; i<items.Count; ++i) this[i].UnBind();
    }
    
    #region IEnumerable Members

    IEnumerator IEnumerable.GetEnumerator() { return GetEnumerator(); }
    public Iterator GetEnumerator()         { return new Iterator(this); }

    /// <summary>
    /// This class implements IEnumerator interface. Use its methods within a 
    /// foreach() loop.  Iterator.Index tells the field's index within a collection
    /// and Iterator.IsLast is true if this is the last item.
    /// </summary>
    public class Iterator: IEnumerator {
      #region Private Fields
      int nIndex;
      FastDbCollection collection;
      #endregion
      #region Constructors
      public Iterator(FastDbCollection coll) {
        collection = coll;
        nIndex = -1;
      }
      #endregion
      #region Methods
      public void Reset()         { nIndex = -1; }
      public bool MoveNext()      { return ++nIndex < collection.items.Count; }
      public FastDbBuffer Current { get { return (FastDbBuffer)collection.items[nIndex]; } }
      public int  Index           { get { return nIndex; } }
      public bool IsLast          { get { return nIndex == collection.items.Count-1; } }
      // The current property on the IEnumerator interface:
      object IEnumerator.Current  { get { return Current; } }
      #endregion
    }
    #endregion
  }

  //-------------------------------------------------------------------------
  /// <summary>
  /// A collection of fields belonging to a FastDbTable.  It is usually accessed
  /// by calling <see cref="FastDbCommand.Fields"/> property.
  /// </summary>
  public class FastDbFields: FastDbCollection {
    protected override FastDbBuffer Add(FastDbBuffer item) { return this.Add((FastDbField)item); }
   
    public FastDbField Add(FastDbField field) { 
      return Add(field.Name, field.Type, field.Flags, field.RefTable, field.InvRefField); 
    }
    public FastDbField Add(string name, CLI.FieldType field_type) { 
      return Add(name, field_type, CLI.FieldFlags.cli_noindex, null, null); 
    }
    public FastDbField Add(string name, CLI.FieldType field_type, CLI.FieldFlags index_type) {
      return Add(name, field_type, index_type, null, null);
    }
    public FastDbField Add(string name, CLI.FieldType field_type, CLI.FieldFlags index_type, string ref_table) {
      return Add(name, field_type, index_type, ref_table, null);
    }
    /// <summary>
    /// Use this method to add a field to the collection.
    /// </summary>
    /// <param name="name">Name of the field</param>
    /// <param name="field_type">Type of the field</param>
    /// <param name="index_type">Optional index to be built on the field (default: CLI.FieldFlags.no_index)</param>
    /// <param name="ref_table">Reference table name (used for inverse references). Default: null.</param>
    /// <param name="inv_ref_field">Inverse reference field name (default: null)</param>
    /// <returns></returns>
    public FastDbField Add(string name, CLI.FieldType field_type, CLI.FieldFlags index_type, string ref_table, string inv_ref_field) {
      int i = items.Add(new FastDbField(name, field_type, index_type, ref_table, inv_ref_field));
      return (FastDbField)items[i];
    }
    /// <summary>
    /// Get a field by index.
    /// </summary>
    public new FastDbField this[int index]   { get { return (FastDbField)base[index]; } }
    /// <summary>
    /// Get a field by name.
    /// </summary>
    public new FastDbField this[string name] { get { return (FastDbField)base[name]; } }
  }

  /// <summary>
  /// A collection of Parameters defined in a FastDb command.
  /// <code>
  /// FastDbCommand command = 
  ///   new FastDbCommand(connection, "select * from tab where id = %id");
  /// </code>
  /// In the case above, the "id" is a parameter that can be associated with a 
  /// value using:
  /// <code>
  /// command.Parameters.Add("id", CLI.FieldType.cli_int4);
  /// command.Parameters[0].Value = 10;
  /// int num_rows = command.Execute();
  /// </code>
  /// </summary>
  public class FastDbParameters: FastDbCollection {
    protected override FastDbBuffer Add(FastDbBuffer param) {
      return this.Add(param.Name, param.Type, param.Capacity);
    }

    public FastDbParameter Add(string name, CLI.FieldType param_type) {
      Debug.Assert(param_type >= CLI.FieldType.cli_oid && param_type < CLI.FieldType.cli_array_of_oid,
        String.Format("Parameter type {0} not supported!", Enum.GetName(typeof(CLI.FieldType), param_type)));
      return this.Add(name, param_type, 0);
    }
    /// <summary>
    /// Add a parameter to the FastDbParameters collection.
    /// </summary>
    /// <param name="name">Parameter name (as it appears in the SQL statement) without preceeding "%".</param>
    /// <param name="param_type">Parameter type (it must match the type of the associated field).</param>
    /// <param name="capacity">Optional capacity of the underlying memory buffer.</param>
    /// <returns>A object representing the newly created parameter</returns>
    public FastDbParameter Add(string name, CLI.FieldType param_type, int capacity) {
      int i = items.Add(new FastDbParameter(name, param_type, capacity));
      return (FastDbParameter)items[i];
    }
    /// <summary>
    /// Get a parameter by index.
    /// </summary>
    public new FastDbParameter this[int index]   { get { return (FastDbParameter)base[index]; } }
    /// <summary>
    /// Get a parameter by name.
    /// </summary>
    public new FastDbParameter this[string name] { get { return (FastDbParameter)base[name]; } }
  }
}
