#!/usr/local/bin/ruby

require 'socket'

=begin
= FastDB API
  This module contains Ruby API to FastDB
=end
module FastDB


=begin
== Connection to the FastDB server
=end
class Connection
=begin
=== Open connection with server
==== host_address - string with server host name
==== host_port    - integer number with server port
=end                     
    def open(host_address, host_port)
        @socket = TCPSocket.new(host_address, host_port)
        @socket.setsockopt(Socket::IPPROTO_TCP, Socket::TCP_NODELAY, true)
        @n_statements = 0
        loadSchema()
    end

=begin
=== Close connection with server
=end                     
    def close()         
        sendCommand(CliCmdCloseSession)
        @socket.close()
        @socket = nil
    end

=begin
=== Create select statement. 
==== sql - SubSQL select statement with parameters. Parameters should be started with % character. 
     Each used parameter should be set before execution of the statement.
=end
    def createStatement(sql)
        @n_statements += 1
        return Statement.new(self, sql, @n_statements)
    end

=begin
=== Execute select statement 
==== sql - SubSQL select statement with parameters. Parameters should be started with % character. 
==== param - hash specifying qyery parameters
==== for_update - if cursor is opened in for update mode
=end
    def select(sql, params = {}, for_update = false)
        stmt = createStatement(stmt)
        params.each_pair {|key, value| stmt[key] = value}
        return stmt.fetch(for_update)
    end

=begin
=== Commit current transaction
=end
    def commit()
        sendReceiveCommand(CliCmdCommit)
    end
 
=begin
=== Exclusively lock database (FastDB set locks implicitely, explicit exclusive lock may be needed to avoid deadlock caused by lock upgrade)
=end
    def lock()
        sendCommand(CliCmdLock)
    end
 
=begin
=== Release all locks set by the current transaction
=end
    def unlock()
        sendReceiveCommand(CliCmdPrecommit)
    end
 
=begin
=== Rollback curent transaction. All changes made by current transaction are lost.
=end
    def rollback()
        sendReceiveCommand(CliCmdAbort)
    end

=begin
=== Execute block in the context of transaction
=end
    def transaction
      commited = false
      yield
      commit
      commited = true
    ensure
      rollback unless commited
    end

=begin
=== Insert object in the database. There is should be table in the database with
    name equal to the full class name of the inserted object (comparison is 
    case sensitive). FastDB will store to the database all non-static and 
    non-transient fields from the class.
==== obj - object to be inserted in the database
==== table - name of the table in which object should be inserted (by default - table corresponding to the object class)
==== Returns reference to the inserted object
=end
    def insert(obj, table=obj.class.name)
        column_defs=""
        column_values=""
        n_columns=0
        table_desc = @tables[table]
        if obj.is_a?(Struct)
            vars = obj.members
            is_struct=true
        else
            vars = obj.instance_variables
            is_struct=false
        end
        if table_desc.nil?
            raise CliError, "Class #{table} is not found in the database"
        end
        for var in vars
            if is_struct
                field = var            
            else 
                field = var[1...var.length] # ignore @ prefix
            end
            field_desc = table_desc[field]
            if field_desc != nil
                n_columns += 1
                if is_struct
                    value = obj[var]
                else 
		    value = obj.instance_variable_get(var)
                end
                type = value.class 
                if type == Fixnum
                    column_defs << CliInt4 << field << 0
                    column_values << [value].pack("N")
                elsif type == Bignum
                    column_defs << CliInt8 << field << 0
                    column_values << [value >> 32, value & 0xffffffff].pack("NN")
                elsif type == Float
                    column_defs << CliReal8 << field << 0
                    column_values << [value].pack("G")
                elsif type == String
                    if field_desc.type == CliArrayOfInt1
                        column_defs << CliArrayOfInt1 << field << 0
                        column_values << [value.length].pack("N") << value
                    else 
                        column_defs << CliAsciiz << field << 0 
                        column_values << [value.length+1].pack("N") << value << 0
                    end
                elsif type == Reference
                    column_defs << CliOid << field << 0
                    column_values << [value.oid].pack("N")
                elsif type == TrueClass
                    column_defs << CliBool << field << 0
                    column_values << 1
                elsif type == FalseClass 
                    column_defs << CliBool << field << 0
                    column_values << 0
                elsif type == Rectangle
                    column_defs << CliRectangle << field << 0
                    column_values << [value.left,value.top,value.right,value.bottom].pack("NNNN")                     
                elsif type == NilClass
                    case field_desc.type 
                        when CliInt1, CliInt2, CliInt4, CliInt8
                           column_defs << CliInt1 << field << 0
                           column_values << 0
                        when CliReal4, CliReal8
                           column_defs << CliReal4 << field << 0
                           column_values << [0.0].pack("g")
                        when CliAsciiz
                            column_defs << CliAsciiz << field << 0 
                            column_values << [1].pack("N") << 0
                        when CliOid
                            column_defs << CliOid << field << 0 
                            column_values << [0].pack("N")
                        else 
                            raise CliError, "Null value is not allowed for field #{var}"
                    end
                elsif type == Array
                    column_defs << field_desc.type << field << 0
                    column_values << [value.length].pack("N")
                    case field_desc.type
                        when CliArrayOfInt1
                            column_values << value.pack("c*")
                        when CliArrayOfBool
                            for elem in value
                                column_values << elem ? 1 : 0
                            end
                        when CliArrayOfInt2
                            column_values << value.pack("n*")
                        when CliArrayOfInt4
                            column_values << value.pack("N*")
                        when CliArrayOfInt8
                            for elem in value
                                column_values << [elem >> 32, elem & 0xffffffff].pack("NN")
                            end
                        when CliArrayOfReal4
                            column_values << value.pack("g*")
                        when CliArrayOfReal8
                            column_values << value.pack("G*")
                        when CliArrayOfOid
                            for elem in value
                                column_values << [elem.to_i].pack("N")
                            end
                        when CliArrayOfString
                            for elem in value
                                column_values << elem << 0
                            end                   
                        else
                            raise CliError, "Unsupported element type #{field_desc.type}"
                    end     
                else
                    raise CliError, "Unsupported type #{type.name}"
                end
            end
        end

        req = [CliRequestSize + 14 + table.length + column_defs.length + column_values.length, CliCmdPrepareAndInsert, 0].pack("NNN")
        req << "insert into " << table << 0 << n_columns << column_defs << column_values
        @socket.send(req, 0)
        rc=@socket.recv(CliRequestSize).unpack("NNN")
        raiseError(rc[0]) unless rc[0] == CliOk
        Reference.new(rc[2]) if rc[2] != 0
    end

    def loadSchema()
        sendCommand(CliCmdShowTables)        
        ret = @socket.recv(8).unpack("NN")
        len = ret[0]
        n_tables = ret[1]
        table_names = @socket.recv(len)
        @tables = {}
        j = 0
        for i in 0...n_tables
            k = table_names.index(0, j)
            table = table_names[j...k]
            j = k + 1
            if Object::const_defined?(table)
                cls=Object::const_get(table)
            else
                cls=Class::new
                Object::const_set(table, cls)
            end
            @socket.send([CliRequestSize + table.length + 1, CliCmdDescribeTable, 0].pack("NNN") << table << 0, 0)
            ret = @socket.recv(8).unpack("NN")
            len = ret[0]
            n_fields = ret[1]
            field_info = @socket.recv(len)
            fields = Array.new(n_fields)
            j = 0
            for k in 0...n_fields
                type = field_info[j]
                j += 1
                flags = field_info[j]
                j += 1

                z = field_info.index(0, j)
                name = field_info[j...z]
                j = z + 1

                z = field_info.index(0, j)
                if z != j
                    ref_table = field_info[j...z]
                else
                    ref_table = nil
                end
                j = z + 1

                z = field_info.index(0, j)
                if z != j
                    inverse_field = field_info[j...z]
                else
                    inverse_field = nil
                end
                j = z + 1

                fields[k] = FieldDescriptor.new(name, ref_table, inverse_field, type, flags)
            end
            @tables[cls.name] = TableDescriptor.new(cls, fields)
        end
    end

    def sendCommand(cmd, id=0)
        @socket.send([CliRequestSize, cmd, id].pack("NNN"), 0)
    end

    def receive(len)
        return @socket.recv(len)
    end 

    def sendReceiveCommand(cmd, id=0)
        sendCommand(cmd, id)
        rc = @socket.recv(4).unpack("N")[0].hash
        raiseError(rc, "Send command failed: ") if rc < 0
        return rc
    end

    def sendReceiveRequest(req)
        @socket.send(req, 0)
        rc = @socket.recv(4).unpack("N")[0].hash
        raiseError(rc, "Send request failed: ") if rc < 0
        return rc
    end
        
=begin
=== Close connection with server
=end                     
    def close() 
        sendCommand(CliCmdCloseSession)
        @socket.close()
        @socket = nil
    end

    def raiseError(error_code, prefix = "")
      raise CliError, "#{prefix}#{ERROR_DESCRIPTIONS[error_code]}" if ERROR_DESCRIPTIONS[error_code]
      raise CliError, "#{prefix}Unknown error code #{error_code}"
    end


    CliRequestSize = 12
    AtChar = 64

=begin
== Field flag
=end
    CliHashed           = 1  # field should be indexed usnig hash table
    CliIndexed          = 2  # field should be indexed using B-Tree
    CliCascadeDelete    = 8   # perfrom cascade delete for for reference or array of reference fields
    CliAutoincremented  = 16 # field is assigned automaticall incremented value

=begin
== Operation result codes
=end
    CliOk=0
    CliBadAddress = -1
    CliConnectionRefused = -2
    CliDatabaseNotFound = -3
    CliBadStatement = -4
    CliParameterNotFound = -5
    CliUnboundParameter = -6

    CliColumnNotFound = -7
    CliIncompatibleType = -8
    CliNetworkError = -9
    CliRuntimeError = -10
    CliClosedStatement = -11
    CliUnsupportedType = -12
    CliNotFound = -13
    CliNotUpdateMode = -14
    CliTableNotFound = -15
    CliNotAllColumnsSpecified = -16
    CliNotFetched = -17
    CliAlreadyUpdated = -18
    CliTableAlreadyExists = -19 
    CliNotImplemented = -20       
    CliLoginFailed = -21
    CliEmptyParameter = -22
    CliClosedConnection = -23

    ERROR_DESCRIPTIONS = {
      CliBadAddress => "Bad address",
      CliConnectionRefused => "Connection refused",
      CliDatabaseNotFound => "Database not found",
      CliBadStatement => "Bad statement",
      CliParameterNotFound => "Parameter not found",
      CliUnboundParameter => "Unbound parameter",
      CliColumnNotFound => "Column not found",
      CliIncompatibleType => "Incomptaible type",
      CliNetworkError => "Network error",
      CliRuntimeError => "Runtime error",
      CliClosedStatement => "Closed statement",
      CliUnsupportedType => "Unsupported type",
      CliNotFound => "Not found",
      CliNotUpdateMode => "Not update mode",
      CliTableNotFound => "Table not found",
      CliNotAllColumnsSpecified => "Not all columns specified",
      CliNotFetched => "Not fetched",
      CliAlreadyUpdated => "Already updated",
      CliTableAlreadyExists => "Table already exists",
      CliNotImplemented => "Not implemented",
      CliLoginFailed => "Login failed",
      CliEmptyParameter => "Empty parameter",
      CliClosedConnection => "Closed connection"}

=begin
== Command codes
=end
    CliCmdCloseSession = 0
    CliCmdPrepareAndExecute = 1
    CliCmdExecute = 2
    CliCmdGetFirst = 3
    CliCmdGetLast = 4
    CliCmdGetNext = 5
    CliCmdGetPrev = 6
    CliCmdFreeStatement = 7
    CliCmdAbort = 8
    CliCmdCommit = 9
    CliCmdUpdate = 10
    CliCmdRemove = 11
    CliCmdRemoveCurrent = 12
    CliCmdInsert = 13
    CliCmdPrepareAndInsert = 14
    CliCmdDescribeTable = 15
    CliCmdShowTables = 16
    CliCmdPrecommit = 17
    CliCmdSkip = 18
    CliCmdCreateTable = 19
    CliCmdDropTable = 20
    CliCmdAlterIndex = 21
    CliCmdFreeze = 22
    CliCmdUnfreeze = 23
    CliCmdSeek = 24
    CliCmdAlterTable = 25
    CliCmdLock = 26    

=begin
== Field type codes
=end
    CliOid =  0
    CliBool = 1
    CliInt1 = 2
    CliInt2 = 3
    CliInt4 = 4
    CliInt8 = 5
    CliReal4 = 6
    CliReal8 = 7
    CliDecimal = 8
    CliAsciiz = 9
    CliPasciiz = 10
    CliCstring = 11
    CliArrayOfOid =  12
    CliArrayOfBool = 13
    CliArrayOfInt1 = 14
    CliArrayOfInt2 = 15
    CliArrayOfInt4 = 16
    CliArrayOfInt8 = 17
    CliArrayOfReal4 = 18
    CliArrayOfReal8 = 19
    CliArrayOfDecimal = 20
    CliArrayOfString = 21
    CliAny =           22
    CliDatetime  =     23
    CliAutoincrement  = 24
    CliRectangle =     25
    CliUndefined =     26

    attr_reader :tables
end


=begin
== Statement class is used to prepare and execute select statement
=end
class Statement
  private
    PERCENT=37
    QUOTE=39
    LETTER_A=65
    LETTER_Z=90
    LETTER_a=97
    LETTER_z=122
    DIGIT_0=48
    DIGIT_9=57
    UNDERSCORE=95

  public
=begin
=== Statement constructor called by Connection class
=end                     
    def initialize(con, sql, stmt_id)
        @con = con
        @stmt_id = stmt_id
        r = sql.match(/\s+from\s+([^\s]+)/i)
        raise CliError, "Bad statement: table name is expected after FROM" unless r        
        table_name = r[1]
        @table = con.tables[table_name]
        in_quotes = false
        param_name = nil
        @param_hash = {}
        @param_list = []
        req_str=""
        sql.each_byte do |ch|
            if ch == QUOTE
                in_quotes = !in_quotes
                req_str << ch
            elsif ch == PERCENT and !in_quotes
                param_name=""
            elsif param_name != nil and ((ch >= LETTER_a and ch <= LETTER_z) or (ch >= LETTER_A and ch <= LETTER_Z) or (ch >= DIGIT_0 and ch <= DIGIT_9) or ch == UNDERSCORE)
                param_name << ch
            else
                if param_name != nil
                    p = Parameter.new(param_name)
                    @param_list << p
                    @param_hash[param_name] = p
                    param_name = nil
                    req_str << 0
                end
                req_str << ch
            end      
        end
        if param_name != nil
            p = Parameter.new(param_name)
            @param_list << p
            @param_hash[param_name] = p
            req_str << 0
        end
        if req_str.length == 0 or req_str[-1] != 0
            req_str << 0
        end         
        @stmt = req_str
        @prepared = false
    end

=begin
=== Get parameter value
=end
    def [](param_name)
        return @param_hash[param_name].value
    end


=begin
=== Assign value to the statement parameter
=end
    def []=(param_name, value)
        @param_hash[param_name].value = value
    end


=begin
=== Prepare (if needed) and execute select statement
    Only object set returned by the select for updated statement allows
    update and deletion of the objects.
==== for_update - if cursor is opened in for update mode
==== Rerturns object set with the selected objects
=end        
    def fetch(for_update = false) 
        cmd=Connection::CliCmdExecute
        req=""
        if !@prepared
            cmd=Connection::CliCmdPrepareAndExecute
            @prepared=true
            req << @param_list.length << @table.fields.length << [@stmt.length + @param_list.length].pack("n")
            param_no=0
            @stmt.each_byte do |ch|
                req << ch
                if ch == 0 and param_no < @param_list.length
                    param = @param_list[param_no]
                    if param.type == Connection::CliUndefined
                        raise CliError, "Unbound parameter #{param.name}"
                    end
                    param_no += 1
                    req << param.type
                end
            end
            for field in @table.fields
                 req << field.type << field.name << 0
            end
        end
        @for_update = for_update  
        req << for_update ? 1 : 0
        for param in @param_list
            case param.type
                when Connection::CliOid
                    req << [param.value.to_i].pack("N")
                when Connection::CliBool
                    req << param.value ? 1 : 0
                when Connection::CliInt4
                    req << [param.value].pack("N")
                when Connection::CliInt8
                    req << [param.value >> 32, param.value & 0xffffffff].pack("NN")
                when Connection::CliReal8
                    req << [param.value].pack("G")
                when Connection::CliAsciiz
                    req << param.value << 0
                when Connection::CliRectangle
                    req << [param.value.left, param.value.top, param.value.right, param.value.bottom].pack("NNNN")                     
                else
                    raise CliError, "Unsupported parameter type #{param.type}"
            end
        end 
        req = [req.length + Connection::CliRequestSize, cmd, @stmt_id].pack("NNN") + req
        return ResultSet.new(self, con.sendReceiveRequest(req))           
    end

=begin
=== Close statement
=end                     
    def close() 
	if con.nil?
	    raise CliError, "Statement already closed"
	end
        @con.sendCommand(Connection::CliCmdFreeStatement)
        @con = nil
    end

    attr_reader :for_update, :con, :stmt_id, :table
end

=begin
== Rectangle class for spatial coordinates
=end                     
class Rectangle    
=begin
=== Rectangle constructor
=end                     
    def initialize(left,top,right,bottom)
       @left = left
       @right = right
       @top = top
       @bottom = bottom
    end

    def to_s()
       return "<#{@left}, #{@top}, #{@right}, #{@bottom}>"
    end

    def inspect
       return "<Rectangle:0x#{"%x" % object_id} #{@left.inspect}, #{@top.inspect}, #{@right.inspect}, #{@bottom.inspect}>"
    end

    attr_reader :left, :right, :top, :bottom
end

=begin
== Descriptor of database table
=end                     
class TableDescriptor
=begin
=== Class descriptor constructor
==== cls - class
==== fields - array of FieldDescriptor 
=end                     
    def initialize(cls, fields)
        @cls=cls
        @fields=fields
        @fields_map={}
        for field in fields
            @fields_map[field.name]=field
        end
    end

    def [](field_name)
        return @fields_map[field_name]
    end

    attr_reader :cls, :fields
end

=begin
== Descriptor of database table field
=end                     
class FieldDescriptor
    def initialize(name, ref_table, inverse_field, type, flags)
       @name = name
       @type = type
       @flags = flags
       @ref_table = ref_table
       @inverse_field = inverse_field
    end

    attr_reader :name, :ref_table, :inverse_field, :type, :flags
end
       
=begin
== Reference to the persistent object
=end                     
class Reference
    def initialize(oid)
        @oid=oid
    end

    def to_i
        return @oid
    end

    def to_s()
        return "##{@oid}"
    end

    attr_reader :oid
end
    
=begin
== Statement parameter
=end                     
class Parameter
   def initialize(name)
       @name = name
       @type = Connection::CliUndefined
   end

   def value=(v)
       @value = v
       type = value.class
       if type == Fixnum
           @type = Connection::CliInt4
       elsif type == Bignum
           @type = Connection::CliInt8
       elsif type == Float
           @type = Connection::CliReal8
       elsif type == String
           @type = Connection::CliAsciiz
       elsif type == Reference
           @type = Connection::CliOid
       elsif type == TrueClass or type == FalseClass
           @type = Connection::CliBool
       elsif type == Rectangle
           @type = Connection::CliRectangle
       else
           raise CliError, "Unsupported parameter type #{value.class.name}"
       end
   end

   attr_reader :name, :type, :value
end

=begin
== CLI exception class
=end    
class CliError < RuntimeError
end

=begin
== Set of objects returned by select. This class allows navigation though the selected objects in orward or backward direction 
=end    
class ResultSet
    def initialize(stmt, n_objects)
        @stmt = stmt
        @n_objects = n_objects
        @updated = false
        @curr_oid = 0
        @curr_obj = nil        
    end

=begin
=== Get first selected object
==== Returns first object in the set or nil if no objects were selected
=end
    def first()  
        return getObject(Connection::CliCmdGetFirst)
    end

=begin
=== Get last selected object
==== Returns last object in the set or nil if no objects were selected
=end
    def last() 
        return getObject(Connection::CliCmdGetLast)
    end

=begin
=== Get next selected object
==== Returns next object in the set or nil if current object is the last one in the
    set or no objects were selected
=end
    def next()  
        return getObject(Connection::CliCmdGetNext)
    end

=begin
=== Get previous selected object
==== Returns previous object in the set or nil if the current object is the first
    one in the set or no objects were selected
=end
    def prev()
        return getObject(Connection::CliCmdGetPrev)
    end

=begin
=== Skip specified number of objects. 
=== if ((|n|)|)) is positive, then this method has the same effect as
    executing getNext() mehod ((|n|)) times.
=== if ((|n|)) is negative, then this method has the same effect of 
    executing getPrev() mehod ((|-n|)) times.     
=== if ((|n|)) is zero, this method has no effect     
==== n - number of objects to be skipped
==== Returns object ((|n|)) positions relative to the current position
=end
    def skip(n)  
        return getObject(Connection::CliCmdSkip, n)
    end

=begin
=== Get reference to the current object
==== Return return reference to the current object or nil if no objects were selected
=end
    def ref()  
        if @curr_oid != 0 
            return Reference.new(@curr_oid) 
        end       
        return nil
    end

=begin
=== Update the current object in the set. Changes made in the current object 
    are saved in the database
=end
    def update()  
        if @stmt.nil? 
            raise CliError, "ResultSet was aleady closed"
        end
        if @stmt.con.nil?
            raise CliError, "Statement was closed"
        end
        if @curr_oid == 0 
            raise CliError, "No object was selected"
        end
        if !@stmt.for_update 
            raise CliError, "Updates not allowed"
        end
        if @updated
            raise CliError, "Record was already updated"
        end
        @updated=true
        column_values=""
        obj=@curr_obj
        is_struct=obj.is_a?(Struct)

        for field in @stmt.table.fields
            if is_struct
                value = obj[field.name]
            else 
                value = obj.instance_variable_get("@#{field.name}")
            end
            case field.type
                when Connection::CliBool
                    column_values << value ? 1 : 0
                when Connection::CliInt1
                    column_values << value.to_i
                when Connection::CliInt2
                    column_values << [value.to_i].pack("n")
                when Connection::CliInt4
                    column_values << [value.to_i].pack("N")
                when Connection::CliInt8
                    column_values << [value.to_i >> 32, value.to_i & 0xffffffff].pack("NN")
                when Connection::CliReal4
                    column_values << [value.to_f].pack("g")
                when Connection::CliReal8
                    column_values << [value.to_f].pack("G")
                when Connection::CliAsciiz
                    if value.nil?
                        column_values << [1].pack("N") << 0
                    else 
                        column_values << [value.length+1].pack("N") << value << 0
                    end
                when Connection::CliOid
                    if value.nil?
                        column_values << [0].pack("N")
                    else
                        column_values << [value.to_i].pack("N")
                    end
                when Connection::CliRectangle
                    column_values << [value.left,value.top,value.right,value.bottom].pack("NNNN")                     
                when Connection::CliArrayOfInt1
                    column_values << [value.length].pack("N") << value.pack("c*")
                when Connection::CliArrayOfBool
                    column_values << [value.length].pack("N")
                    for elem in value
                        column_values << elem ? 1 : 0
                    end
                when Connection::CliArrayOfInt2
                    column_values << [value.length].pack("N") << value.pack("n*")
                when Connection::CliArrayOfInt4
                    column_values << [value.length].pack("N") << value.pack("N*")
                when Connection::CliArrayOfInt8
                    column_values << [value.length].pack("N")
                    for elem in value
                        column_values << [elem >> 32, elem & 0xffffffff].pack("NN")
                    end
                when Connection::CliArrayOfReal4
                    column_values << [value.length].pack("N") << value.pack("g*")
                when Connection::CliArrayOfReal8
                    column_values << [value.length].pack("N") << value.pack("G*")
                when Connection::CliArrayOfOid
                    column_values << [value.length].pack("N")
                    for elem in value
                        column_values << [elem.to_i].pack("N")
                    end
                when Connection::CliArrayOfString
                    column_values << [value.length].pack("N")
                    for elem in value
                        column_values << elem << 0
                    end                   
                else
                    raise CliError, "Unsupported type #{field.type}"
             end     
        end       
        req = [Connection::CliRequestSize + column_values.length, Connection::CliCmdUpdate, @stmt.stmt_id].pack("NNN") + column_values
        @stmt.con.sendReceiveRequest(req)
   end

=begin
=== Remove all selected objects. 
    All objects in the object set are removed from the database.
=end
    def removeAll()
        if @stmt.nil?
            raise CliError, "ResultSet was aleady closed"
        end
        if @stmt.con.nil? 
            raise CliError, "Statement was closed"
        end
        if !@stmt.for_update
            raise CliError, "Updates not allowed"
        end
        @stmt.con.sendReceiveCommand(Connection::CliCmdRemove, @stmt.stmt_id)
    end
   
=begin
=== Remove current object. 
    All objects in the object set are removed from the database.
=end
    def removeCurrent()
        if @stmt.nil?
            raise CliError, "ResultSet was aleady closed"
        end
        if @stmt.con.nil? 
            raise CliError, "Statement was closed"
        end
        if !@stmt.for_update
            raise CliError, "Updates not allowed"
        end
        if @curr_oid == 0 
            raise CliError, "No object was selected"
        end
        @stmt.con.sendReceiveCommand(Connection::CliCmdRemoveCurrent, @stmt.stmt_id)
    end
   
=begin
=== Get the number of objects in the object set.
==== Returna number of the selected objects
=end
    def size()  
        return @n_objects
    end

=begin
=== Close result set and statement. Any followin operation with this result set or statement will raise an exception.
=end
    def close()  
        @stmt.close()
        @stmt = nil
    end

=begin
=== Iterator through result set
=end
    def each() 
       if first != nil
           yield @curr_obj 
           while self.next != nil
               yield @curr_obj 
           end
       end
    end


    def getObject(cmd, n=0)
        if @stmt.nil?
            raise CliError, "ResultSet was aleady closed"
        end
        if @stmt.con.nil?
            raise CliError, "Statement was closed"
        end
        if cmd == Connection::CliCmdSkip
            @socket.send([16, cmd, @stmt.stmt_id, n].pack("NNNN"), 0)
        else
            @stmt.con.sendCommand(cmd, @stmt.stmt_id)
        end
        rc = @stmt.con.receive(4).unpack("N")[0].hash
        if rc == Connection::CliNotFound 
            return nil
        elsif rc <= 0
            @stmt.con.raiseError(rc, "Failed to get object: ")
        end
        resp = @stmt.con.receive(rc-4)
        @curr_oid = resp.unpack("N")[0]
        @updated = false
        @curr_obj = nil
        if @curr_oid == 0 
            return nil
        end
        obj = @stmt.table.cls.allocate
        @curr_obj = obj
        is_struct = obj.is_a?(Struct)
        i = 4
        for field in @stmt.table.fields
            type = resp[i]
            if field.type != type
                raise CliError, "Unexpected type of column: #{type} instead of #{field.type}"
            end
            i += 1
            case type
                when Connection::CliBool
                    value = (resp[i] != 0)
                    i += 1
                when Connection::CliInt1
                    value = resp[i]
                    i += 1
                when Connection::CliInt2
                    value = resp[i,2].unpack("n")[0]
                    i += 2
                when Connection::CliInt4
                    value = resp[i,4].unpack("N")[0]
                    i += 4
                when Connection::CliInt8
                    word = resp[i,8].unpack("NN")
                    value = (word[0] << 32) | (word[1] & 0xffffffff)
                    i += 8
                when Connection::CliReal4
                    value = resp[i,4].unpack("g")[0]
                    i += 4
                when Connection::CliReal8
                    value = resp[i,8].unpack("G")[0]
                    i += 8
                when Connection::CliAsciiz
                    len = resp[i,4].unpack("N")[0]
                    value = resp[i+4, len-1]
                    i += len + 4
                when Connection::CliOid
                    value = Reference.new(resp[i,4].unpack("N")[0])
                    i += 4
                when Connection::CliRectangle
                    coord = resp[i, 16].unpack("NNNN")
                    value = Rectangle.new(coord[0], coord[1], coord[2], coord[3])
                when Connection::CliArrayOfInt1
                    len = resp[i,4].unpack("N")[0]
                    i += 4
                    value = resp[i, len]
                    i += len
                when Connection::CliArrayOfBool
                    value = Array.new(resp[i,4].unpack("N")[0])
                    i += 4
                    for j in 0...value.length
                        value[j] = resp[i] != 0
                        i += 1
                    end 
                when Connection::CliArrayOfInt2
                    len = resp[i,4].unpack("N")[0]
                    i += 4
                    value = resp[i, len*2].unpack("n*")
                    i += len*2
                when Connection::CliArrayOfInt4
                    len = resp[i,4].unpack("N")[0]
                    i += 4
                    value = resp[i, len*4].unpack("N*")
                    i += len*4
                when Connection::CliArrayOfInt8
                    len = resp[i,4].unpack("N")[0]
                    i += 4
                    word = resp[i, len*8].unpack("N*")
                    value = Array.new(len)                    
                    for j in 0...value.length
                        value[j] = (word[j*2] << 32) | (word[j*2+1] & 0xffffffff)
                    end 
                    i += len*8
                when Connection::CliArrayOfReal4
                    len = resp[i,4].unpack("N")[0]
                    i += 4
                    value = resp[i, len*4].unpack("g*")
                    i += len*4
                when Connection::CliArrayOfReal8
                    len = resp[i,4].unpack("N")[0]
                    i += 4
                    value = resp[i, len*8].unpack("G*")
                    i += len*8
                when Connection::CliArrayOfOid
                    len = resp[i,4].unpack("N")[0]
                    value = Array.new(len)
                    i += 4
                    oid = resp[i, len*4].unpack("N*")
                    for j in 0...len
                        value[j] = Reference.new(oid[j])
                    end 
                    i += len*4
                when Connection::CliArrayOfString
                    value = Array.new(resp[i,4].unpack("N")[0])
                    i += 4
                    for j in 0...value.length
                        k = resp.index(0, i)
                        value[j] = resp[i...k]
                        i = k + 1
                    end 
                else
                    raise CliError, "Unsupported type #{type}"
            end  
            if is_struct
                obj[field.name] = value
            else
                obj.instance_variable_set("@#{field.name}", value)                
            end
        end               
        return obj
    end
end

end
