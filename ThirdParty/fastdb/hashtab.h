//-< HASHTAB.CPP >---------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Extensible hash table interface
//-------------------------------------------------------------------*--------*

#ifndef __HASHTAB_H__
#define __HASHTAB_H__

BEGIN_FASTDB_NAMESPACE

class FASTDB_DLL_ENTRY dbHashTableItem { 
  public:
    oid_t next;
    oid_t record;
    nat4  hash;
};

const size_t dbInitHashTableSize = 16*1024-1;

class dbFieldDescriptor;

class FASTDB_DLL_ENTRY dbHashTable { 
    nat4  size;
    nat4  used;
    oid_t page;

  public:
    static dbUDTHashFunction getHashFunction(int version, dbFieldDescriptor* fd);

    static oid_t allocate(dbDatabase* db, size_t nRows = 0);
    
    static void  insert(dbDatabase* db, dbFieldDescriptor* fd, oid_t rowId, size_t nRows);
    
    static void  remove(dbDatabase* db, dbFieldDescriptor* fd, oid_t rowId);
    
    static void  find(dbDatabase* db, oid_t hashId, dbSearchContext& sc);
    
    static void  drop(dbDatabase* db, oid_t hashId);

    static void  purge(dbDatabase* db, oid_t hashId);
};

END_FASTDB_NAMESPACE

#endif
