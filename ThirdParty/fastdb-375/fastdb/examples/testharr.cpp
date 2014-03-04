//-< TESTHARR.CPP >--------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     26-Mar-2003  K.A. Knizhnik  * / [] \ *
//                          Last update: 26-Mar-2003  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Tets of hierarhical arrays and bitmap
//-------------------------------------------------------------------*--------*


#include "fastdb.h"
#include "harray.h"

USE_FASTDB_NAMESPACE

class RootObject { 
  public:
    dbBitmap bitmap;

    TYPE_DESCRIPTOR((FIELD(bitmap)));
};

    
REGISTER(RootObject);

int main() 
{    
    dbDatabase db; 
    char buf[128];
    if (db.open(_T("testharr"))) {
        dbBitmap* bitmap;
        RootObject root;
        dbCursor<RootObject> cursor(dbCursorForUpdate);
        if (cursor.select() == 0) {
            root.bitmap.create(&db);
            insert(root);
            bitmap = &root.bitmap;
        } else { 
            bitmap = &cursor->bitmap;
        }
        printf("Commands:\n"
               "\tset   BITNO\n"
               "\tclear BITNO\n"
               "\ttest  BITNO\n"
               "\texit\n");
        while (fgets(buf, (int)sizeof(buf), stdin) != NULL) {             
            char cmd[64];
            int position;
            *cmd = '\0';
            int n = sscanf(buf, "%s%d", cmd, &position);
            if (strcmp(cmd, "exit") == 0) { 
                break;
            }
            if (n != 2) { 
                fprintf(stderr, "The number of the bit should be specified\n");
            } else { 
                if (strcmp(cmd, "set") == 0) { 
                    bitmap->set(position, true, &db);
                } else if (strcmp(cmd, "clear") == 0) { 
                    bitmap->set(position, false, &db);
                } else if (strcmp(cmd, "test") == 0) { 
                    if (bitmap->get(position, &db)) {
                        printf("The bit is set\n");
                    } else { 
                        printf("The bit not set\n");
                    }
                } else { 
                    fprintf(stderr, "Commands:\n"
                            "\tset   BITNO\n"
                            "\tclear BITNO\n"
                            "\ttest  BITNO\n"
                            "\texit\n");
                }
            }
        }
        db.close();
        printf("The program is terminated\n");
        return EXIT_SUCCESS;
    } else { 
        return EXIT_FAILURE;
    }
}
