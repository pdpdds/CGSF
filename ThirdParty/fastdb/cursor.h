//-< CURSOR.H >------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Table cursor
//-------------------------------------------------------------------*--------*

#ifndef __CURSOR_H__
#define __CURSOR_H__

BEGIN_FASTDB_NAMESPACE

class dbOrderByNode;

class FASTDB_DLL_ENTRY dbSelection { 
  public:
    enum { quantum = 1024 };
    class segment { 
      public:
        segment* prev;
        segment* next;
        int      nRows;
        oid_t    rows[quantum];

        segment(segment* after) { 
            prev = after;
            next = NULL;
            nRows = 0;
        }       
    };
    segment*  first;
    segment*  last;
    segment*  curr;
    cardinality_t nRows;
    int pos;

    segment*  createNewSegment(segment* after);

    void add(oid_t oid) {
        if (last == NULL) { 
            first = last = createNewSegment(NULL);
        } else if (last->nRows == quantum) { 
            last = last->next = createNewSegment(last);
        }
        last->rows[last->nRows++] = oid;
        nRows += 1;
    }
   
    void sort(dbDatabase* db, dbOrderByNode* order);
    static int compare(oid_t a, oid_t b, dbOrderByNode* order);

    void toArray(oid_t* oids) const;
    void truncate(cardinality_t from, cardinality_t length);

    dbSelection() { 
        nRows = 0;
        pos = 0;
        first = curr = last = NULL;
    }
    void reverse();
    void reset();
};

enum dbCursorType { 
    dbCursorViewOnly,
    dbCursorForUpdate
};

/**
 * Base class for all cursors
 */
class FASTDB_DLL_ENTRY dbAnyCursor : public dbL2List { 
    friend class dbAnyContainer;
    friend class dbDatabase;
    friend class dbHashTable;
    friend class dbTtreeNode;
    friend class dbRtreePage;
    friend class dbSubSql;
    friend class dbStatement;
    friend class dbServer;
    friend class dbCLI;
    friend class JniResultSet;
  public:
    /**
     * Get number of selected records
     * @return number of selected records
     */
    cardinality_t getNumberOfRecords() const { return selection.nRows; }

    /**
     * Remove current record
     */
    void remove();
    
    /**
     * Checks whether selection is empty
     * @return true if there is no current record
     */
    bool isEmpty() const { return currId == 0; }

    /**
     * Check whether this cursor can be used for update
     * @return true if it is update cursor
     */
    bool isUpdateCursor() const { 
        return type == dbCursorForUpdate;
    }

    /**
     * Checks whether limit for number of selected reacord is reached
     * @return true if limit is reached
     */
    bool isLimitReached() const { return selection.nRows >= limit || selection.nRows >= stmtLimitLen; }

    /**
     * Extract OIDs of selected recrods in array
     * @param arr if <code>arr</code> is not null, then this array is used as destination (it should
     *   be at least selection.nRows long)<BR>
     *  If <code>arr</code> is null, then new array is created by  new oid_t[] and returned by this method
     * @return if <code>arr</code> is not null, then <code>arr</code>, otherwise array created by this method
     */
    oid_t* toArrayOfOid(oid_t* arr) const; 

    /**
     * Execute query.
     * @param query selection criteria
     * @param aType cursor type: <code>dbCursorForUpdate, dbCursorViewOnly</code>
     * @param paramStruct pointer to structure with parameters. If you want to create reentrant precompiled query, i.e.
     * query which can be used concurrently by different threadsm you should avoid to use static variables in 
     * such query, and instead of it place paramters into some structure, specify in query relative offsets to the parameters,
     * fill local structure and pass pointer to it to select method.
     * @return number of selected records
     */
    cardinality_t select(dbQuery& query, dbCursorType aType, void* paramStruct = NULL) {
        type = aType;
        reset();
        paramBase = paramStruct;
        db->select(this, query);
        paramBase = NULL;
        if (gotoFirst() && prefetch) { 
            fetch();
        }
        return selection.nRows;
    } 
    
    /**
     * Execute query with default cursor type.
     * @param query selection criteria
     * @param paramStruct pointer to structure with parameters.
     * @return number of selected records
     */    
    cardinality_t select(dbQuery& query, void* paramStruct = NULL) { 
        return select(query, defaultType, paramStruct);
    }
     
    /**
     * Execute query.
     * @param condition selection criteria
     * @param aType cursor type: <code>dbCursorForUpdate, dbCursorViewOnly</code>
     * @param paramStruct pointer to structure with parameters.
     * @return number of selected records
     */
    cardinality_t select(char const* condition, dbCursorType aType, void* paramStruct = NULL) { 
        dbQuery query(condition);
        return select(query, aType, paramStruct);
    } 

    /**
     * Execute query with default cursor type.
     * @param condition selection criteria
     * @param paramStruct pointer to structure with parameters.
     * @return number of selected records
     */    
    cardinality_t select(char const* condition, void* paramStruct = NULL) { 
        return select(condition, defaultType, paramStruct);
    }

    /**
     * Select all records from the table
     * @param aType cursor type: <code>dbCursorForUpdate, dbCursorViewOnly</code>
     * @return number of selected records
     */    
    cardinality_t select(dbCursorType aType) { 
        type = aType;
        reset();
        db->select(this); 
        if (gotoFirst() && prefetch) { 
            fetch();
        }
        return selection.nRows;
    } 

    /**
     * Select all records from the table with default cursor type
     * @return number of selected records
     */    
    cardinality_t select() {
        return select(defaultType);
    }

    /**
     * Select all records from the table with specfied value of the key
     * @param key name of the key field
     * @param value searched value of the key
     * @return number of selected records
     */    
    cardinality_t selectByKey(char const* key, void const* value);

    /**
     * Select all records from the table with specfied value of the key
     * @param field key field
     * @param value searched value of the key
     * @return number of selected records
     */    
    cardinality_t selectByKey(dbFieldDescriptor* field, void const* value);

    /**
     * Select all records from the table with specfied range of the key values
     * @param key name of the key field
     * @param minValue inclusive low bound for key values, if <code>NULL</code> then there is no low bound
     * @param maxValue inclusive high bound for key values, if <code>NULL</code> then there is no high bound
     * @return number of selected records
     */    
    cardinality_t selectByKeyRange(char const* key, void const* minValue, void const* maxValue);

    /**
     * Select all records from the table with specfied range of the key values
     * @param field key field
     * @param minValue inclusive low bound for key values, if <code>NULL</code> then there is no low bound
     * @param maxValue inclusive high bound for key values, if <code>NULL</code> then there is no high bound
     * @return number of selected records
     */    
    cardinality_t selectByKeyRange(dbFieldDescriptor* field, void const* minValue, void const* maxValue);

    /**
     * Update current record. You should changed value of current record before and then call
     * update method to save changes to the database
     */
    void update() { 
        if (type != dbCursorForUpdate) {
            db->handleError(dbDatabase::ReadonlyCursor);
        }
        if (currId == 0) {
            db->handleError(dbDatabase::NoCurrentRecord);
        }
        db->update(currId, table, record);
    }

    /**
     * Remove all records in the table
     */
    void removeAll() {
        assert(db != NULL);
        if (type != dbCursorForUpdate) {
            db->handleError(dbDatabase::ReadonlyCursor);
        }
        db->deleteTable(table);
        reset();
    }

    /**
     * Remove all selected records
     */
    void removeAllSelected();

    /**
     * Specify maximal number of records to be selected
     */
    void setSelectionLimit(cardinality_t lim) { limit = lim; }
    
    /**
     * Remove selection limit
     */
    void unsetSelectionLimit() { limit = dbDefaultSelectionLimit; }

    /**
     * Set prefetch mode. By default, current record is fetch as soon as it is becomes current.
     * But sometimesyou need only OIDs of selected records. In this case setting prefetchMode to false can help.
     * @param mode if <code>false</code> then current record is not fetched. You should explicitly call <code>fetch</code>
     * method if you want to fetch it.
     */
    void setPrefetchMode(bool mode) { prefetch = mode; }


    /**
     * Enable or disable duplicates checking (if programmer knows that disjuncts in query do not intersect, then
     * he can disable duplicate checking and avoid bitmap allocation
     */
    void enableCheckForDuplicates(bool enabled) {
        checkForDuplicatedIsEnabled = enabled;
    }
        

    /**
     * Reset cursor
     */
    void reset();

    /**
     * Check whether current record is the last one in the selection
     * @return true if next() method will return <code>NULL</code>
     */
    bool isLast() const; 

    /**
     * Check whether current record is the first one in the selection
     * @return true if prev() method will return <code>NULL</code>
     */
    bool isFirst() const; 

    /**
     * Freeze cursor. This method makes it possible to save current state of cursor, close transaction to allow
     * other threads to proceed, and then later restore state of the cursor using unfreeze method and continue 
     * traversal through selected records.
     */     
    void freeze();

    /**
     * Unfreeze cursor. This method starts new transaction and restore state of the cursor
     */
    void unfreeze();

    /**
     * Skip specified number of records
     * @param n if positive then skip <code>n</code> records forward, if negative then skip <code>-n</code> 
     * records backward
     * @return <code>true</code> if specified number of records was successfully skipped, <code>false</code> if
     * there is no next (<code>n &gt; 0</code>) or previous (<code>n &lt; 0</code>) record in the selction.
     */
    bool skip(int n);


    /**
     * Position cursor on the record with the specified OID
     * @param oid object identifier of record
     * @return poistion of the record in the selection or -1 if record with such OID is not in selection
     */
    int seek(oid_t oid);

    /**
     * Get table for which cursor is opened
     */
    dbTableDescriptor* getTable() { return table; }


    /**
     * Set table for the cursor
     * @param aTable table which records will be iterated
     */
    void setTable(dbTableDescriptor* aTable) { 
        table = aTable;
        db = aTable->db;
    }

    /**
     * Set destination for selected record
     * rec - buffer to which fields of current record will be fetched
     */
    void setRecord(void* rec) { 
        record = (byte*)rec;
    }

    /**
     * Get pointer to the location where fields of the current record are fetched
     * @return pointer to the memory location set by cursor constructor or setRecord method
     */
    void* getRecord() { 
        return record;
    }

    /**
     * Check if record with specified OID is in selection
     * @return <code>true</code> if record with such OID was selected
     */
    bool isInSelection(oid_t oid);


    /**
     * Fetch current record.
     * You should use this method only if prefetch mode is disabled 
     */
    void fetch() { 
        if (db->currIndex[currId] & (dbInternalObjectMarker|dbFreeHandleMarker)) {
            db->handleError(dbDatabase::AccessToDeletedObject);
        }
        table->columns->fetchRecordFields(record, 
                                          (byte*)db->getRow(currId));
    }

    /**
     * Check if there is more records in the selection
     */
    bool hasNext() const;
    
  protected: 
    dbDatabase*        db;
    dbTableDescriptor* table;
    dbCursorType       type;
    dbCursorType       defaultType;
    dbSelection        selection;
    bool               allRecords;
    oid_t              firstId;
    oid_t              lastId;
    oid_t              currId;
    byte*              record;
    cardinality_t      limit;

    int4*              bitmap; // bitmap to avoid duplicates
    size_t             bitmapSize;
    bool               eliminateDuplicates;
    bool               checkForDuplicatedIsEnabled;
    bool               prefetch;
    bool               removed; // current record was removed
    bool               lastRecordWasDeleted; //last record was deleted

    cardinality_t      stmtLimitStart;
    cardinality_t      stmtLimitLen;
    cardinality_t      nSkipped;

    void*              paramBase;
    
    void checkForDuplicates();
    void deallocateBitmap();

    bool isMarked(oid_t oid) { 
        return bitmap != NULL && (bitmap[oid >> 5] & (1 << (oid & 31))) != 0;
    }

    void setStatementLimit(dbQuery const& q) { 
        stmtLimitStart = q.stmtLimitStartPtr != NULL ? (nat4)*q.stmtLimitStartPtr : q.stmtLimitStart;
        stmtLimitLen = q.stmtLimitLenPtr != NULL ? (nat4)*q.stmtLimitLenPtr : q.stmtLimitLen;
    }

    void truncateSelection() { 
        selection.truncate(stmtLimitStart, stmtLimitLen);
    }

    void mark(oid_t oid) { 
        if (bitmap != NULL) { 
            bitmap[oid >> 5] |= 1 << (oid & 31);
        }
    }    

    bool add(oid_t oid) { 
        if (selection.nRows < limit && selection.nRows < stmtLimitLen) { 
            if (nSkipped < stmtLimitStart) { 
                nSkipped += 1;
                return true;
            }
            if (eliminateDuplicates) { 
                if (bitmap[oid >> 5] & (1 << (oid & 31))) { 
                    return true;
                }
                bitmap[oid >> 5] |= 1 << (oid & 31);
            } 
            selection.add(oid);
            return selection.nRows < limit;
        } 
        return false;
    }

    byte* fetchNext();
    byte* fetchPrev();

    bool gotoNext();
    bool gotoPrev(); 
    bool gotoFirst();
    bool gotoLast();

    
    bool moveNext();
    bool movePrev();

    void setCurrent(dbAnyReference const& ref);

    void adjustReferences(size_t base, size_t size, size_t shift) { 
        if (currId != 0 && record != NULL) { 
            table->columns->adjustReferences(record, base, size, shift);
        }
    }

    dbAnyCursor(dbTableDescriptor& aTable, dbCursorType aType, byte* rec)
    : table(&aTable),type(aType),defaultType(aType),
      allRecords(false),currId(0),record(rec)
    {
        db = aTable.db;
        limit = dbDefaultSelectionLimit;
        prefetch = rec != NULL;
        removed = false;
        bitmap = NULL; 
        bitmapSize = 0;
        eliminateDuplicates = false;
        checkForDuplicatedIsEnabled = true;
        paramBase = NULL;
        stmtLimitLen = dbDefaultSelectionLimit;
        stmtLimitStart = 0;
        nSkipped = 0;
    }
  public:
    dbAnyCursor() 
    : table(NULL),type(dbCursorViewOnly),defaultType(dbCursorViewOnly),
          allRecords(false),currId(0),record(NULL)
    {
        limit = dbDefaultSelectionLimit;
        prefetch = false;
        removed = false;
        bitmap = NULL;
        bitmapSize = 0;
        eliminateDuplicates = false;
        checkForDuplicatedIsEnabled = true;
        db = NULL;
        paramBase = NULL;
        stmtLimitLen = dbDefaultSelectionLimit;
        stmtLimitStart = 0;
        nSkipped = 0;
    }
    ~dbAnyCursor();
};


/**
 * Cursor template parameterized by table class
 */
template<class T>
class dbCursor : public dbAnyCursor { 
  private:
    // It is not possible to copy cursors
    dbCursor<T> operator = (dbCursor<T> const& src) { 
        return *this;
    } 
    dbCursor(dbCursor<T> const& src) {assert(0);}

  protected:
    T record;
    
  public:
    /**
     * Cursor constructor
     * @param type cursor type (dbCursorViewOnly by default)
     */
    dbCursor(dbCursorType type = dbCursorViewOnly) 
        : dbAnyCursor(T::dbDescriptor, type, (byte*)&record) {}

    /**
     * Cursor constructor with explicit specification of database.
     * This cursor should be used for unassigned tables. 
     * @param aDb database in which table lookup is performed
     * @param type cursor type (dbCursorViewOnly by default)
     */
    dbCursor(dbDatabase* aDb, dbCursorType type = dbCursorViewOnly)
        : dbAnyCursor(T::dbDescriptor, type, (byte*)&record)
    {
        db = aDb;
        dbTableDescriptor* theTable = db->lookupTable(table);
        if (theTable != NULL) { 
            table = theTable;
        }
    }

    /**
     * Get pointer to the current record
     * @return pointer to the current record or <code>NULL</code> if there is no current record
     */
    T* get() { 
        return currId == 0 ? (T*)NULL : &record; 
    }
    
    /**
     * Get next record
     * @return pointer to the next record or <code>NULL</code> if there is no next record
     */     
    T* next() {
        return (T*)fetchNext();
    }

    /**
     * Get previous record
     * @return pointer to the previous record or <code>NULL</code> if there is no previous record
     */     
    T* prev() { 
        return (T*)fetchPrev();
    }

    /**
     * Get pointer to the first record
     * @return pointer to the first record or <code>NULL</code> if no records were selected
     */
    T* first() { 
        if (gotoFirst()) {
            fetch();
            return &record;
        }
        return NULL;
    }

    /**
     * Get pointer to the last record
     * @return pointer to the last record or <code>NULL</code> if no records were selected
     */
    T* last() { 
        if (gotoLast()) {
            fetch();
            return &record;
        }
        return NULL;
    }    
    
    /**
     * Position cursor on the record with the specified OID
     * @param ref reference to the object
     * @return poistion of the record in the selection or -1 if record with such OID is not in selection
     */
    int seek(dbReference<T> const& ref) { 
        return dbAnyCursor::seek(ref.getOid());
    }

    /**
     * Overloaded operator for accessing components of the current record
     * @return pointer to the current record
     */
    T* operator ->() { 
        if (currId == 0) {
            db->handleError(dbDatabase::NoCurrentRecord);
        }
        return &record;
    }

    /**
     * Select record by reference
     * @param ref reference to the record
     * @return pointer to the referenced record
     */
    T* at(dbReference<T> const& ref) { 
        setCurrent(ref);
        return &record;
    }
    
    /**
     * Convert selection to array of reference
     * @param arr [OUT] array of refeences in which references to selected recrods will be placed
     */
    void toArray(dbArray< dbReference<T> >& arr) const { 
        arr.resize(selection.nRows);
        toArrayOfOid((oid_t*)arr.base());
    }

    /**
     * Get current object idenitifer
     * @return reference to the current record
     */
    dbReference<T> currentId() const { 
        return dbReference<T>(currId);
    }

    /**
     * Check if record with specified OID is in selection
     * @return <code>true</code> if record with such OID was selected
     */
    bool isInSelection(dbReference<T>& ref) {
        return dbAnyCursor::isInSelection(ref.getOid());
    }
};

class dbParallelQueryContext { 
  public:
    dbDatabase* const      db;
    dbCompiledQuery* const query;
    oid_t                  firstRow;
    dbTable*               table;
    dbAnyCursor*           cursor;
    dbSelection            selection[dbMaxParallelSearchThreads];

    void search(int i); 

    dbParallelQueryContext(dbDatabase* aDb, dbTable* aTable, 
                           dbCompiledQuery* aQuery, dbAnyCursor* aCursor)
      : db(aDb), query(aQuery), firstRow(aTable->firstRow), table(aTable), cursor(aCursor) {}
};


extern char* strupper(char* s);

extern char* strlower(char* s);

END_FASTDB_NAMESPACE

#endif
