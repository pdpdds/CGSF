//-< TESTDB.CPP >----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:      1-Jan-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 18-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Test of FastDB data definition and manipulation facilities. 
// First create database by running this application, then 
// fill database by "subsql testddl.sql" and then run this applciation once
// again to check correctness of inserted data.
//-------------------------------------------------------------------*--------*

#include "fastdb.h"
#include <stdio.h>
#include <math.h>

USE_FASTDB_NAMESPACE

class Subrecord { 
  public:
    char* str;
    db_int8  i;

    void dump() const { 
        printf("('%s' " INT8_FORMAT ") ", str, i);
    }

    TYPE_DESCRIPTOR((FIELD(str), 
                     FIELD(i)));
};

class Base { 
  public:
    dbArray<Subrecord> x;
    dbArray<char*>     y;
    dbArray<int2>      z;
    dbArray< dbArray<int4> > w;

    void dump() const { 
        size_t i, j;
        printf("(");
        for (i = 0; i < x.length(); i++) { 
            x[i].dump();
        }
        printf(") (");
        for (i = 0; i < y.length(); i++) { 
            printf("'%s' ", y[i]);
        }
        printf(") (");
        for (i = 0; i < z.length(); i++) { 
            printf("%d ", z[i]);
        }
        printf(") (");
        for (i = 0; i < w.length(); i++) { 
            printf("(");
            for (j = 0; j < w[i].length(); j++) { 
                printf("%d ", w[i][j]);
            }
            printf(") ");
        }
        printf(")");
    }
    TYPE_DESCRIPTOR((FIELD(x), 
                     FIELD(y),
                     FIELD(z),
                     FIELD(w)));
};

class Record : public Base { 
  public:
    int4  a;
    int1  b;
    real8 c;
    bool  d;
    int2  e;
    real4 f;
    dbReference<Record> g;
    Subrecord h;
    struct obscure { 
        int1 data[16];
    } o;
    
    void dump() const {
        Base::dump();
        printf(" %d %d %f %s %d %f (%02x%02x%02x%02x)", 
               a, b, c, d ? "true" : "false", e, f, 
               o.data[0], o.data[1], o.data[2], o.data[3]);
        h.dump();
        printf("\n");
    }
    TYPE_DESCRIPTOR((SUPERCLASS(Base),
                     FIELD(a), 
                     FIELD(b),
                     FIELD(c),
                     FIELD(d),
                     FIELD(e),
                     FIELD(f),
                     FIELD(g),
                     FIELD(h),
                     RAWFIELD(o)));
};

REGISTER(Record);

static double mysin(double x) { return sin(x); }
static double mycos(double x) { return cos(x); }

USER_FUNC(mysin);
USER_FUNC(mycos);

int main() 
{
    dbDatabase db;
    
    if (db.open(_T("testddl"))) { 
        printf("Database intialized\n");
        dbCursor<Record> cursor;
        if (cursor.select("abs(sin(c)^2 + cos(c)^2 - 1) < 1e-5") > 0) { 
            do { 
                cursor->dump();
            } while (cursor.next());
        } else { 
            printf("No records in table\n");
        }
        db.close();
    } else { 
        printf("Failed to open database\n");
    }
    return 0;
}
