//-< TESTSPAT.CPP >--------------------------------------------------*--------*
// Fastdb                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Post Relational Database Management System)                      *   /\|  *
//                                                                   *  /  \  *
//                          Created:     23-Nov-2001  K.A. Knizhnik  * / [] \ *
//                          Last update: 23-Nov-2001  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Test program for R-tree class 
//-------------------------------------------------------------------*--------*

#include "fastdb.h"
#include <stdio.h>
#include <time.h>

USE_FASTDB_NAMESPACE

class SpatialRecord { 
  public:
    rectangle   rect;
    int         id;

    TYPE_DESCRIPTOR((KEY(rect, INDEXED), 
                     KEY(id, INDEXED)));
};

        
REGISTER(SpatialRecord);

class TestSpatialRecord : public SpatialRecord { 
  public:
    TestSpatialRecord* nextFree;
};

inline int random(unsigned mod) { return rand() % mod; }

const int N_ITERATIONS  = 100000;
const int MAX_RECORDS = 100000;
const int MAX_COORD = 32*1024;
const int MAX_LEN = 1024;

int main(int argc, char* argv[]) 
{ 
    dbDatabase db;
    if (!db.open(_T("testspat"))) { 
        fprintf(stderr, "Faield to open database\n");
        return EXIT_FAILURE;
    }
    TestSpatialRecord* recordArray = new TestSpatialRecord[MAX_RECORDS];
    TestSpatialRecord* freeChain;
    int i, n;
    int n_inserts = 0, n_searches = 0, n_removes = 0;
    TestSpatialRecord* rp;
    rectangle     r;
    int           id;
    dbQuery       q[8];

    q[0] = "rect <=",r;
    q[1] = "rect <",r;
    q[2] = "rect >",r;
    q[3] = "rect >=",r;
    q[4] = "rect =",r;
    q[5] = r,"in rect";
    q[6] = "rect overlaps",r;
    q[7] = "id=",id;

    static rectangle::comparator comparators[] = {
        &rectangle::operator <=, 
        &rectangle::operator <, 
        &rectangle::operator >, 
        &rectangle::operator >=, 
        &rectangle::operator ==, 
        &rectangle::operator >=, 
        &rectangle::operator &
    };

    srand(2007);//time(NULL));
    for (i = 0; i < MAX_RECORDS; i++) { 
        recordArray[i].id = 0;
        recordArray[i].nextFree = &recordArray[i+1];
    }
    recordArray[MAX_RECORDS-1].nextFree = NULL;
    freeChain = recordArray;

    dbCursor<SpatialRecord> cursor(dbCursorForUpdate);
    dbCursor<SpatialRecord> cursor2;
    cursor.removeAll();
    time_t startTime = time(NULL);
    rectangle::comparator cmp;

    for (int n_iterations = 0; n_iterations < N_ITERATIONS; n_iterations++) { 
        int c = random(10);
        switch (c) { 
          case 9: // insert
          case 8:
            if (freeChain != NULL) { 
                rp = freeChain;
                freeChain = freeChain->nextFree;
                for (i = 0; i < rectangle::dim; i++) { 
                    rp->rect.boundary[i] = random(MAX_COORD*2) - MAX_COORD; 
                    rp->rect.boundary[i+rectangle::dim] = rp->rect.boundary[i] + random(MAX_LEN);
                }
                rp->id = (int)(rp - recordArray + 1);
                insert(*rp);
                n_inserts += 1;
            }
            break;
          case 7: // remove
            i = random(MAX_RECORDS);
            rp = &recordArray[i];
            if (rp->id != 0) { 
                id = rp->id;
                n = cursor.select(q[c], dbCursorForUpdate);
                assert(n == 1);
                cursor.remove();                    
                rp->id = 0;
                rp->nextFree = freeChain;
                freeChain = rp;
                n_removes += 1;
            } 
            break;
          default: // search
            for (i = 0; i < rectangle::dim; i++) { 
                r.boundary[i] = random(MAX_COORD*2) - MAX_COORD; 
                r.boundary[i+rectangle::dim] = r.boundary[i] + random(MAX_LEN);
            }
            n = cursor2.select(q[c]);
            rp = recordArray;       
            cmp = comparators[c];
            for (i = 0; i < MAX_RECORDS; i++, rp++) { 
                if (rp->id != 0) { 
                    if ((rp->rect.*cmp)(r)) { 
                        if (cursor2.first()) { 
                            while (cursor2->id != rp->id && cursor2.next());
                        }
                        assert(cursor2->id == rp->id);
                        for (int j = 0; j < rectangle::dim*2; j++) { 
                            assert(cursor2->rect.boundary[j] == rp->rect.boundary[j]);
                        }
                        n -= 1;
                    }
                }
            }
            n_searches += 1;
        }
        if (n_iterations % 100 == 0) { 
            printf("Iteration %d: %d inserts, %d removes, %d searches\r", 
                   n_iterations, n_inserts, n_removes, n_searches);
        }
    }
    printf("\nElapsed time: %ld\n", time(NULL) - startTime);
    delete[] recordArray;
    db.close();
    return EXIT_SUCCESS;
}
