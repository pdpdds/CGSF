//-< SYMTAB.CPP >----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 20-Nov-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Symbol table implementation
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "stdtp.h"
#include "sync.h"
#include "symtab.h"

BEGIN_FASTDB_NAMESPACE

const size_t hashTableSize = 1009;
dbSymbolTable::HashTableItem* dbSymbolTable::hashTable[hashTableSize];

dbSymbolTable dbSymbolTable::instance;

dbSymbolTable::~dbSymbolTable() { 
    for (int i = hashTableSize; --i >= 0;) { 
        HashTableItem *ip, *next;
        for (ip = hashTable[i]; ip != NULL; ip = next) {
            next = ip->next;
            delete ip;
        }
    }
}
#ifdef IGNORE_CASE
#define strcmp(x,y) stricmp(x,y)
#endif

int dbSymbolTable::add(char* &str, int tag, bool allocate) {
    static dbMutex mutex;
    dbCriticalSection cs(mutex);
    unsigned hash = 0;
    byte* p = (byte*)str;
    while (*p != 0) { 
        byte b = *p++;
#ifdef IGNORE_CASE
        b = tolower(b);
#endif
        hash = hash*31 + b;
    }
    int index = hash % hashTableSize;
    HashTableItem *ip;
    for (ip = hashTable[index]; ip != NULL; ip = ip->next) { 
        if (ip->hash == hash && strcmp(ip->str, str) == 0) { 
            str = ip->str;
            if (tag > ip->tag) { 
                ip->tag = tag;
            }
            return ip->tag;
        }
    }
    ip = new HashTableItem;
    ip->allocated = false;
    if (allocate) { 
        char* dupstr = new char[strlen(str) + 1];
        strcpy(dupstr, str);
        str = dupstr;
        ip->allocated = true;
    }
    ip->str = str;
    ip->hash = hash;
    ip->tag = tag;
    ip->next = hashTable[index];
    hashTable[index] = ip;
    return tag;
}

END_FASTDB_NAMESPACE
