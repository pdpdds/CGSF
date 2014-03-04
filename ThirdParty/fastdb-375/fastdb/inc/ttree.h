//-< TTREE.CPP >-----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// T-Tree interface
//-------------------------------------------------------------------*--------*

#ifndef __TTREE_H__
#define __TTREE_H__

BEGIN_FASTDB_NAMESPACE

class FASTDB_DLL_ENTRY dbTtreeNode { 
    enum { 
        pageSize = 125,
        minItems = pageSize - 2 // minimal number of items in internal node
    };

  public:
    oid_t left;
    oid_t right;
    int1  balance;
    nat2  nItems;
    oid_t item[pageSize];

    static oid_t allocate(dbDatabase* db, oid_t recordId);
    
    static bool  insert(dbDatabase* db, oid_t& nodeId, oid_t recordId, 
                        void* key, int type, int sizeofType, dbUDTComparator comparator, int offs);
    static int   remove(dbDatabase* db, oid_t& nodeId, oid_t recordId,  
                        void* key, int type, int sizeofType, dbUDTComparator comparator, int offs);
    static int   balanceRightBranch(dbDatabase* db, oid_t& nodeId);
    static int   balanceLeftBranch(dbDatabase* db, oid_t& nodeId);
    
    static void  purge(dbDatabase* db, oid_t nodeId);

    bool find(dbDatabase* db, dbSearchContext& sc);
    bool prefixSearch(dbDatabase* db, dbSearchContext& sc);

    bool traverseForward(dbDatabase* db,dbAnyCursor* cursor);
    bool traverseBackward(dbDatabase* db, dbAnyCursor* cursor);
    bool traverseForward(dbDatabase* db,dbAnyCursor* cursor,dbExprNode* cond);
    bool traverseBackward(dbDatabase* db,dbAnyCursor* cursor,dbExprNode* cond);
};

class FASTDB_DLL_ENTRY dbTtree { 
  protected:
    oid_t root;

  public:
    static oid_t allocate(dbDatabase* db);
    static void  find(dbDatabase* db, oid_t treeId, dbSearchContext& sc);
    static void  prefixSearch(dbDatabase* db, oid_t treeId, dbSearchContext& sc);
    static void  insert(dbDatabase* db, oid_t treeId, oid_t recordId, 
                        int type, int sizeofType, dbUDTComparator comparator, int offs);
    static void  remove(dbDatabase* db, oid_t treeId, oid_t recordId,
                        int type, int sizeofType, dbUDTComparator comparator, int offs);
    static void  drop(dbDatabase* db, oid_t treeId);
    static void  purge(dbDatabase* db, oid_t treeId);
    
    static void  traverseForward(dbDatabase* db, oid_t treeId,
                                 dbAnyCursor* cursor);
    static void  traverseBackward(dbDatabase* db, oid_t treeId,
                                  dbAnyCursor* cursor);
    static void  traverseForward(dbDatabase* db, oid_t treeId,
                                 dbAnyCursor* cursor, dbExprNode* condition);
    static void  traverseBackward(dbDatabase* db, oid_t treeId,
                                  dbAnyCursor* cursor, dbExprNode* condition);
};

END_FASTDB_NAMESPACE

#endif
