//-< HASHTAB.CPP >---------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 19-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Extensible hash table implementation
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "fastdb.h"
#include "hashtab.h"
#include <ctype.h>

BEGIN_FASTDB_NAMESPACE

static const size_t primeNumbers[] = {
    17,             /* 0 */
    37,             /* 1 */
    79,             /* 2 */
    163,            /* 3 */
    331,            /* 4 */
    673,            /* 5 */
    1361,           /* 6 */
    2729,           /* 7 */
    5471,           /* 8 */
    10949,          /* 9 */
    21911,          /* 10 */
    43853,          /* 11 */
    87719,          /* 12 */
    175447,         /* 13 */
    350899,         /* 14 */
    701819,         /* 15 */
    1403641,        /* 16 */
    2807303,        /* 17 */
    5614657,        /* 18 */
    11229331,       /* 19 */
    22458671,       /* 20 */
    44917381,       /* 21 */
    89834777,       /* 22 */
    179669557,      /* 23 */
    359339171,      /* 24 */
    718678369,      /* 25 */
    1437356741,     /* 26 */
    2147483647      /* 27 (largest signed int prime) */
};


oid_t dbHashTable::allocate(dbDatabase* db, size_t nRows)
{
    size_t size = dbInitHashTableSize;
    if (size < nRows) { 
        size = nRows;
    }
    size_t i;
    for (i = 0; i < itemsof(primeNumbers)-1 && primeNumbers[i] < size; i++);
    size = primeNumbers[i];
    oid_t hashId = db->allocateObject(dbHashTableMarker);
    int nPages = (int)((size+dbIdsPerPage-1) / dbIdsPerPage);
    oid_t pageId = db->allocateId(nPages);
    offs_t pos = db->allocate(nPages*dbPageSize);
    assert((pos & (dbPageSize-1)) == 0);
    memset(db->baseAddr+pos, 0, nPages*dbPageSize);
    dbHashTable* hash = (dbHashTable*)db->get(hashId);
    hash->size = (nat4)size;
    hash->page = pageId;
    hash->used = 0;
    while (--nPages >= 0) {
        db->currIndex[pageId++] = pos + dbPageObjectMarker;
        pos += dbPageSize;
    }
    return hashId;
}
 

static unsigned stringHashFunction(void const* ptr, size_t size)
{
    unsigned h;
    byte* key = (byte*)ptr;
    int keylen = (int)size;
    for (h = 0; --keylen >= 0; h = h*31 + *key++);
    return h;
}

static unsigned stringIgnoreCaseHashFunction(void const* ptr, size_t size)
{
    unsigned h;
    byte* key = (byte*)ptr;
    int keylen = (int)size;
    for (h = 0; --keylen >= 0;) { 
        int code = *key++;
        h = h*31 + toupper(code);
    }
    return h;
}

static unsigned wstringHashFunction(void const* ptr, size_t size)
{
    unsigned h;
    wchar_t* key = (wchar_t*)ptr;
    int keylen = (int)size;
    for (h = 0; --keylen >= 0; h = h*31 + *key++);
    return h;
}

static unsigned wstringIgnoreCaseHashFunction(void const* ptr, size_t size)
{
    unsigned h;
    wchar_t* key = (wchar_t*)ptr;
    int keylen = (int)size;
    for (h = 0; --keylen >= 0;) { 
        int code = *key++;
        h = h*31 + towupper(code);
    }
    return h;
}

static unsigned addByteShiftHashFunction(void const* ptr, size_t size)
{
    unsigned h;
    int keylen = (int)size;
    byte* key = (byte*)ptr + keylen;
    for (h = 0; --keylen >= 0; h = (h << 8) + *--key);
    return h;
}

static unsigned xorByteRotateHashFunction(void const* ptr, size_t size)
{
    unsigned h;
    int keylen = (int)size;
    byte* key = (byte*)ptr;
    for (h = 0; --keylen >= 0; h = (h << 8) ^ (h >> 24) ^ *key++);
    return h;
}

static unsigned byteHashFunction(void const* key, size_t) 
{
    return *(db_nat1*)key;
}

static unsigned shortHashFunction(void const* key, size_t) 
{
    return *(db_nat2*)key;
}

static unsigned intHashFunction(void const* key, size_t) 
{
    return *(db_nat4*)key;
}

static unsigned longHashFunction(void const* key, size_t) 
{
    return *(db_nat4*)key ^ *((db_nat4*)key+1);
}

dbUDTHashFunction dbHashTable::getHashFunction(int version, dbFieldDescriptor* fd)
{
    if (fd->type == dbField::tpString) { 
#ifdef IGNORE_CASE 
        return &stringIgnoreCaseHashFunction;
#else
        return (fd->indexType & CASE_INSENSITIVE) ? &stringIgnoreCaseHashFunction : &stringHashFunction;
#endif
    } else if (version < 288) { 
        return &stringHashFunction;
    } else if (version < 308) { 
        return &addByteShiftHashFunction;
    } else { 
        switch (fd->type) { 
          case dbField::tpBool:
          case dbField::tpInt1:
            return &byteHashFunction;
          case dbField::tpInt2:  
            return &shortHashFunction;
          case dbField::tpInt4:
          case dbField::tpReal4: 
            return &intHashFunction;
          case dbField::tpInt8:
          case dbField::tpReal8: 
            return &longHashFunction;
          case dbField::tpWString: 
#ifdef IGNORE_CASE 
            return &wstringIgnoreCaseHashFunction;
#else
            return (fd->indexType & CASE_INSENSITIVE) ? &wstringIgnoreCaseHashFunction : &wstringHashFunction;
#endif
          default:
            return (version < 333) ? &addByteShiftHashFunction 
                : (version < 359) ? &stringHashFunction
                : &xorByteRotateHashFunction;
        }
    }
}

inline unsigned calculateHashCode(dbDatabase* db, byte* record, byte* key, dbFieldDescriptor* fd) 
{ 
    if (fd->_hashcode == NULL) { 
        fd->_hashcode = dbHashTable::getHashFunction(db->getVersion(), fd);
    }
    switch (fd->type) {
      case dbField::tpString:
      case dbField::tpWString:
        return fd->_hashcode(record + ((dbVarying*)key)->offs, ((dbVarying*)key)->size - 1);
        break;
      case dbField::tpArray:
        return fd->_hashcode(record + ((dbVarying*)key)->offs, ((dbVarying*)key)->size*fd->elemSize);
        break;
      default:
        return fd->_hashcode(key, fd->dbsSize);
    }
}

void dbHashTable::insert(dbDatabase* db, dbFieldDescriptor* fd, oid_t rowId, size_t nRows)
{
    oid_t hashId = fd->hashTable;
    dbHashTable* hash = (dbHashTable*)db->get(hashId);
    byte* record = db->get(rowId);
    byte* key = record + fd->dbsOffs;
    unsigned hashcode = calculateHashCode(db, record, key, fd);
    size_t size = hash->size;
    oid_t pageId = hash->page;
    if (size < nRows && hash->used*3/2 > size) {
        TRACE_MSG(("Reallocate hash table, used=%ld, size=%ld\n", hash->used, size));
        int nPages = (int)((size+dbIdsPerPage-1) / dbIdsPerPage);
        size_t i;
        for (i = 0; i < itemsof(primeNumbers)-1 && primeNumbers[i] < size; i++);
        if (i < itemsof(primeNumbers)-1) { 
            i += 1;
        }
        size = primeNumbers[i];
        int nNewPages = (int)((size+dbIdsPerPage-1) / dbIdsPerPage);
        oid_t newPageId = db->allocateId(nNewPages);
        offs_t pos = db->allocate(nNewPages*dbPageSize);
        assert((pos & (dbPageSize-1)) == 0);
        memset(db->baseAddr + pos, 0, nNewPages*dbPageSize);
        hash = (dbHashTable*)db->put(hashId);
        hash->size = (nat4)size;
        hash->page = newPageId;
        size_t used = 0;
        while (--nPages >= 0) { 
            for (i = 0; i < dbIdsPerPage; i++) { 
                oid_t itemId = ((oid_t*)db->get(pageId))[i];
                while (itemId != 0) { 
                    dbHashTableItem* item = (dbHashTableItem*)db->get(itemId);
                    oid_t nextId = item->next;
                    unsigned h = item->hash % size;
                    oid_t* tab = (oid_t*)(db->baseAddr + pos);
                    if (item->next != tab[h]) { 
                        item = (dbHashTableItem*)db->put(itemId);
                        tab = (oid_t*)(db->baseAddr + pos);
                        item->next = tab[h];
                    }
                    if (tab[h] == 0) { 
                        used += 1;
                    }
                    tab[h] = itemId;
                    itemId = nextId;
                }
            }
            db->freeObject(pageId++);
        }
        ((dbHashTable*)db->get(hashId))->used = (nat4)used;
        pageId = newPageId;
        while (--nNewPages >= 0) { 
            db->currIndex[newPageId++] = pos + dbPageObjectMarker;
            pos += dbPageSize;
        }
    } 
    oid_t itemId = db->allocateObject(dbHashTableItemMarker);
    unsigned h = hashcode % size;
    oid_t* ptr = (oid_t*)db->put(pageId + h/dbIdsPerPage) + h%dbIdsPerPage;
    dbHashTableItem* item = (dbHashTableItem*)db->get(itemId);
    item->record = rowId;
    item->hash = hashcode;
    item->next = *ptr;
    *ptr = itemId;
    if (item->next == 0) { 
        ((dbHashTable*)db->get(hashId))->used += 1;
        db->file.markAsDirty(db->currIndex[hashId] & ~dbInternalObjectMarker, sizeof(dbHashTable));
    }
}

    
void dbHashTable::remove(dbDatabase* db, dbFieldDescriptor* fd, oid_t rowId)
{
    oid_t hashId = fd->hashTable;
    dbHashTable* hash = (dbHashTable*)db->get(hashId);
    byte* record = (byte*)db->getRow(rowId);
    byte* key = record + fd->dbsOffs;
    unsigned hashcode = calculateHashCode(db, record, key, fd);
    unsigned h = hashcode % hash->size;
    oid_t pageId = hash->page + h / dbIdsPerPage;
    int i = h % dbIdsPerPage;
    oid_t itemId = ((oid_t*)db->get(pageId))[i];
    oid_t prevItemId = 0;
    while (true) { 
        assert(itemId != 0);
        dbHashTableItem* item = (dbHashTableItem*)db->get(itemId);
        if (item->record == rowId) { 
            oid_t next = item->next;
            if (prevItemId == 0) { 
                if (next == 0) { 
                    hash->used -= 1; // consistency can be violated
                    db->file.markAsDirty(db->currIndex[hashId] & ~dbInternalObjectMarker, 
                                         sizeof(dbHashTable));
                }
                *((oid_t*)db->put(pageId) + i) = next;
            } else {    
                item = (dbHashTableItem*)db->put(prevItemId);
                item->next = next;
            }
            db->freeObject(itemId);
            return;
        }
        prevItemId = itemId;
        itemId = item->next;
    }
}
    
void dbHashTable::find(dbDatabase* db, oid_t hashId, dbSearchContext& sc)
{
    dbHashTable* hash = (dbHashTable*)db->get(hashId);
    if (hash->size == 0) { 
        return;
    }
    if (sc.field->_hashcode == NULL) { 
        sc.field->_hashcode = getHashFunction(db->getVersion(), sc.field);
    }
    unsigned keylen;
    unsigned hashcode;
    switch (sc.type) {
      case dbField::tpString:
        keylen = (unsigned)strlen(sc.firstKey);
        hashcode = sc.field->_hashcode(sc.firstKey, keylen);
        break;
      case dbField::tpWString:
        keylen = (unsigned)wcslen((wchar_t*)sc.firstKey);
        hashcode = sc.field->_hashcode(sc.firstKey, keylen);
        break;
      case dbField::tpArray:
        keylen = ((dbAnyArray*)sc.firstKey)->length();
        hashcode = sc.field->_hashcode(((dbAnyArray*)sc.firstKey)->base(), keylen*sc.field->elemSize);
        break;
      default:
        keylen = sc.field->dbsSize;
        hashcode = sc.field->_hashcode(sc.firstKey, keylen);
    }
    unsigned h = hashcode % hash->size;
    oid_t itemId = 
        ((oid_t*)db->get(hash->page + h/dbIdsPerPage))[h % dbIdsPerPage];
    dbTable* table = (dbTable*)db->getRow(sc.cursor->table->tableId);
    while (itemId != 0) { 
        dbHashTableItem* item = (dbHashTableItem*)db->get(itemId);
        sc.probes += 1;
        if (item->hash == hashcode) { 
            byte* rec = (byte*)db->getRow(item->record);
            bool equals;
            switch (sc.type) { 
              case dbField::tpString:
              case dbField::tpWString:
                equals = sc.field->_comparator(sc.firstKey, rec+((dbVarying*)(rec+sc.offs))->offs, MAX_STRING_LENGTH) == 0;
                break;
              case dbField::tpArray:
              {
                  dbArray<byte> a(rec+((dbVarying*)(rec+sc.offs))->offs, ((dbVarying*)(rec+sc.offs))->size);
                  equals = sc.field->_comparator(sc.firstKey, &a, sc.field->elemSize) == 0;
                  break;
              }
             default:
                equals = sc.field->_comparator(sc.firstKey, rec + sc.offs, keylen) == 0; 
            }
            if (equals) { 
                if (!sc.condition 
                    || db->evaluate(sc.condition, item->record, table, sc.cursor))
                {
                    if (!sc.cursor->add(item->record)) { 
                        return;
                    }
                }
            }
        }
        itemId = item->next;
    }
}



void dbHashTable::purge(dbDatabase* db, oid_t hashId)
{
    dbHashTable* hash = (dbHashTable*)db->put(hashId);
    oid_t pageId = hash->page;
    int nPages = (hash->size + dbIdsPerPage  - 1) / dbIdsPerPage;
    hash->used = 0;
    while (--nPages >= 0) { 
        for (size_t i = 0; i < dbIdsPerPage; i++) { 
            oid_t itemId = ((oid_t*)db->get(pageId))[i];
            while (itemId != 0) { 
                oid_t nextId = ((dbHashTableItem*)db->get(itemId))->next;
                db->freeObject(itemId);
                itemId = nextId;
            }
        }
        memset(db->put(pageId++), 0, dbPageSize);
    }
}

void dbHashTable::drop(dbDatabase* db, oid_t hashId)
{
    dbHashTable* hash = (dbHashTable*)db->get(hashId);
    oid_t pageId = hash->page;
    int nPages = (hash->size + dbIdsPerPage - 1) / dbIdsPerPage;
    while (--nPages >= 0) { 
        for (size_t i = 0; i < dbIdsPerPage; i++) { 
            oid_t itemId = ((oid_t*)db->get(pageId))[i];
            while (itemId != 0) { 
                oid_t nextId = ((dbHashTableItem*)db->get(itemId))->next;
                db->freeObject(itemId);
                itemId = nextId;
            }
        }
        db->freeObject(pageId++);
    }
    db->freeObject(hashId);
}

END_FASTDB_NAMESPACE






