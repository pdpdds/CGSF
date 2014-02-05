//-< TESTPERF.CPP >--------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     10-Dec-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 20-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Performance test for index and sequential searches
//-------------------------------------------------------------------*--------*

#include "fastdb.h"
#include <stdio.h>

USE_FASTDB_NAMESPACE

const int nRecords = 1000000;
const int nIterations = 10;
const int nReadsPerWrite = 1013;

class Record { 
  public:
    char const* key;
    char const* value;
    int         counter;

    TYPE_DESCRIPTOR((KEY(key, HASHED|INDEXED), FIELD(value), FIELD(counter)));
};

REGISTER(Record);


int main(int argc, char* argv[]) 
{
    int i, j, n;
    char buf[32];

    dbDatabase db;
    if (db.open(_T("testfuzzy"))) { 
        nat8 key = 1999;
        time_t start = time(NULL);
        for (i = 0; i < nRecords; i++) { 
            Record rec;
            key = (3141592621u*key + 2718281829u) % 1000000007u;
            sprintf(buf, INT8_FORMAT ".", key);
            rec.key = buf;
            rec.value = buf;
            insert(rec);
        }
        printf("Elapsed time for inserting %d record: %d seconds\n", 
               nRecords, int(time(NULL) - start));
        start = time(NULL);
        db.commit();
        printf("Commit time: %d\n", int(time(NULL) - start));

        dbQuery q;
        dbCursor<Record> cursor;
        dbCursor<Record> updateCursor(dbCursorForUpdate);

        q = "key=",buf;
        for (j = 0; j < nIterations; j++) { 
            key = 1999;
            for (i = 0; i < nRecords; i++) { 
                key = (3141592621u*key + 2718281829u) % 1000000007u;
                sprintf(buf, INT8_FORMAT ".", key);
                if (i*j % nReadsPerWrite == 0) { 
                    n = updateCursor.select(q);
                    updateCursor->counter += 1;
                    db.commit();
                } else { 
                    n = cursor.select(q);
                }
                assert(n == 1);
            }
        }
        printf("Elapsed time for %d reads and %d writes: %d seconds\n", 
               nIterations*nRecords, nIterations*nRecords/nReadsPerWrite, 
               int(time(NULL) - start));
        start = time(NULL);
        updateCursor.removeAll();
        printf("Elapsed time for deleting all %d records: %d seconds\n", 
               nRecords, int(time(NULL) - start));
        db.close();
        return EXIT_SUCCESS;
    } else { 
        printf("Failed to open database\n");
        return EXIT_FAILURE;
    }
}




