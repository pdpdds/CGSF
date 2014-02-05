//-< TESTRAW.CPP >---------------------------------------------------*--------*
// GigaBASE                  Version 1.0         (c) 1999  GARRET    *     ?  *
// (Post Relational Database Management System)                      *   /\|  *
//                                                                   *  /  \  *
//                          Created:     08-Sep-2001  K.A. Knizhnik  * / [] \ *
//                          Last update: 08-Jan-2001  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Test of working raw binary data
//-------------------------------------------------------------------*--------*

#include "fastdb.h"
#include <stdio.h>

USE_FASTDB_NAMESPACE

const int nRecords = 1000000;
const int nSequentialSearches = 10;

#if !defined(GUID_DEFINED) && !defined(_GUID_DEFINED)
typedef struct _GUID
{
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;
#endif

class Record {
  public:
    GUID guid;
    db_int8 value;

    TYPE_DESCRIPTOR((RAWKEY(guid, INDEXED|HASHED), FIELD(value)));
};

REGISTER(Record);


int main(int argc, char* argv[])
{
    int i, n;

    dbDatabase db(dbDatabase::dbAllAccess);
    if (db.open(_T("testraw"))) {
        dbQuery q;
        dbCursor<Record> cursor;
        GUID guid;
        memset(&guid, 0, sizeof guid);
        q = "guid=",&guid;
        nat8 key = 1999;
        time_t start = time(NULL);
        Record rec;
        memset(&rec, 0, sizeof rec);
        for (i = 0; i < nRecords; i++) {
            key = (3141592621u*key + 2718281829u) % 1000000007u;
            *(nat8*)&rec.guid = key;
            rec.value = key;
            insert(rec);
        }
        printf("Elapsed time for inserting %d record: %d seconds\n",
               nRecords, int(time(NULL) - start));
        start = time(NULL);
        db.commit();
        printf("Commit time: %d\n", int(time(NULL) - start));

        start = time(NULL);
        key = 1999;
        for (i = 0; i < nRecords; i++) {            
            key = (3141592621u*key + 2718281829u) % 1000000007u;
            *(nat8*)&guid = key;
            n = cursor.select(q);
            assert(n == 1 && cursor->value == (db_int8)key);
        }
        printf("Elapsed time for %d index searches: %d seconds\n",
               nRecords, int(time(NULL) - start));
        q = "value != 0 and guid >=",&guid;
        memset(&guid, 0, sizeof(guid));
        start = time(NULL);
        for (i = 0; i < nSequentialSearches; i++) {
            n = cursor.select(q);
            assert(n == nRecords);
        }
        printf("Elapsed time for %d sequential search through %d records: "
               "%d seconds\n", nSequentialSearches, nRecords,
               int(time(NULL) - start));

        q = "value != 0 and guid>=",&guid,"order by guid";
        start = time(NULL);
        n = cursor.select(q, dbCursorForUpdate);
        assert(n == nRecords);
        printf("Elapsed time for search with sorting %d records: %d seconds\n",
               nRecords, int(time(NULL)-start));
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




