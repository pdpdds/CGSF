//-< TESTPERF.CPP >--------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     10-Feb-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Feb-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Test for index insert/search/delete operations
//-------------------------------------------------------------------*--------*

#include <stdio.h>
#include "fastdb.h"

USE_FASTDB_NAMESPACE

const int nInsertedRecords = 10000000;
const int nRecords = 100000;
const int maxDuplicates = 256;

class Record { 
  public:
    nat8 key;

    TYPE_DESCRIPTOR((KEY(key, INDEXED) ));
};

REGISTER(Record);


int main() 
{
    dbDatabase db(dbDatabase::dbAllAccess, 4096); // 32Mb page pool
    db.setFixedSizeAllocator(16, 32, 16, 10000);
    if (db.open(_T("testidx"))) { 
        nat8 insKey = 1999;
        nat8 delKey = 1999;
        db_int8 key;
        dbQuery q;
        Record  rec;
        dbCursor<Record> cursor(dbCursorForUpdate);
        q = "key=",key;
        time_t start = time(NULL);
        for (int i = 0, j = 0, n = 0; i < nInsertedRecords;) { 
            if (n >= nRecords) { 
                delKey = (3141592621u*delKey + 2718281829u) % 1000000007u;
                key = delKey;
                unsigned r = cursor.select(q);
                assert(r == ((unsigned)delKey % maxDuplicates) + 1);
                n -= r;
                cursor.removeAllSelected();
            }           
            insKey = (3141592621u*nat8(insKey) + 2718281829u) % 1000000007;
            unsigned r = ((unsigned)insKey % maxDuplicates) + 1;
            rec.key = insKey;
            n += r;
            i += r;
            do { 
                insert(rec);
            } while (--r != 0);
            if (i > j) { 
                printf("Insert %d objects...\r", i);
                fflush(stdout);
                j = i + 1000;
            }
        }
        printf("Elapsed time for %d record: %d seconds\n", 
               nInsertedRecords, int(time(NULL) - start));
        db.close();
        return EXIT_SUCCESS;
    } else { 
        printf("Failed to open database\n");
        return EXIT_FAILURE;
    }
}




