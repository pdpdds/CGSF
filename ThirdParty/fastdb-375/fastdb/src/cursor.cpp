//-< CURSOR.CPP >----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 21-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Table cursor
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "fastdb.h"
#include "compiler.h"
#include "hashtab.h"
#include "ttree.h"
#include "rtree.h"
#include <ctype.h>

BEGIN_FASTDB_NAMESPACE

dbSelection::segment*  dbSelection::createNewSegment(dbSelection::segment* after)
{
    return new segment(after);
}

inline void dbSelection::reset()
{
    for (segment *next, *seg = first; seg != NULL; seg = next) { 
        next = seg->next;
        delete seg;
    }
    first = last = curr = NULL;
    nRows = 0;
    pos = 0;
}

void dbSelection::reverse() 
{
    segment *next, *seg;
    for (seg = first; seg != NULL; seg = next) { 
        next = seg->next;
        seg->next = seg->prev;
        seg->prev = next;
        for (int l = 0, r = seg->nRows-1; l < r; l++, r--) { 
            oid_t oid = seg->rows[l];
            seg->rows[l] = seg->rows[r];
            seg->rows[r] = oid;
        }
    }
    seg = first;
    first = last;
    last = seg;
}

void dbSelection::toArray(oid_t* oids) const
{
    for (segment *seg = first; seg != NULL; seg = seg->next) { 
        for (int i = 0, n = seg->nRows; i < n; i++) {
            *oids++ = seg->rows[i];
        }
    }
}

void dbSelection::truncate(cardinality_t from, cardinality_t length)
{
    if (from == 0 && length >= nRows) { 
        // do nothing
        return;
    }
    segment *src = NULL;
    bool empty = true;
    if (from < nRows) { 
        for (src = first; src != NULL; src = src->next) { 
            if (from < (cardinality_t)src->nRows) { 
                empty = false;
                break;
            }
            from -= src->nRows;
        }
    }
    if (from + length > nRows) { 
        length = nRows - from;
    }
    nRows = 0;
    segment* dst = first;
    size_t pos = 0;
    if (!empty) { 
        while (length != 0) { 
            size_t n = src->nRows - from;
            if (n > length) { 
                n = length;
            }
            if (size_t(dst->nRows) == pos) { 
                dst = dst->next;
                pos = 0;
            }
            if (n > dst->nRows - pos) { 
                n = dst->nRows - pos;
            }
            memcpy(dst->rows + pos, src->rows + from, n*sizeof(oid_t));
            pos += n;
            length -= n;
            nRows += n;
            if ((from += n) == cardinality_t(src->nRows)) { 
                if ((src = src->next) == NULL) { 
                    break;
                }
                from = 0;
            }
        }
    }
    dst->nRows = pos;
    segment* next = dst->next;
    dst->next = NULL;
    while (next != NULL) { 
        dst = next;
        next = dst->next; 
        delete dst;
    }
}

int dbSelection::compare(oid_t o1, oid_t o2, dbOrderByNode* order)
{
    dbDatabase* db = order->table->db;
    char* p = (char*)db->getRow(o1);
    char* q = (char*)db->getRow(o2);
    int diff = 0;
    do { 
        if (order->expr != NULL) { 
            dbInheritedAttribute   iattr1;
            dbInheritedAttribute   iattr2;
            dbSynthesizedAttribute sattr1;
            dbSynthesizedAttribute sattr2;
            iattr1.db = iattr2.db = db;
            iattr1.table = iattr2.table = (dbTable*)db->getRow(order->table->tableId);
            iattr1.record = sattr1.base = (byte*)p;
            iattr1.oid = o1;
            iattr2.record = sattr2.base = (byte*)q;
            iattr2.oid = o2;
            db->execute(order->expr, iattr1, sattr1);
            db->execute(order->expr, iattr2, sattr2);
            switch (order->expr->type) { 
              case tpInteger:
                diff = sattr1.ivalue < sattr2.ivalue ? -1 : sattr1.ivalue == sattr2.ivalue ? 0 : 1;
                break;
              case tpReal:
                diff = sattr1.fvalue < sattr2.fvalue ? -1 : sattr1.fvalue == sattr2.fvalue ? 0 : 1;
                break;
              case tpBoolean:
                diff = sattr1.bvalue != 0 ? sattr2.bvalue != 0 ? 0 : 1 : sattr2.bvalue != 0 ? -1 : 0;
                break;
              case tpString:
#ifdef USE_LOCALE_SETTINGS
#ifdef IGNORE_CASE
                diff = stricoll((char*)sattr1.array.base, (char*)sattr2.array.base); 
#else
                diff = strcoll((char*)sattr1.array.base, (char*)sattr2.array.base); 
#endif
#else
#ifdef IGNORE_CASE
                diff = stricmp((char*)sattr1.array.base, (char*)sattr2.array.base);
#else
                diff = strcmp((char*)sattr1.array.base, (char*)sattr2.array.base);
#endif
#endif
                break;
              case tpWString:
#ifdef IGNORE_CASE
                diff = wcsicmp((wchar_t*)sattr1.array.base, (wchar_t*)sattr2.array.base);
#else
                diff = wcscmp((wchar_t*)sattr1.array.base, (wchar_t*)sattr2.array.base);
#endif
                break;
              case tpReference:
                diff = sattr1.oid < sattr2.oid ? -1 : sattr1.oid == sattr2.oid ? 0 : 1;
                break;
              default:
                assert(false);
            }
        } else { 
            int offs = order->field->dbsOffs;
            switch (order->field->type) { 
              case dbField::tpBool:
                diff = *(bool*)(p + offs) - *(bool*)(q + offs);
                break;
              case dbField::tpInt1:
                diff = *(int1*)(p + offs) - *(int1*)(q + offs);
                break;
              case dbField::tpInt2:
                diff = *(int2*)(p + offs) - *(int2*)(q + offs);
                break;
              case dbField::tpInt4:
                diff = *(int4*)(p + offs) < *(int4*)(q + offs) ? -1 
                    : *(int4*)(p + offs) == *(int4*)(q + offs) ? 0 : 1;
                break;
              case dbField::tpInt8:
                diff = *(db_int8*)(p + offs) < *(db_int8*)(q + offs) ? -1 :
                    *(db_int8*)(p + offs) == *(db_int8*)(q + offs) ? 0 : 1;
                break;
              case dbField::tpReference:
                diff = *(oid_t*)(p + offs) < *(oid_t*)(q + offs) ? -1 :
                    *(oid_t*)(p + offs) == *(oid_t*)(q + offs) ? 0 : 1;
                break;
              case dbField::tpReal4:
                diff = *(real4*)(p + offs) < *(real4*)(q + offs) ? -1 :
                    *(real4*)(p + offs) == *(real4*)(q + offs) ? 0 : 1;
                break;
              case dbField::tpReal8:
                diff = *(real8*)(p + offs) < *(real8*)(q + offs) ? -1 :
                    *(real8*)(p + offs) == *(real8*)(q + offs) ? 0 : 1;
                break;
              case dbField::tpString:
                if (order->field != NULL) { 
                    diff = order->field->_comparator(p + ((dbVarying*)(p + offs))->offs, 
                                                     q + ((dbVarying*)(q + offs))->offs, 
                                                     MAX_STRING_LENGTH);
                } else { 
#ifdef USE_LOCALE_SETTINGS
#ifdef IGNORE_CASE
                    diff = stricoll(p + ((dbVarying*)(p + offs))->offs, q + ((dbVarying*)(q + offs))->offs);
#else
                    diff = strcoll(p + ((dbVarying*)(p + offs))->offs, q + ((dbVarying*)(q + offs))->offs);
#endif
#else
#ifdef IGNORE_CASE
                    diff = stricmp(p + ((dbVarying*)(p + offs))->offs, q + ((dbVarying*)(q + offs))->offs);
#else
                    diff = strcmp(p + ((dbVarying*)(p + offs))->offs, q + ((dbVarying*)(q + offs))->offs);
#endif
#endif
                }
                break;
              case dbField::tpWString:
                if (order->field != NULL) { 
                    diff = order->field->_comparator(p + ((dbVarying*)(p + offs))->offs, 
                                                     q + ((dbVarying*)(q + offs))->offs, 
                                                     MAX_STRING_LENGTH);
                } else { 
#ifdef IGNORE_CASE
                    diff = wcsicmp((wchar_t*)(p + ((dbVarying*)(p + offs))->offs), 
                                      (wchar_t*)(q + ((dbVarying*)(q + offs))->offs));
#else
                    diff = wcscmp((wchar_t*)(p + ((dbVarying*)(p + offs))->offs), 
                                  (wchar_t*)(q + ((dbVarying*)(q + offs))->offs));
#endif
                }
                break;
              case dbField::tpArray:
              {
                  dbArray<char> a1(p + ((dbVarying*)(p + offs))->offs, ((dbVarying*)(p + offs))->size);
                  dbArray<char> a2(q + ((dbVarying*)(q + offs))->offs, ((dbVarying*)(q + offs))->size);                  
                  diff = order->field->_comparator(&a1, &a2, 0);
                  break;
              }
              case dbField::tpRawBinary:
                diff = order->field->_comparator(p + offs, q + offs, order->field->dbsSize);
                break;
              default:
                assert(false);
            }
        }
        if (!order->ascent) { 
            diff = -diff;
        }
    } while (diff == 0 && (order = order->next) != NULL);

    return diff;
}

#ifdef USE_HEAP_SORT

#define ELEM(i)   index[(i-1)/quantum]->rows[(i-1)%quantum]
#define SWAP(i,j) temp = ELEM(i), ELEM(i) = ELEM(j), ELEM(j) = temp

void dbSelection::sort(dbDatabase* db, dbOrderByNode* order)
{
    size_t i, j, k, n = nRows;
    oid_t temp;
    if (n <= 1) { 
        return;
    }
    TRACE_MSG(("Sort %d records\n", n));
    segment** index = new segment*[(n + quantum - 1) / quantum];
    segment* seg = first;
    for (i = 0; seg != NULL; seg = seg->next) { 
        index[i++] = seg;
    }
    for (i = n/2, j = i; i >= 1; i--) { 
        k = i;
        oid_t top = ELEM(k);
        do { 
            if (k*2 == n || compare(ELEM(k*2), ELEM(k*2+1), order) > 0) { 
                if (compare(top, ELEM(k*2), order) >= 0) {
                    break;
                }
                ELEM(k) = ELEM(k*2);
                k = k*2;
            } else { 
                if (compare(top, ELEM(k*2+1), order) >= 0) {
                    break;
                }
                ELEM(k) = ELEM(k*2+1);
                k = k*2+1;
            }
        } while (k <= j);
        ELEM(k) = top; 
    }
    for (i = n; i >= 2; i--) { 
        SWAP(1, i);
        oid_t top = ELEM(1);
        for (k = 1, j = (i-1)/2; k <= j;) { 
            if (k*2 == i-1 || compare(ELEM(k*2), ELEM(k*2+1), order) > 0) { 
                if (compare(top, ELEM(k*2), order) >= 0) {
                    break;
                }
                ELEM(k) = ELEM(k*2);
                k = k*2;
            } else { 
                if (compare(top, ELEM(k*2+1), order) >= 0) {
                    break;
                }
                ELEM(k) = ELEM(k*2+1);
                k = k*2+1;
            }
        } 
        ELEM(k) = top; 
    }
    delete[] index;
}

#else

struct dbSortContext {
    dbOrderByNode* order;
};
static dbThreadContext<dbSortContext> sortThreadContext;

#ifdef USE_STDLIB_QSORT

static int compareRecords(void const* a, void const* b)
{
    dbSortContext* ctx = sortThreadContext.get();
    return dbSelection::compare(*(oid_t*)a, *(oid_t*)b, ctx->order);
}


void dbSelection::sort(dbDatabase* db, dbOrderByNode* order)
{
    size_t n = nRows;
    if (n <= 1) { 
        return;
    }
    TRACE_MSG(("Sort %d records\n", n));
    oid_t* oids = new oid_t[n];
    toArray(oids);
    dbSortContext ctx;
    ctx.order = order;
    sortThreadContext.set(&ctx);    
    qsort(oids, n, sizeof(oid_t), &compareRecords);
    oid_t* p = oids;
    for (segment *seg = first; seg != NULL; seg = seg->next) { 
        for (int i = 0, n = seg->nRows; i < n; i++) {
            seg->rows[i] = *p++;
        }
    }
}

#else

#include "iqsort.h"

struct ObjectRef {
    oid_t oid;

    bool operator > (ObjectRef& ref) { 
        return compare(this, &ref) > 0;
    }

    bool operator < (ObjectRef& ref) { 
        return compare(this, &ref) < 0;
    }

    bool operator >= (ObjectRef& ref) { 
        return compare(this, &ref) >= 0;
    }

    bool operator <= (ObjectRef& ref) { 
        return compare(this, &ref) <= 0;
    }

    bool operator == (ObjectRef& ref) { 
        return compare(this, &ref) == 0;
    }

    bool operator != (ObjectRef& ref) { 
        return compare(this, &ref) != 0;
    }

    static int compare(ObjectRef* a, ObjectRef* b) { 
        dbSortContext* ctx = sortThreadContext.get();
        return dbSelection::compare(a->oid, b->oid, ctx->order);
    }
};


void dbSelection::sort(dbDatabase* db, dbOrderByNode* order)
{
    size_t n = nRows;
    if (n <= 1) { 
        return;
    }
    TRACE_MSG(("Sort %d records\n", n));
    ObjectRef* refs = new ObjectRef[n];
    segment *seg;
    int k = 0;
    for (seg = first; seg != NULL; seg = seg->next) { 
        for (int i = 0, nr = seg->nRows; i < nr; i++) {
            refs[k++].oid = seg->rows[i];
        }
    }
    dbSortContext ctx;
    ctx.order = order;
    sortThreadContext.set(&ctx);    
    iqsort(refs, n);
    k = 0;
    for (seg = first; seg != NULL; seg = seg->next) { 
        for (int i = 0, nr = seg->nRows; i < nr; i++) {
            seg->rows[i] = refs[k++].oid;
        }
    }
    delete[] refs;
}

#endif
#endif

void dbAnyCursor::checkForDuplicates() 
{ 
    if (!eliminateDuplicates && checkForDuplicatedIsEnabled && limit > 1) { 
        eliminateDuplicates = true;
        size_t size = (db->currIndexSize + 31) / 32;
        if (size > bitmapSize) { 
            delete[] bitmap;
            bitmap = new int4[size];
            bitmapSize = size;
        }
        memset(bitmap, 0, size*4);
    }
}

void dbAnyCursor::deallocateBitmap() 
{
    if (bitmap != NULL) { 
        delete[] bitmap;
        bitmapSize = 0;
        bitmap = NULL;
    }
}

oid_t* dbAnyCursor::toArrayOfOid(oid_t* arr) const
{ 
    if (arr == NULL) { 
        arr = new oid_t[selection.nRows];
    }
    if (allRecords) { 
        oid_t* oids = arr;
        for (oid_t oid = firstId; oid != 0; oid = db->getRow(oid)->next) { 
            *oids++ = oid;
        }
    } else { 
        selection.toArray(arr);
    }
    return arr;
}

void dbAnyCursor::setCurrent(dbAnyReference const& ref) 
{ 
    if (ref.oid == 0) { 
        db->handleError(dbDatabase::NullReferenceError, "Attempt to set NULL reference as cursor current value");
    } 
    reset();
    db->beginTransaction(type == dbCursorForUpdate 
                         ? dbDatabase::dbExclusiveLock : dbDatabase::dbSharedLock);
    db->threadContext.get()->cursors.link(this);
    currId = ref.oid;
    add(currId);
    if (prefetch) { 
        fetch();
    }
}


cardinality_t dbAnyCursor::selectByKey(char const* key, void const* value)
{
    dbFieldDescriptor* field = table->find(key);
    assert(field != NULL);
    return selectByKey(field, value);
}

cardinality_t dbAnyCursor::selectByKey(dbFieldDescriptor* field, void const* value)
{
    assert(field->hashTable != 0 || field->tTree != 0);
    reset();
    db->beginTransaction(type == dbCursorForUpdate 
                         ? dbDatabase::dbExclusiveLock : dbDatabase::dbSharedLock);
    db->threadContext.get()->cursors.link(this);
    dbSearchContext sc;
    sc.db = db;
    sc.probes = 0;
    sc.offs = field->dbsOffs;
    sc.cursor = this;
    sc.condition = NULL;
    sc.prefixLength = 0;
    sc.firstKey = sc.lastKey = (char*)value;
    sc.firstKeyInclusion = sc.lastKeyInclusion = true;
    sc.field = field;
    sc.type = field->type;
    if (field->hashTable != 0) { 
        dbHashTable::find(db, field->hashTable, sc);
    } else { 
        dbTtree::find(db, field->tTree, sc);
    }
    if (gotoFirst() && prefetch) {
        fetch();
    }
    return selection.nRows;
}

int dbAnyCursor::seek(oid_t oid)
{
    int pos = 0;
    if (gotoFirst()) { 
        do { 
            if (currId == oid) { 
                if (prefetch) { 
                    fetch();
                }
                return pos;
            }
            pos += 1;
        } while (gotoNext());
    }
    return -1;
}

bool dbAnyCursor::skip(int n) { 
    while (n > 0) { 
        if (!gotoNext()) { 
            return false;
        }
        n -= 1;
    } 
    while (n < 0) { 
        if (!gotoPrev()) { 
            return false;
        }
        n += 1;
    } 
    if (prefetch) { 
        fetch();
    }
    return true;
}

cardinality_t dbAnyCursor::selectByKeyRange(char const* key, void const* minValue, 
                                  void const* maxValue)
{
    dbFieldDescriptor* field = table->find(key);
    assert(field != NULL);
    return selectByKeyRange(field, minValue, maxValue);
}

cardinality_t dbAnyCursor::selectByKeyRange(dbFieldDescriptor* field, void const* minValue, 
                                  void const* maxValue)
{
    assert(field->tTree != 0);
    reset();
    db->beginTransaction(type == dbCursorForUpdate 
                         ? dbDatabase::dbExclusiveLock : dbDatabase::dbSharedLock);
    db->threadContext.get()->cursors.link(this);
    dbSearchContext sc;
    sc.db = db;
    sc.probes = 0;
    sc.offs = field->dbsOffs;
    sc.cursor = this;
    sc.condition = NULL;
    sc.prefixLength = 0;
    sc.firstKey = (char*)minValue;
    sc.lastKey = (char*)maxValue;
    sc.firstKeyInclusion = sc.lastKeyInclusion = true;
    sc.field = field;
    sc.type = field->type;
    dbTtree::find(db, field->tTree, sc);
    if (gotoFirst() && prefetch) {
        fetch();
    }
    return selection.nRows;
}


void dbAnyCursor::remove()
{
    oid_t removedId = currId;
    lastRecordWasDeleted = false;
    if (removedId == 0) { 
        db->handleError(dbDatabase::NoCurrentRecord, "Attempt to remove unexisted record");
    } 
    if (type != dbCursorForUpdate) {
        db->handleError(dbDatabase::ReadonlyCursor);
    }
    if (allRecords) { 
        dbRecord* rec = db->getRow(removedId);
        oid_t nextId = rec->next;
        oid_t prevId = rec->prev;
        if (nextId != 0) { 
            if (currId == firstId) {
                firstId = currId = nextId;
            } else { 
                currId = nextId;
            }
        } else { 
            lastRecordWasDeleted = true;
            if (removedId == firstId) {
                firstId = lastId = currId = 0;
            } else { 
                lastId = currId = prevId;
            }
        }
    } else { 
        if (selection.curr != NULL) { 
            if (--selection.curr->nRows == 0 || selection.pos == selection.curr->nRows) { 
                dbSelection::segment* next = selection.curr->next;
                dbSelection::segment* prev = selection.curr->prev;
                if (selection.curr->nRows == 0) { 
                    if (prev != NULL) { 
                        prev->next = next;
                    } else { 
                        selection.first = next;
                    }
                    if (next != NULL) { 
                        next->prev = prev;
                    } else { 
                        selection.last = prev;
                    }
                    delete selection.curr;
                }
                if (next != NULL) { 
                    currId = next->rows[0];
                    selection.curr = next;
                    selection.pos = 0;
                } else { 
                    lastRecordWasDeleted = true;
                    if ((selection.curr = selection.last) != NULL) { 
                        selection.pos = selection.curr->nRows-1;
                        currId = selection.curr->rows[selection.pos];
                    } else { 
                        currId = 0;
                    }
                }
            } else { 
                memcpy(selection.curr->rows + selection.pos, 
                       selection.curr->rows + selection.pos + 1, 
                       (selection.curr->nRows - selection.pos)
                       *sizeof(oid_t));
                currId = selection.curr->rows[selection.pos];
            }    
        } else { 
            currId = 0;
        }
    }
    byte* saveRecord = record;
    record = NULL;
    db->remove(table, removedId);
    record = saveRecord;
    removed = true;
    if (currId != 0 && prefetch) {
        fetch();        
    }
}


void dbAnyCursor::removeAllSelected()
{
    if (type != dbCursorForUpdate) {
        db->handleError(dbDatabase::ReadonlyCursor);
    }
    byte* saveRecord = record;
    record = NULL;
    if (allRecords) { 
        removeAll();
    } else if (selection.first != NULL) { 
        dbSelection::segment* curr;
        for (curr = selection.first; curr != NULL; curr = curr->next) { 
            for (int i = 0, n = curr->nRows; i < n; i++) { 
                db->remove(table, curr->rows[i]);
            }
        }
        reset();
    } else if (currId != 0) {   
        db->remove(table, currId);
        currId = 0;
    }
    record = saveRecord;
}


bool dbAnyCursor::hasNext() const
{ 
    return allRecords
        ? currId != 0 && db->getRow(currId)->next != 0
        : selection.curr != NULL && (selection.pos+1 < selection.curr->nRows || selection.curr->next != NULL);
}

byte* dbAnyCursor::fetchNext()
{
    if (!removed) {
        if (gotoNext()) {
            fetch();
            return record;
        }
    } else { 
        removed = false; 
        if (currId != 0 && !lastRecordWasDeleted) { 
            if (!prefetch) { 
                fetch();
            }
            return record;
        }
    }
    return NULL;
}

byte* dbAnyCursor::fetchPrev()
{
    if (removed) {
        removed = false; 
        if (lastRecordWasDeleted) { 
            if (currId != 0) { 
                if (!prefetch) { 
                    fetch();
                }
                return record;
            }
            return NULL;
        }
    }
    if (gotoPrev()) {
        fetch();
        return record;
    }
    return NULL;
}

bool dbAnyCursor::moveNext() 
{
    if (!removed) { 
        return gotoNext(); 
    } else {
        removed = false;
        return !lastRecordWasDeleted;
    }
}
    
bool dbAnyCursor::movePrev() 
{
    if (!removed) { 
        return gotoPrev(); 
    } else {
        removed = false;
        return lastRecordWasDeleted ? (currId != 0) : gotoPrev();
    }
}
    

bool dbAnyCursor::gotoNext() 
{ 
    removed = false;
    if (allRecords) { 
        if (currId != 0) { 
            oid_t next = db->getRow(currId)->next;
            if (next != 0) {
                currId = next;
                return true;
            } 
        }
    } else if (selection.curr != NULL) { 
        if (++selection.pos == selection.curr->nRows) { 
            if (selection.curr->next == NULL) { 
                selection.pos -= 1;
                return false;
            }
            selection.pos = 0;
            selection.curr = selection.curr->next;
        }
        currId = selection.curr->rows[selection.pos];
        return true;
    }  
    return false;
}

bool dbAnyCursor::isLast() const
{ 
    if (allRecords) { 
        if (currId != 0) { 
            return db->getRow(currId)->next == 0;
        }
    } else if (selection.curr != NULL) { 
        return selection.pos+1 == selection.curr->nRows
            && selection.curr->next == NULL; 
    }  
    return false;
}


bool dbAnyCursor::isFirst() const
{ 
    if (allRecords) { 
        if (currId != 0) { 
            return db->getRow(currId)->prev == 0;
        }
    } else if (selection.curr != NULL) { 
        return selection.pos == 0
            && selection.curr->prev == NULL; 
    }  
    return false;
}

bool dbAnyCursor::gotoPrev()
{
    removed = false;
    if (allRecords) { 
        if (currId != 0) { 
            oid_t prev = db->getRow(currId)->prev;
            if (prev != 0) {
                currId = prev;
                return true;
            }
        }
    } else if (selection.curr != NULL) { 
        if (selection.pos == 0) { 
            if (selection.curr->prev == NULL) { 
                return false;
            }
            selection.curr = selection.curr->prev;
            selection.pos = selection.curr->nRows;
        } 
        currId = selection.curr->rows[--selection.pos];
        return true;
    } 
    return false;
}


bool dbAnyCursor::gotoFirst()
{
    removed = false;
    if (allRecords) { 
        currId = firstId;
        return (currId != 0);
    } else { 
        selection.curr = selection.first;
        selection.pos = 0;
        if (selection.curr == NULL) { 
            return (currId != 0);
        } else { 
            currId = selection.curr->rows[0];
            return true;
        }
    }
}

bool dbAnyCursor::gotoLast()
{
    removed = false;
    if (allRecords) { 
        currId = lastId;
        return (currId != 0);
    } else { 
        selection.curr = selection.last;
        if (selection.curr == NULL) { 
            return (currId != 0);
        } else { 
            selection.pos = selection.curr->nRows-1;
            currId = selection.curr->rows[selection.pos];
            return true;
        }
    }
}

#define BUILD_BITMAP_THRESHOLD 100

bool dbAnyCursor::isInSelection(oid_t oid) 
{
    dbSelection::segment* curr;
    if (eliminateDuplicates) { 
        return isMarked(oid);
    } else if (selection.nRows > BUILD_BITMAP_THRESHOLD) {
        checkForDuplicates();
        for (curr = selection.first; curr != NULL; curr = curr->next) { 
            for (int i = 0, n = curr->nRows; i < n; i++) { 
                oid_t o = curr->rows[i];
                bitmap[o >> 5] |= 1 << (o & 31);
            }
        }
        return isMarked(oid);
    } else { 
        for (curr = selection.first; curr != NULL; curr = curr->next) { 
            for (int i = 0, n = curr->nRows; i < n; i++) { 
                if (curr->rows[i] == oid) { 
                    return true;
                }
            }
        }
        return false;
    }
}

void dbAnyCursor::reset()
{
    if (db == NULL) {
        db = table->db;
        assert(((void)"cursor associated with online database table",
                table->tableId != 0));
    } else if (table->db != db) {
        table = db->lookupTable(table);
    }
    unlink();
    selection.reset();
    eliminateDuplicates = false;
    allRecords = false;
    currId = 0;
    removed = false;
}

void dbAnyCursor::freeze()
{
    unlink();    
}

void dbAnyCursor::unfreeze()
{
    db->beginTransaction(type == dbCursorForUpdate
                         ? dbDatabase::dbExclusiveLock : dbDatabase::dbSharedLock);
    db->threadContext.get()->cursors.link(this);
    if (currId != 0 && prefetch) {
        fetch();
    }
}

dbAnyCursor::~dbAnyCursor() 
{ 
    selection.reset();
    delete[] bitmap;
}




void dbParallelQueryContext::search(int i)
{
    int nThreads = db->parThreads;
    oid_t oid = firstRow;
    int j;
    for (j = i; --j >= 0;) { 
        oid = db->getRow(oid)->next;
    }
    while (oid != 0) { 
        if (db->evaluate(query->tree, oid, table, cursor)) { 
            selection[i].add(oid);
        }
        oid = db->getRow(oid)->next;
        for (j = nThreads; --j > 0 && oid != 0;) { 
            oid = db->getRow(oid)->next;
        }
    }
    if (query->order != NULL) { 
        selection[i].sort(db, query->order);
    }
}


char* strupper(char* s)
{
    byte* p = (byte*)s;
    while (*p != '\0') { 
        *p = toupper(*p);
        p += 1;
    }
    return s;
}


char* strlower(char* s)
{
    byte* p = (byte*)s;
    while (*p != '\0') { 
        *p = (byte)tolower(*p);
        p += 1;
    }
    return s;
}

END_FASTDB_NAMESPACE





