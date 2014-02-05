//-< TESTARRIDX.CPP >------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     24-Nov-2009  K.A. Knizhnik  * / [] \ *
//                          Last update: 26-Nov-2009  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Test array index
//-------------------------------------------------------------------*--------*

#include <stdio.h>
#include "fastdb.h"

USE_FASTDB_NAMESPACE

const int nRecords = 100000;
const int nSequentialSearches = 10;

class Record { 
  public:
    dbArray<int4> key;
    dbArray<int4> value;

    TYPE_DESCRIPTOR((KEY(key, HASHED|INDEXED), FIELD(value)));
};

REGISTER(Record);


int main(int argc, char* argv[]) 
{
    int i, j, n, keylen;
    dbDatabase db;
    if (db.open(_T("testarridx"))) { 
        nat8 key = 1999;
        time_t start = time(NULL);
        for (i = 0; i < nRecords; i++) { 
            Record rec;
            key = (3141592621u*key + 2718281829u) % 1000000007u;
            keylen = (int)(key % 10 + 1);
            rec.key.resize(keylen);
            rec.value.resize(keylen);
            for (j = 0; j < keylen; j++) { 
                key = (3141592621u*key + 2718281829u) % 1000000007u;
                rec.key.putat(j, (int4)key);
                rec.value.putat(j, (int4)key);
            }
            insert(rec);
        }
        printf("Elapsed time for inserting %d record: %d seconds\n", 
               nRecords, int(time(NULL) - start));
        start = time(NULL);
        db.commit();
        printf("Commit time: %d\n", int(time(NULL) - start));

        dbQuery q;
        dbCursor<Record> cursor;

        dbArray<int4> arr;
        q = "key=",arr;
        start = time(NULL);
        key = 1999;
        for (i = 0; i < nRecords; i++) { 
            key = (3141592621u*key + 2718281829u) % 1000000007u;
            keylen = (int)(key % 10 + 1);
            arr.resize(keylen);
            for (j = 0; j < keylen; j++) { 
                key = (3141592621u*key + 2718281829u) % 1000000007u;
                arr.putat(j, (int4)key);
            }
            n = cursor.select(q);
            assert(n == 1);
        }
        printf("Elapsed time for %d hash searches: %d seconds\n", 
               nRecords, int(time(NULL) - start));
        q = "key between",arr,"and",arr;
        start = time(NULL);
        key = 1999;
        for (i = 0; i < nRecords; i++) { 
            key = (3141592621u*key + 2718281829u) % 1000000007u;
            keylen = (int)(key % 10 + 1);
            arr.resize(keylen);
            for (j = 0; j < keylen; j++) { 
                key = (3141592621u*key + 2718281829u) % 1000000007u;
                arr.putat(j, (int4)key);
            }
            n = cursor.select(q);
            assert(n == 1);
        }
        printf("Elapsed time for %d index searches: %d seconds\n", 
               nRecords, int(time(NULL) - start));
        q = "value=key";
        start = time(NULL);
        for (i = 0; i < nSequentialSearches; i++) { 
            n = cursor.select(q);
            assert(n == nRecords);
        }
        printf("Elapsed time for %d sequential search through %d records: "
               "%d seconds\n", nSequentialSearches, nRecords, 
               int(time(NULL) - start));
        
        q = "value=key order by value";
        start = time(NULL);
        n = cursor.select(q, dbCursorForUpdate);
        assert(n == nRecords);
        printf("Elapsed time for search with sorting %d records: %d seconds\n",
               nRecords, int(time(NULL)-start));
        dbArray<int4> curr = cursor->key;
        for (i = nRecords; --i != 0;) { 
            dbArray<int4> succ = cursor.next()->key; 
            assert(succ.compare(curr) > 0);
            curr = succ;
        }
        start = time(NULL);
        cursor.removeAll();
        printf("Elapsed time for deleting all %d records: %d seconds\n", 
               nRecords, int(time(NULL) - start));
        db.close();
        return EXIT_SUCCESS;
    } else { 
        printf("Failed to open database\n");
        return EXIT_FAILURE;
    }
}




