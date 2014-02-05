//-< TESTIREF.CPP >--------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     12-Jan-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 14-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Test of reference integrity. temporary strings allocation and
// index insert/remove operations
//-------------------------------------------------------------------*--------*

#include "fastdb.h"
#include <stdio.h>

USE_FASTDB_NAMESPACE

const int nRecords = 16*1024;
const int nInsertedRecords = 20000;

class Record { 
  public:
    nat8 key;
    nat8 ikey;
    char const* value;
    dbReference<Record>            ref;
    dbArray< dbReference<Record> > iref;
    dbArray< dbReference<Record> > refs;
    dbArray< dbReference<Record> > irefs;

    TYPE_DESCRIPTOR((KEY(key, HASHED|INDEXED), 
                     FIELD(ikey), 
                     FIELD(value), 
                     RELATION(ref, iref),
                     RELATION(refs, irefs),
                     RELATION(iref, ref),
                     RELATION(irefs, refs)));
};

REGISTER(Record);


int main() 
{
    dbDatabase db;
    dbReference<Record>* refs = new dbReference<Record>[nRecords];
    if (db.open(_T("testiref"))) { 
        db.setConcurrency(0);
        db_int8 inskey = 1999;
        db_int8 remkey = 1999;
        dbReference<Record> ref;
        dbQuery q;
        dbQuery q2;
        Record rec;
        char buf[256];
        dbCursor<Record> cursor(dbCursorForUpdate);
        q="key=",remkey,
          "and (ref is null or ref.key=ikey and current in ref.iref)"
          "and not exists i:(current not in refs[i].irefs)";
        q2="ref=",ref,"or",ref,"in refs or",ref,"in iref or",ref,"in irefs or"
           " lower(value) <> string(key)+'.'";

        time_t start = time(NULL);
        int i, n = 1, p = 0, j, k, l, m = 0;
        sprintf(buf, INT8_FORMAT ".", inskey);
        rec.ikey = rec.key = inskey;    
        rec.value = buf;
        ref = insert(rec);
        cursor.at(ref);
        cursor->ref = ref;
        cursor->iref.resize(1);
        cursor->iref.putat(0, ref);
        refs[p++] = ref;
        cursor.update();
        for (i = 0; i < nInsertedRecords; i++) { 
            if (n == nRecords) {
                int s = cursor.select(q);
                assert(s == 1);
                assert(refs[p] == cursor.currentId());
                ref = cursor.currentId();
                cursor.remove();
                remkey = (3141592621u*nat8(remkey) + 2718281829u) % 1000000007u;
                s = cursor.select(q2);
                assert(s == 0);
                refs[p] = null;
            }
            inskey = (3141592621u*nat8(inskey) + 2718281829u) % 1000000007u;
            rec.key = inskey;
            sprintf(buf, INT8_FORMAT ".", inskey);
            ref = refs[nat8(inskey) % n];
            if (ref != null) { 
                rec.ikey = cursor.at(ref)->key;
                rec.ref = ref;
            }
            j = (int(inskey) >> 16) & 0xF;
            k = 0;
            rec.refs.resize(0);
            while (--j >= 0) { 
                ref = refs[nat8(inskey >> j) % n];
                if (ref != null) { 
                    for (l = 0; l < k && rec.refs[l] != ref; l++);
                    if (l == k) { 
                        rec.refs.resize(++k);
                        rec.refs.putat(l, ref);
                    }
                }
            }
            refs[p++] = insert(rec);
            p &= nRecords - 1;
            if (n < nRecords) {
                n += 1;
            }
            if (i >= m) { 
                printf("Proceed %d records...\r", i);
                fflush(stdout);
                m = i + 100;
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




