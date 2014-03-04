// -*- Mode: C++ -*-
//
//          GiSTdb.cpp

#include "GiSTdb.h"
#include "GiSTpath.h"

REGISTER(GiSTtable);

GiSTdb::GiSTdb(dbDatabase& aDB) : GiSTstore(), db(aDB) 
{
    query = "name=",&tableName;
    rootPageId = 0;
}


void 
GiSTdb::Create(const char *tableName)
{
    if (IsOpen()) { 
	return;
    }
    dbCursor<GiSTtable> cursor;
    this->tableName = tableName;
    SetOpen(1);
    if (cursor.select(query) == 0) { 
	GiSTtable table;
	table.name = tableName;
	rootPageId = Allocate();
	table.rootPageId = rootPageId;
	insert(table);
    } else { 
	rootPageId = cursor->rootPageId;
    }
}


void 
GiSTdb::Open(const char *tableName)
{
    dbCursor<GiSTtable> cursor;
    this->tableName = tableName;
    if (cursor.select(query) != 0) { 
	rootPageId = cursor->rootPageId;
	SetOpen(1);
    }
}

void 
GiSTdb::Close()
{
    if (!IsOpen()) { 
	return;
    }
    SetOpen(0);
}

void 
GiSTdb::Read(GiSTpage page, char *buf)
{
    if (IsOpen()) {
	if (page == GiSTRootPage) { 
	    page = rootPageId;
	}
	memcpy(buf, db.get(page), dbPageSize);
    }
}

void 
GiSTdb::Write(GiSTpage page, const char *buf)
{
    if (IsOpen()) {
	if (page == GiSTRootPage) { 
	    page = rootPageId;
	}
	memcpy(db.put(page), buf, dbPageSize);
    }
}

GiSTpage 
GiSTdb::Allocate()
{
    if (!IsOpen()) { 
	return 0;
    }
    oid_t page = db.allocateObject(dbPageObjectMarker);
    byte* pg = db.get(page);
    memset(pg, 0, dbPageSize);
    return page;
}

void 
GiSTdb::Deallocate(GiSTpage page)
{
    db.freeObject(page);
}








