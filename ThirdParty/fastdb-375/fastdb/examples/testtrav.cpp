//-< TESTTRAV.CPP >-------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     15-Jan-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 15-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Test of record traverse mechanism and set operations
//-------------------------------------------------------------------*--------*

#include "fastdb.h"
#include "set.h"
#include <stdio.h>


USE_FASTDB_NAMESPACE

class Detail { 
  public: 
    char const*name;

    enum DetailClass { 
        dcResistor,
        dcTransistor,
        dcCondenser,
        dcDiode,
        dcBoard
    };
        
    dbSet<DetailClass> type;

    dbReference<Detail>             assembly;
    dbArray< dbReference<Detail> >  parts;

    TYPE_DESCRIPTOR((KEY(name, HASHED|INDEXED), 
                     RELATION(assembly, parts),                      
                     RELATION(parts, assembly),
                     FIELD(type)));
    
    void print() const;
};

REGISTER(Detail);


dbSet<Detail::DetailClass> inputDetailClass()
{
    char buf[64], *type = buf;
    printf("Specify detail class(es)\n"
           "\tr - resistor\n"
           "\tt - transistor\n"
           "\tc - condenser\n"
           "\td - diode\n"
           "\tb - board\n"
           "? ");

    scanf("%s", type);
    dbSet<Detail::DetailClass> set;
    while (*type != '\0') { 
        switch (*type++) { 
          case 'r':
            set += Detail::dcResistor;
            break;
          case 't':
            set += Detail::dcTransistor;
            break;
          case 'c':
            set += Detail::dcCondenser;
            break;
          case 'd':
            set += Detail::dcDiode;
            break;
          case 'b':
            set += Detail::dcBoard;
            break;
          default:
            printf("Invalid detail class: %c\n", *(type-1));
            return set;
        }
    }
    return set;
}
        
void Detail::print() const
{ 
    printf("detail %s (", name);
    if (type.has(dcResistor)) { 
        printf("resistor ");
    }
    if (type.has(dcTransistor)) { 
        printf("transistor ");
    }
    if (type.has(dcCondenser)) { 
        printf("condenser ");
    }
    if (type.has(dcDiode)) { 
        printf("diode ");
    }
    if (type.has(dcBoard)) { 
        printf("board ");
    }
    printf(")\n");
}

int main() 
{ 
    dbDatabase db;
    if (db.open(_T("testtrav"))) { 
        dbReference<Detail> assembly;
        dbArray< dbReference<Detail> > parts;
        dbCursor<Detail> cursor;
        Detail detail;
        const int bufLen = 256;
        char name[bufLen];
        char newName[bufLen];
        dbSet<Detail::DetailClass> type;
        dbQuery q1, q2, q3, q4;
        bool notEmpty = cursor.select() > 0;

        q1 = "name=",name;
        q2 = type >= "type","start from",parts,"follow by parts";
        q3 = "start from",parts;
        q4 = "start from",assembly,"follow by assembly";

        detail.name = newName;

        while (true) { 
            printf("\nMenu:\n"
                   "1. Add detail\n"
                   "2. Search for details\n"
                   "3. All details\n"
                   "4. Assembly parts\n"
                   "5. Part of\n"
                   "6. Exit\n>> ");
            
            int choice = 0;
            
            scanf("%d", &choice);
            switch (choice) { 
              case 1:
                if (notEmpty) { 
                    printf("Enter assembly name: ");
                    scanf("%s", name);
                }
                printf("Enter new detail name: ");
                scanf("%s", newName);
                detail.type = inputDetailClass();
                if (notEmpty) { 
                    db.lock();
                    if (cursor.select(q1) > 0) { 
                        detail.assembly = cursor.currentId();
                    } else { 
                        printf("No such detail\n");
                        break;
                    }
                } else { 
                    detail.assembly = null;
                } 
                insert(detail);
                notEmpty = true;
                break;
              case 2:
                printf("Enter assembly name: ");
                scanf("%s", name);
                type = inputDetailClass();
                if (cursor.select(q1) != 0) { 
                    parts = cursor->parts;
                    if (cursor.select(q2)) { 
                        do { 
                            cursor->print();
                        } while (cursor.next());
                    } else { 
                        printf("This detail doesn't contain subparts\n");
                    }
                } else { 
                    printf("No such detail\n");
                }
                break;
              case 3:
                if (cursor.select("start from first follow by parts")) { 
                    do { 
                        cursor->print();
                    } while (cursor.next());
                } else { 
                    printf("No details\n");
                }
                break;
              case 4:
                printf("Enter assembly name: ");
                scanf("%s", name);
                if (cursor.select(q1) != 0) { 
                    parts = cursor->parts;
                    if (cursor.select(q3)) { 
                        do { 
                            cursor->print();
                        } while (cursor.next());
                    } else { 
                        printf("This detail doesn't contain subparts\n");
                    }
                } else { 
                    printf("No such detail\n");
                }
                break;
              case 5:
                printf("Enter detail name: ");
                scanf("%s", name);
                if (cursor.select(q1) != 0) { 
                    assembly = cursor->assembly;
                    if (cursor.select(q4)) { 
                        do { 
                            printf("assembly %s\n", cursor->name);
                        } while (cursor.next());
                    } else { 
                        printf("This detail is not part of any assembly\n");
                    }
                } else { 
                    printf("No such detail\n");
                }
                break;
              case 6:
                db.close();
                return EXIT_SUCCESS;
            }
            db.commit();
        }
    } else { 
        fprintf(stderr, "Failed to open database\n");
        return EXIT_FAILURE;
    }
}







