// Mode: -*- C++ -*-

//          GiSdb.h
//
// This provides interface beween GiST and GigaBASE storage

#ifndef GISTDB_H
#define GISTDB_H

#include "GiSTstore.h"
#include "fastdb.h"

struct GiSTtable { 
    char const* name;
    oid_t       rootPageId;
    
    TYPE_DESCRIPTOR((KEY(name, INDEXED), FIELD(rootPageId)));
};

class GiSTdb : public GiSTstore {
  public:
    GiSTdb(dbDatabase& aDB);

    void Create(const char *filename);
    void Open(const char *filename);
    void Close();
    
    void Read(GiSTpage page, char *buf);
    void Write(GiSTpage page, const char *buf);
    GiSTpage Allocate();
    void Deallocate(GiSTpage page);
    void Sync() {}
    int PageSize() const { return dbPageSize; }
    
  private:
    dbDatabase& db;
    oid_t       rootPageId;
    dbQuery     query;
    char const* tableName;
};

#endif
