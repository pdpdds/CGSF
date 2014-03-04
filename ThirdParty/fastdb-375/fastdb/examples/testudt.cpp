//-< TESTUDT.CPP >---------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     26-Nov-2009  K.A. Knizhnik  * / [] \ *
//                          Last update: 26-Nov-2009  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Test user defined types
//-------------------------------------------------------------------*--------*

#include <stdio.h>
#include "fastdb.h"

USE_FASTDB_NAMESPACE

const int nRecords = 100000;
const int nSequentialSearches = 10;

struct Point {
    int x;
    int y;
    int z;
};

int pointComparator(void const* a, void const* b, size_t) { 
    Point* p1 = (Point*)a;
    Point* p2 = (Point*)b;
    return p1->x < p2->x ? -1 : p1->x > p2->x ? 1 : p1->y < p2->y ? -1 : p1->y > p2->y ? 1 : 0;
}

unsigned pointHashCode(void const* a, size_t) { 
    Point* p = (Point*)a;
    return p->x ^ p->y;
}

int stringIgnoreSpaceComparator(void const* a, void const* b, size_t) { 
    char* s1 = (char*)a;
    char* s2 = (char*)b;
    char ch1 = *s1++;
    char ch2 = *s2++;
    while (true) { 
        if (ch1 == '\0') { 
            return ch2 == '\0' ? 0 : -1;
        } else if (ch2 == '\0') { 
            return 1;
        } else if (ch1 == ' ') { 
            ch1 = *s1++;
        } else if (ch2 == ' ') { 
            ch2 = *s2++;
        } else if (ch1 != ch2) { 
            return (unsigned char)ch1 - (unsigned char)ch2;
        } else { 
            ch1 = *s1++;
            ch2 = *s2++;
        }            
    }
}

unsigned stringIgnoreSpaceHashCode(void const* a, size_t) { 
    char* s = (char*)a;
    unsigned h = 0;
    char ch;
    while ((ch = *s++) != '\0') { 
        if (ch != ' ') {
            h = h*31 + (unsigned char)ch;
        }
    }
    return h;
}
        
class Record { 
  public:
    char const* key; 
    Point point;

    TYPE_DESCRIPTOR((KEY(key, HASHED|INDEXED).comparator(stringIgnoreSpaceComparator).hashcode(stringIgnoreSpaceHashCode), 
                     UDT(point, HASHED|INDEXED, pointComparator).hashcode(pointHashCode)));
};

REGISTER(Record);


int main(int argc, char* argv[]) 
{
    int i, n;
    Point point;
    char buf[256];

    dbDatabase db;
    if (db.open(_T("testudt"))) { 
        nat8 key = 1999;
        time_t start = time(NULL);
        for (i = 0; i < nRecords; i++) { 
            Record rec;
            key = (3141592621u*key + 2718281829u) % 1000000007u;
            sprintf(buf, "%08x %08x", (nat4)key, (nat4)(key >> 32));
            rec.key = buf;
            rec.point.x = (nat4)key;
            rec.point.y = (nat4)(key >> 32);
            rec.point.z = 1;
            insert(rec);
        }
        printf("Elapsed time for inserting %d record: %d seconds\n", 
               nRecords, int(time(NULL) - start));
        start = time(NULL);
        db.commit();
        printf("Commit time: %d\n", int(time(NULL) - start));

        dbQuery q1;
        dbQuery q2;
        dbCursor<Record> cursor;

        q1 = "key=",buf;
        q2 = "point=",&point;
        start = time(NULL);
        key = 1999;
        for (i = 0; i < nRecords; i++) { 
            key = (3141592621u*key + 2718281829u) % 1000000007u;
            sprintf(buf, "%08x%08x", (nat4)key, (nat4)(key >> 32));
            n = cursor.select(q1);
            assert(n == 1);
            point.x = (nat4)key;
            point.y = (nat4)(key >> 32);
            point.z = 0;
            n = cursor.select(q2);
            assert(n == 1);
        }
        printf("Elapsed time for %d hash searches: %d seconds\n", 
               nRecords*2, int(time(NULL) - start));

        q1 = "key between",buf,"and",buf;
        q2 = "point between",&point,"and",&point;
        start = time(NULL);
        key = 1999;
        for (i = 0; i < nRecords; i++) { 
            key = (3141592621u*key + 2718281829u) % 1000000007u;
            sprintf(buf, "%08x%08x", (nat4)key, (nat4)(key >> 32));
            n = cursor.select(q1);
            assert(n == 1);
            point.x = (nat4)key;
            point.y = (nat4)(key >> 32);
            point.z = 0;
            n = cursor.select(q2);
            assert(n == 1);
        }
        printf("Elapsed time for %d index searches: %d seconds\n", 
               nRecords*2, int(time(NULL) - start));

        q1 = "order by key";
        start = time(NULL);
        n = cursor.select(q1);
        assert(n == nRecords);
        char const* currKey = cursor->key;
        for (i = nRecords; --i != 0;) { 
            char const* nextKey = cursor.next()->key; 
            assert(stringIgnoreSpaceComparator(currKey, nextKey, 0) < 0);
            currKey = nextKey;
        }
        printf("Elapsed time for traversing %d records using string index: %d seconds\n",
               nRecords, int(time(NULL)-start));

        q2 = "order by point";
        start = time(NULL);
        n = cursor.select(q2);
        assert(n == nRecords);
        Point currPoint = cursor->point;
        for (i = nRecords; --i != 0;) { 
            Point nextPoint = cursor.next()->point; 
            assert(pointComparator(&currPoint, &nextPoint, 0) < 0);
            currPoint = nextPoint;
        }
        printf("Elapsed time for traversing %d records using point index: %d seconds\n",
               nRecords, int(time(NULL)-start));


        start = time(NULL);
        dbCursor<Record> updateCursor(dbCursorForUpdate);
        updateCursor.removeAll();
        printf("Elapsed time for deleteing %d records: %d seconds\n",
               nRecords, int(time(NULL)-start));

        db.close();
        return EXIT_SUCCESS;
    } else { 
        printf("Failed to open database\n");
        return EXIT_FAILURE;
    }
}




