//-< HARRAY.H >------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     25-Mar-2003  K.A. Knizhnik  * / [] \ *
//                          Last update: 25-Mar-2003  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Hierarchical array 
//-------------------------------------------------------------------*--------*

#ifndef __HARRAY_H__
#define __HARRAY_H__

BEGIN_FASTDB_NAMESPACE

#include "fastdb.h"

const size_t dbHArrayPageSize = dbPageSize / sizeof(oid_t);

#ifdef HAS_TEMPLATE_FRIENDS
template<class T>  
class dbHArray : public dbAnyReference { 
  public:
    enum { 
        leafPageSize = dbPageSize / sizeof(T),
        maxArraySize = dbHArrayPageSize*dbHArrayPageSize*leafPageSize
    };

    void create(dbDatabase* db) { 
        db->beginTransaction(dbDatabase::dbExclusiveLock);
        oid = db->allocateObject(dbPageObjectMarker);
        memset(db->get(oid), 0, dbPageSize);
    }

    T get(size_t i, dbDatabase* db) const { 
        assert (oid != 0 && i < maxArraySize); 
        db->beginTransaction(dbDatabase::dbSharedLock);
        oid_t* page = (oid_t*)db->get(oid);
        oid_t pageOid = page[i / (dbHArrayPageSize*leafPageSize)];
        if (pageOid == 0) { 
            return 0;
        }
        page = (oid_t*)db->get(pageOid);
        pageOid = page[i / leafPageSize % dbHArrayPageSize];
        if (pageOid == 0) { 
            return 0;
        }
        T* leaf = (T*)db->get(pageOid);
        return leaf[i % leafPageSize];
    }

    T& set(size_t i, dbDatabase* db) { 
        assert (oid != 0 && i < maxArraySize);
        db->beginTransaction(dbDatabase::dbExclusiveLock);
        oid_t* page = (oid_t*)db->get(oid);
        oid_t pageOid = page[i / (dbHArrayPageSize*leafPageSize)];
        if (pageOid == 0) { 
            pageOid = db->allocateObject(dbPageObjectMarker);
            page = (oid_t*)db->put(oid);
            page[i / (dbHArrayPageSize*leafPageSize)] = pageOid;
            page = (oid_t*)db->get(pageOid);
            memset(page, 0, dbPageSize);
        } else { 
            page = (oid_t*)db->get(pageOid);
        }
        oid_t leafPageOid = page[i / leafPageSize % dbHArrayPageSize];
        T* leaf;
        if (leafPageOid == 0) { 
            leafPageOid = db->allocateObject(dbPageObjectMarker);
            page = (oid_t*)db->put(pageOid);
            page[i / leafPageSize % dbHArrayPageSize] = leafPageOid;
            leaf = (T*)db->get(leafPageOid);
            memset(leaf, 0, dbPageSize);
        } else { 
            leaf = (T*)db->put(leafPageOid);
        }
        return leaf[i % leafPageSize];
    }

    void set(size_t i, T value, dbDatabase* db) { 
        set(i, db) = value;
    }
};
#else
class dbAnyHArray : public dbAnyReference { 
  public:
    void create(dbDatabase* db) { 
        db->beginTransaction(dbDatabase::dbExclusiveLock);
        oid = db->allocateObject(dbPageObjectMarker);
        memset(db->get(oid), 0, dbPageSize);
    }

    byte* get(size_t i, dbDatabase* db, const size_t maxArraySize, const size_t leafPageSize) const { 
        assert (oid != 0 && i < maxArraySize); 
        db->beginTransaction(dbDatabase::dbSharedLock);
        oid_t* page = (oid_t*)db->get(oid);
        oid_t pageOid = page[i / (dbHArrayPageSize*leafPageSize)];
        if (pageOid == 0) { 
            return 0;
        }
        page = (oid_t*)db->get(pageOid);
        pageOid = page[i / leafPageSize % dbHArrayPageSize];
        if (pageOid == 0) { 
            return 0;
        }
        return db->get(pageOid);
    }

    byte* set(size_t i, dbDatabase* db, const size_t maxArraySize, const size_t leafPageSize) { 
        assert (oid != 0 && i < maxArraySize);
        db->beginTransaction(dbDatabase::dbExclusiveLock);
        oid_t* page = (oid_t*)db->get(oid);
        oid_t pageOid = page[i / (dbHArrayPageSize*leafPageSize)];
        if (pageOid == 0) { 
            pageOid = db->allocateObject(dbPageObjectMarker);
            page = (oid_t*)db->put(oid);
            page[i / (dbHArrayPageSize*leafPageSize)] = pageOid;
            page = (oid_t*)db->get(pageOid);
            memset(page, 0, dbPageSize);
        } else { 
            page = (oid_t*)db->get(pageOid);
        }
        oid_t leafPageOid = page[i / leafPageSize % dbHArrayPageSize];
        byte* leaf;
        if (leafPageOid == 0) { 
            leafPageOid = db->allocateObject(dbPageObjectMarker);
            page = (oid_t*)db->put(pageOid);
            page[i / leafPageSize % dbHArrayPageSize] = leafPageOid;
            leaf = db->get(leafPageOid);
            memset(leaf, 0, dbPageSize);
        } else { 
            leaf = db->put(leafPageOid);
        }
        return leaf;
    }
};

template<class T>  
class dbHArray : public dbAnyHArray { 
  public:
    enum { 
        leafPageSize = dbPageSize / sizeof(T),
        maxArraySize = dbHArrayPageSize*dbHArrayPageSize*leafPageSize
    };

    void set(size_t i, T value, dbDatabase* db) { 
        set(i, db) = value;
    }
    T get(size_t i, dbDatabase* db) const { 
        return ((T*)dbAnyHArray::get(i, db, maxArraySize, leafPageSize))[i % leafPageSize];
    }
    T& set(size_t i, dbDatabase* db) { 
        return ((T*)dbAnyHArray::set(i, db, maxArraySize, leafPageSize))[i % leafPageSize];
    }
};
#endif




class dbBitmap : public dbHArray<int4> { 
    typedef dbHArray<int4> base;
  public:
    void create(dbDatabase* db) { 
        base::create(db);
    }

    bool get(size_t i, dbDatabase* db) const { 
        return (base::get(i >> 5, db) & (1 << (i & 31))) != 0;
    }

    void set(size_t i, bool value, dbDatabase* db) {
        int4& mask = base::set(i >> 5, db);
        if (value) { 
            mask |= 1 << (i & 31);
        } else { 
            mask &= ~(1 << (i & 31));
        }
    }
};        

END_FASTDB_NAMESPACE
 
#endif

