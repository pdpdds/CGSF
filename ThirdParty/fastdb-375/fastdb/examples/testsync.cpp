//-< TESTSYNC.CPP >--------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Dec-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 12-Jan-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Test for interprocess and interthread synchronization 
//-------------------------------------------------------------------*--------*

#include "fastdb.h"
#include <stdio.h>

USE_FASTDB_NAMESPACE

time_t commitDelay;

const int nRecords = 100000;
const int nSequentialSearches = 10;

class Record { 
  public:
    char const* key;

    TYPE_DESCRIPTOR((KEY(key, HASHED|INDEXED)));
};

REGISTER(Record);

dbDatabase db;

void thread_proc doTest(void* arg)
{
    int  i, n;
    int  id = (long)arg;
    char buf[32];

    db.attach();

    nat8 key = 1999;
    time_t start = time(NULL);
    for (i = 0; i < nRecords; i++) { 
        Record rec;
        key = (3141592621u*key + 2718281829u) % 1000000007u;
        sprintf(buf, INT8_FORMAT ".%d", key, id);
        rec.key = buf;
        insert(rec);
        if (commitDelay != 0) { 
            db.commit();
        } else { 
            db.precommit();
        }
    }
    db.commit();
    printf("Elapsed time for inserting %d record: %d seconds\n", 
           nRecords, int(time(NULL) - start));
    dbQuery q;
    dbCursor<Record> cursor;
    dbCursor<Record> updateCursor(dbCursorForUpdate);
    q = "key=",buf;
    start = time(NULL);
    key = 1999;
    for (i = 0; i < nRecords; i++) { 
        key = (3141592621u*key + 2718281829u) % 1000000007u;
        sprintf(buf, INT8_FORMAT ".%d", key, id);
        n = cursor.select(q);
        assert(n == 1);
        db.commit();
    }
    printf("Elapsed time for %d hash searches: %d seconds\n", 
           nRecords, int(time(NULL) - start));
    q = "key >=",buf,"and key <=",buf;
    start = time(NULL);
    key = 1999;
    cursor.setSelectionLimit(1);
    for (i = 0; i < nRecords; i++) { 
        key = (3141592621u*key + 2718281829u) % 1000000007u;
        sprintf(buf, INT8_FORMAT ".%d", key, id);
        n = cursor.select(q);
        assert(n == 1);
        db.commit();
    }
    printf("Elapsed time for %d index searches: %d seconds\n", 
           nRecords, int(time(NULL) - start));
    q = "key like",buf;
    sprintf(buf, "%%.%d", id);
    cursor.unsetSelectionLimit();
    start = time(NULL);
    for (i = 0; i < nSequentialSearches; i++) { 
        n = cursor.select(q);
        assert(n == nRecords);
        db.commit();
    }
    printf("Elapsed time for %d sequential search through %d records: "
           "%d seconds\n", nSequentialSearches, nRecords, 
           int(time(NULL) - start));
    
    q = "key like",buf,"order by key";
    start = time(NULL);
    for (i = 0; i < nSequentialSearches; i++) { 
        n = cursor.select(q);
        assert(n == nRecords);
        char const* curr = cursor->key;
        for (n = nRecords; --n != 0;) { 
            char const* succ = cursor.next()->key; 
#ifdef USE_LOCALE_SETTINGS
            assert(strcoll(succ, curr) > 0); 
#else
            assert(strcmp(succ, curr) > 0); 
#endif
            curr = succ;
        }
        db.commit();
    } 
    printf("Elapsed time for %d searchs with sorting %d records: %d seconds\n",
           nSequentialSearches, nRecords, int(time(NULL)-start));
    start = time(NULL);
    q = "key like",buf;
    sprintf(buf, "1%%.%d", id);
    n = updateCursor.select(q);
    for (i = 0; i < n; i++) { 
        updateCursor.remove();
    }
    assert(updateCursor.get() == NULL);
    db.commit();
    sprintf(buf, "%%.%d", id);
    n += updateCursor.select(q);
    assert(n == nRecords);
    updateCursor.removeAllSelected();
    db.commit();
    printf("Elapsed time for deleting all %d records: %d seconds\n", 
           nRecords, int(time(NULL) - start));
    db.detach();
}

int main(int argc, char* argv[]) 
{
    int i, id = 0;
    int nThreads = 1;

    if (argc > 1) { 
        nThreads = atoi(argv[1]);
    }
    if (argc > 2) { 
        id = atoi(argv[2]);
    }
    char* env = getenv("FASTDB_COMMIT_DELAY");
    if (env != NULL) { 
        commitDelay = atoi(env);
    }
    dbThread* thread = new dbThread[nThreads];
    //    if (db.open("testsync", NULL, 2000)) { // deadlock detection timeout
    if (db.open(_T("testsync"), NULL, INFINITE, commitDelay)) { 
        db.setConcurrency(0);
        for (i = 0; i < nThreads; i++) { 
            thread[i].create(doTest, (void*)(long)id++);
        }
        for (i = 0; i < nThreads; i++) { 
            thread[i].join();
        }
        db.close();
        return EXIT_SUCCESS;
    } else { 
        printf("Failed to open database\n");
        return EXIT_FAILURE;
    }
}











