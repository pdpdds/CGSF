//-< TESTCONC.CPP >--------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     31-Nov-2002  K.A. Knizhnik  * / [] \ *
//                          Last update: 31-Nov-2002  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Test for concurrent update model
//-------------------------------------------------------------------*--------*

#include <stdio.h>
#include <assert.h>
#include "fastdb.h"

USE_FASTDB_NAMESPACE

class Record { 
  public:
    db_int8       id;
    dbArray<int1> body;
    TYPE_DESCRIPTOR((KEY(id, HASHED), FIELD(body)));
};

REGISTER(Record);


const int nIterations = 10000;
const int nThreads = 4;
const size_t maxBodySize = 1000;
dbDatabase* db;

void thread_proc producer(void* arg)
{
    nat8 key = (nat8)arg;
    db->attach();
    for (int i = 0; i < nIterations; i++) { 
        Record rec;
        key = (3141592621u*key + 2718281829u) % 1000000007u;
        rec.id = key;
        rec.body.resize((size_t)(key % maxBodySize));
        insert(rec);
        db->commit();
    }
    db->detach();
}

int main(int argc, char* argv[])
{
    int i;
    char* servers[2] = {"localhost:6101", "localhost:6102"};       
    if (argc != 2) { 
        fprintf(stderr, "Usage: testreplic (master|slave)\n");
        return EXIT_FAILURE;
    }
    if (strcmp(argv[1], "master") == 0) { 
        dbReplicatedDatabase master(dbDatabase::dbConcurrentUpdate);
        db = &master;
        if (master.open("master", "master.fdb", 0, servers, itemsof(servers))) { 
            dbThread* thread = new dbThread[nThreads];
            dbThread::sleep(2);
            for (i = 0; i < nThreads; i++) { 
                thread[i].create(producer, (void*)(1999 + i));
            }        
            for (i = 0; i < nThreads; i++) { 
                thread[i].join();
            }
            printf("Master finish\n");
            master.close();
            return EXIT_SUCCESS;
        }
    } else { 
        dbReplicatedDatabase slave(dbDatabase::dbConcurrentRead);
        if (slave.open("slave", "slave.fdb", 1, servers, itemsof(servers))) {             
            int count = 0;
            do { 
                dbCursor<Record> cursor;
                i = cursor.select();
                if (i > 0) { 
                    int n = 0;
                    do { 
                        assert(cursor->id % maxBodySize == cursor->body.length());
                        n += 1;
                    } while (cursor.next());
                    assert(i == n);                    
                }
                slave.commit();
                count += 1;
            } while (i < nThreads*nIterations);
            printf("Slave finish after %d iterations\n", count);
            assert(i == nThreads*nIterations);
            slave.close();
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}
