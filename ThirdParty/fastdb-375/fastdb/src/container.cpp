//-< CONTAINER.CPP >---------------------------------------------------*--------*
// GigaBASE                  Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     05-Nov-2002  K.A. Knizhnik  * / [] \ *
//                          Last update: 05-Nov-2002  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// T-Tree object container
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "fastdb.h"
#include "compiler.h"
#include "ttree.h"
#include "rtree.h"
#include "symtab.h"

BEGIN_FASTDB_NAMESPACE

void dbAnyContainer::create(dbDatabase* db)
{
    db->beginTransaction(dbDatabase::dbExclusiveLock);
    oid = (fd->type == dbField::tpRectangle) ? dbRtree::allocate(db) : dbTtree::allocate(db);
}

void dbAnyContainer::add(dbDatabase* db, dbAnyReference const& ref)
{
    db->beginTransaction(dbDatabase::dbExclusiveLock);
    if (fd->type == dbField::tpRectangle) { 
        dbRtree::insert(db, oid, ref.getOid(), fd->dbsOffs);
    } else { 
        dbTtree::insert(db, oid, ref.getOid(), fd->type, (int)fd->dbsSize, fd->_comparator, fd->dbsOffs);
    }
}

void dbAnyContainer::remove(dbDatabase* db, dbAnyReference const& ref)
{
    db->beginTransaction(dbDatabase::dbExclusiveLock);
    if (fd->type == dbField::tpRectangle) { 
        dbRtree::remove(db, oid, ref.getOid(), fd->dbsOffs);
    } else { 
        dbTtree::insert(db, oid, ref.getOid(), fd->type, (int)fd->dbsSize, fd->_comparator, fd->dbsOffs);
    }
}

void dbAnyContainer::purge(dbDatabase* db)
{
    db->beginTransaction(dbDatabase::dbExclusiveLock);
    if (fd->type == dbField::tpRectangle) { 
        dbRtree::purge(db, oid);
    } else { 
        dbTtree::purge(db, oid);
    }
}

void dbAnyContainer::deallocate(dbDatabase* db)
{
    db->beginTransaction(dbDatabase::dbExclusiveLock);
    if (fd->type == dbField::tpRectangle) { 
        dbRtree::drop(db, oid);
    } else { 
        dbTtree::drop(db, oid);
    }
}

int dbAnyContainer::prefixSearch(dbAnyCursor& cursor, char const* key)
{
    dbDatabase* db = cursor.table->db;
    db->beginTransaction(cursor.type == dbCursorForUpdate ? dbDatabase::dbExclusiveLock : dbDatabase::dbSharedLock);
    dbDatabaseThreadContext* ctx = db->threadContext.get();
    ctx->cursors.link(&cursor);
    cursor.reset();
    dbSearchContext sc;
    sc.db = db;
    sc.condition = NULL;
    sc.firstKey = (char*)key;
    sc.firstKeyInclusion = 1;
    sc.lastKey = (char*)key;
    sc.lastKeyInclusion = 1;
    sc.type = dbField::tpString;
    sc.offs = fd->dbsOffs;
    sc.field = fd;
    sc.cursor = &cursor;
    dbTtree::prefixSearch(db, oid, sc);
    return cursor.getNumberOfRecords();
}

int dbAnyContainer::prefixSearch(dbAnyCursor& cursor, wchar_t const* key)
{
    dbDatabase* db = cursor.table->db;
    db->beginTransaction(cursor.type == dbCursorForUpdate ? dbDatabase::dbExclusiveLock : dbDatabase::dbSharedLock);
    dbDatabaseThreadContext* ctx = db->threadContext.get();
    ctx->cursors.link(&cursor);
    cursor.reset();
    dbSearchContext sc;
    sc.db = db;
    sc.condition = NULL;
    sc.firstKey = (char*)key;
    sc.firstKeyInclusion = 1;
    sc.lastKey = (char*)key;
    sc.lastKeyInclusion = 1;
    sc.type = dbField::tpWString;
    sc.offs = fd->dbsOffs;
    sc.field = fd;
    sc.cursor = &cursor;
    dbTtree::prefixSearch(db, oid, sc);
    return cursor.getNumberOfRecords();
}

int dbAnyContainer::search(dbAnyCursor& cursor, void const* from, void const* till)
{
    dbDatabase* db = cursor.table->db;
    db->beginTransaction(cursor.type == dbCursorForUpdate ? dbDatabase::dbExclusiveLock : dbDatabase::dbSharedLock);
    dbDatabaseThreadContext* ctx = db->threadContext.get();
    ctx->cursors.link(&cursor);
    cursor.reset();
    if (from == NULL && till == NULL) {
        dbTtree::traverseForward(db, oid, &cursor);
    } else {
        dbSearchContext sc;
        sc.db = db;
        sc.condition = NULL;
        sc.firstKey = (char*)from;
        sc.firstKeyInclusion = 1;
        sc.lastKey = (char*)till;
        sc.lastKeyInclusion = 1;
        sc.field = fd;
        sc.type = fd->type;
        sc.offs = fd->dbsOffs;
        sc.cursor = &cursor;
        dbTtree::find(db, oid, sc);
    }
    return cursor.getNumberOfRecords();
}

int dbAnyContainer::spatialSearch(dbAnyCursor& cursor, rectangle const& r, SpatialSearchType type)
{
    dbDatabase* db = cursor.table->db;
    db->beginTransaction(cursor.type == dbCursorForUpdate ? dbDatabase::dbExclusiveLock : dbDatabase::dbSharedLock);
    dbDatabaseThreadContext* ctx = db->threadContext.get();
    ctx->cursors.link(&cursor);
    cursor.reset();
    assert(fd->type != dbField::tpRectangle);
    dbSearchContext sc;
    sc.db = db;
    sc.condition = NULL;
    sc.firstKey = (char*)&r;
    sc.firstKeyInclusion = type;
    dbRtree::find(db, oid, sc);
    return cursor.getNumberOfRecords();
}

dbAnyContainer::dbAnyContainer(char const* name, dbTableDescriptor& desc)
{
    fd = desc.find(name);
}

END_FASTDB_NAMESPACE
