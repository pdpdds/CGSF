//-< TESTPAR.CPP >---------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     10-Dec-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 20-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Test parallel sequential search
//-------------------------------------------------------------------*--------*

#include "fastdb.h"
#include <stdio.h>

USE_FASTDB_NAMESPACE

const int nRecords = 10000;

struct Record {
    int4 id;

    TYPE_DESCRIPTOR((FIELD(id)));
};

REGISTER(Record);


int main(int argc, char* argv[])
{
    int i, j;
    dbDatabase db(dbDatabase::dbAllAccess, 16*1024); // 96Mb page pool

    if (db.open(_T("testpar"))) {
        db.setConcurrency(4);
        for (i = 0; i < nRecords; i++) { 
            Record rec;
            rec.id = i;
            insert(rec);
        }
        db.commit();
        
        dbQuery q;
        dbCursor<Record> cursor;
        int min;
        int max;
        q = "id between",min,"and",max;
        time_t start = time(NULL);

        for (i = 0, j = 0; i+j < nRecords; i += j) { 
            min = i;
            max = i + j;
            j += 1;
            int n = cursor.select(q);
            assert(n == j);
            int k = 0;
            int sum = 0;
            do { 
                sum += cursor->id;
                k += 1;
            } while (cursor.next());
            assert(k == j);
            assert(sum == k*(k-1)/2 + k*i);
        }
        printf("Elapsed time for %d sequential searchs in %d records: %d seconds\n",
               j, nRecords, int(time(NULL) - start)); 

        q = "id between",min,"and",max,"order by id";
        start = time(NULL);
        for (i = 0, j = 0; i+j < nRecords; i += j) { 
            min = i;
            max = i + j;
            j += 1;
            int n = cursor.select(q);
            assert(n == j);
            int k = i;
            do { 
                assert(cursor->id == k);
                k += 1;
            } while (cursor.next());
            assert(k == i+j);
        }
        printf("Elapsed time for %d sequential searchs with sorting in %d records: %d seconds\n",
               j, nRecords, int(time(NULL) - start));
        db.close();
    } 
    return 0;
}
