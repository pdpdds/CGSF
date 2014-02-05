#include "fastdb.h"
#include <stdio.h>
#include <string.h>

USE_FASTDB_NAMESPACE

int main(int argc, char* argv[])
{
    if (argc < 2) { 
        fprintf(stderr, "Usage: forcerecovery [-shadow] [-norecovery] DATABASE-FILE-NAME\n");
        return 1;
    }
    bool swapCurr = false;
    int  dirty = 1;
    FILE* f = NULL;
    for (int i = 1; i < argc; i++) { 
        if (strcmp(argv[i], "-shadow") == 0) { 
            swapCurr = true;
        } else if (strcmp(argv[i], "-norecovery") == 0) { 
            dirty = 0;
        } else if (*argv[i] == '-') { 
            fprintf(stderr, "No such option '%s'\n", argv[i]);
            return 1;
        } else if (f != NULL) {
            fprintf(stderr, "File was already specified\n");
            return 1;
        } else { 
            f = fopen(argv[i], "r+b");
            if (f == NULL) { 
                fprintf(stderr, "Failed to open database file '%s'\n", argv[i]);
                return 1;
            }
        }
    }
    if (f == NULL) { 
        fprintf(stderr, "File was not specified\n");
        return 1;
    }
    dbHeader rec;
    if (fread(&rec, sizeof(dbHeader), 1, f) != 1) { 
        fprintf(stderr, "Failed to read database header\n");
        return 1;
    }
    if (swapCurr) { 
        rec.curr = 1 - rec.curr;
    }
    rec.dirty = dirty;
    fseek(f, 0, 0);
    if (fwrite(&rec, sizeof(dbHeader), 1, f) != 1) { 
        fprintf(stderr, "Failed to write database header\n");
        return 1;
    }
    fclose(f);
    return 0;
}
