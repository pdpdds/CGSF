//-< RTREE.H >-------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Post Relational Database Management System)                      *   /\|  *
//                                                                   *  /  \  *
//                          Created:     22-Nov-2001  K.A. Knizhnik  * / [] \ *
//                          Last update: 22-Nov-2001  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// R-tree: spatial index
//-------------------------------------------------------------------*--------*

#ifndef __RTREE_H__
#define __RTREE_H__

#include "rectangle.h"

BEGIN_FASTDB_NAMESPACE

#define MAX_RTREE_HEIGHT 8

class dbAnyCursor;

class dbRtreePage { 
  public:
    struct branch { 
        rectangle rect;
        oid_t     p;
    };
    
    enum { 
        card = (dbPageSize - 4) / sizeof(branch), // maximal number of branches at page
        min_fill = card/2        // minimal number of branches at non-root page
    };

    struct reinsert_list { 
        oid_t     chain;
        int       level;
        reinsert_list() { chain = 0; }
    };

    bool find(dbDatabase* db, dbSearchContext& sc, int level) const;
    static bool find(dbDatabase* db, oid_t rootId, dbSearchContext& sc, int level);

    static oid_t insert(dbDatabase* db, rectangle const& r, oid_t pageId, oid_t recordId, int level);

    static bool remove(dbDatabase* db, rectangle const& r, oid_t pageId, oid_t recordId, 
                       int level, reinsert_list& rlist);

    void cover(rectangle& r) const;
    static void cover(dbDatabase* db, oid_t pageId, rectangle& r);

    static oid_t add_branch(dbDatabase* db, oid_t pageId, branch const& br);

    void remove_branch(int i);

    static void purge(dbDatabase* db, oid_t pageId, int level);

    oid_t next_reinsert_page() const { 
        return b[card-1].p; 
    }

    static oid_t allocate(dbDatabase* db, oid_t recordId, rectangle const& r);
    static oid_t allocate(dbDatabase* db, oid_t rootId, oid_t p);

    int4   n; // number of branches at page
    branch b[card];
    };

class FASTDB_DLL_ENTRY dbRtree {
  public: 
    enum searchOp { 
        EQUAL, 
        OVERLAPS, 
        SUPERSET,
        PROPER_SUPERSET, 
        SUBSET, 
        PROPER_SUBSET
    };

    static oid_t allocate(dbDatabase* db);
    static bool  find(dbDatabase* db, oid_t treeId, dbSearchContext& sc);
    static void  insert(dbDatabase* db, oid_t treeId, oid_t recordId, int offs);
    static void  insert(dbDatabase* db, oid_t treeId, oid_t recordId, rectangle const& r);
    static void  remove(dbDatabase* db, oid_t treeId, oid_t recordId, int offs);
    static void  purge(dbDatabase* db, oid_t treeId);
    static void  drop(dbDatabase* db, oid_t treeId);

  protected:
    int4   height;
    oid_t  root;
};

END_FASTDB_NAMESPACE

#endif
