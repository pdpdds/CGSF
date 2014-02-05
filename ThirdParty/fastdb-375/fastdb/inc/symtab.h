//-< SYMTAB.H >----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Symbol table interface
//-------------------------------------------------------------------*--------*

#ifndef __SYMTAB_H__
#define __SYMTAB_H__

BEGIN_FASTDB_NAMESPACE

#if defined(CLONE_IDENTIFIERS) || defined(FASTDB_DLL)
#define FASTDB_CLONE_ANY_IDENTIFIER true
#else
#define FASTDB_CLONE_ANY_IDENTIFIER false
#endif

class  FASTDB_DLL_ENTRY dbSymbolTable { 
    struct HashTableItem { 
        HashTableItem* next;
        char*          str;
        unsigned       hash;
        int            tag;
        byte           allocated;
        
        ~HashTableItem() { 
            if (allocated) { 
                delete[] str;
            }
        }
    };
    static HashTableItem* hashTable[];

  public:
    ~dbSymbolTable();
    static dbSymbolTable instance;

    static int add(char* &str, int tag,  bool allocate = true);
};

END_FASTDB_NAMESPACE

#endif

