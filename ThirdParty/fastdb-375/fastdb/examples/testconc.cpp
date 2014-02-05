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
#include "fastdb.h"

USE_FASTDB_NAMESPACE

class Bank { 
  public:
    db_int8 total;
    bool    online;

    TYPE_DESCRIPTOR((FIELD(total), FIELD(online)));
};

REGISTER(Bank);

class Account {
  public:
    int4    id;
    db_int8 amount;
    
    TYPE_DESCRIPTOR((KEY(id, HASHED), FIELD(amount)));
};

REGISTER(Account);

dbLocalEvent timer;

const int nUpdateIterations = 10000;
const int nMaxAccounts = 1000;

void thread_proc inspector(void* arg)
{
    dbDatabase* db = (dbDatabase*)arg;
    db->attach();
    dbCursor<Account> accounts;
    dbCursor<Bank> bank;
    dbMutex mutex;
    int n = 0;
    bool online = true;
    mutex.lock();
    do {
        if (accounts.select() > 0) { 
            db_int8 total = 0;
            do { 
                total += accounts->amount;
            } while (accounts.next());
            bank.select();    
            assert(total == bank->total);
            online = bank->online;
        }
        db->commit();
        n += 1;
        timer.wait(mutex, 1);
    } while (online);
    mutex.unlock();
    printf("Thread performs %d inspection iterations\n", n);
}

int main(int argc, char* argv[])
{
    int i;
    int nThreads = 4;
#ifdef REPLICATION_SUPPORT
    char* servers[3] = {"localhost:6101", "localhost:6102", "localhost:6103"};       
    char dbName[16];
    if (argc < 3) { 
        fprintf(stderr, "Usage: testconc (update|inspect|coinspect) <REPLICATION-NODE-ID> [number-of-inspection-threads]\n");
        return 1;
    }
    int replNodeId = atoi(argv[2]);    
    sprintf(dbName, "testconc%d", replNodeId);
    if (argc == 4) {
        nThreads = atoi(argv[3]);
    }
    if (strcmp(argv[1], "coinspect") == 0) { 
        char fileName[32];
        sprintf(fileName, "%s.fdb", dbName);

        dbReplicatedDatabase db(dbDatabase::dbConcurrentRead);
        if (db.open(dbName, fileName, replNodeId, servers, itemsof(servers))) { 
            printf("Start %d inspection threads\n", nThreads);
            dbThread* thread = new dbThread[nThreads];
            timer.open(false);
            while (true) { 
                for (i = 0; i < nThreads; i++) { 
                    thread[i].create(inspector, &db);
                }        
                for (i = 0; i < nThreads; i++) { 
                    thread[i].join();
                }
                dbThread::sleep(10);                
            }
            timer.close();
            db.close();
            return EXIT_SUCCESS;
        }
    } else 
#else 
    char* dbName = "testconc";
    if (argc < 2) { 
        fprintf(stderr, "Usage: testconc (update|inspect) [number-of-inspection-threads]\n");
        return 1;
    }
    if (argc == 3) {
        nThreads = atoi(argv[2]);
    }
#endif
    if (strcmp(argv[1], "inspect") == 0) { 
        dbDatabase db(dbDatabase::dbConcurrentRead);
        if (db.open(WC_STRING(dbName))) { 
            printf("Start %d inspection threads\n", nThreads);
            dbThread* thread = new dbThread[nThreads];
            timer.open(false);
            for (i = 0; i < nThreads; i++) { 
                thread[i].create(inspector, &db);
            }        
            for (i = 0; i < nThreads; i++) { 
                thread[i].join();
            }
            timer.close();
            db.close();
            return EXIT_SUCCESS;
        }
    } else {
#ifdef REPLICATION_SUPPORT
        char fileName[32];
        sprintf(fileName, "%s.fdb", dbName);

        dbReplicatedDatabase db(dbDatabase::dbConcurrentUpdate);
        if (db.open(WC_STRING(dbName), WC_STRING(fileName), replNodeId, servers, itemsof(servers))) { 
            printf("Update process started, now you should start inspectors: \"testconc inspector 1 N\", where N is number of inspectpors\n");            
#else 
        dbDatabase db(dbDatabase::dbConcurrentUpdate);
        if (db.open(WC_STRING(dbName))) { 
            printf("Update process started, now you should start inspectors: \"testconc inspector N\", where N is number of inspectpors\n");            
#endif
            dbThread::sleep(2);
            dbCursor<Bank> bank(dbCursorForUpdate);
            dbCursor<Account> accounts(dbCursorForUpdate);
            dbQuery q;
            int id;
            q = "id=",id;

            // Initialization
            if (bank.select() == 0) { 
                Bank theBank;
                theBank.total = 0;
                theBank.online = true;
                insert(theBank);
            } else { 
                bank->online = true;
                bank.update();
            }
            db.commit();

            for (i = 0; i < nUpdateIterations; i++) { 
                int nAccounts = accounts.select();
                id = 0;
                db_int8 delta = i;
                if (accounts.last()) { 
                    id = accounts->id + 1;
                    accounts->amount += i;
                    accounts.update();
                    delta += i;
                }
                Account acc;
                acc.amount = i;
                acc.id = id;
                insert(acc);
                if (nAccounts > nMaxAccounts) {
                    accounts.first();
                    id = accounts->id;
                    int n = accounts.select(q);
                    assert(n == 1);
                    delta -= accounts->amount;
                    accounts.remove();
                }
                bank.select();
                bank->total += delta;
                bank.update();
                db.commit();
            }
            printf("End of %d update iterations\n", nUpdateIterations);
            bank.select();
            bank->online = false;
            bank.update();
            db.close();
            return EXIT_SUCCESS;
        }
    } 
    printf("Failed to open database\n");
    return EXIT_FAILURE;
}




