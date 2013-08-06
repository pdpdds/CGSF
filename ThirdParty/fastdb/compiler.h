//-< COMPILE.H >-----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 17-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Conditional expresion compiler
//-------------------------------------------------------------------*--------*

#ifndef __COMPILER_H__
#define __COMPILER_H__

#include <setjmp.h>
#ifdef USE_REGEX
#include <regex.h>
#endif

BEGIN_FASTDB_NAMESPACE

#if defined(__osf__) || defined(__FreeBSD__)
#define longjmp(b,s) _longjmp(b,s) // do not restore signal context
#define setjmp(b)  _setjmp(b)
#endif

enum dbvmCodes { 
#define DBVM(cop, type, n_operands, commutative) cop,
#ifdef VXWORKS
#include "compiler.dd"
#else
#include "compiler.d"
#endif // VXWORKS
dbvmLastCode
};

#define IS_CONSTANT(c) \
    (unsigned(c) - dbvmLoadVarBool <= (unsigned)dbvmLoadRectangleConstant - dbvmLoadVarBool)

enum nodeType { 
    tpInteger,
    tpBoolean,
    tpReal,
    tpString,
    tpWString,
    tpReference,
    tpRectangle,
    tpArray,
    tpRawBinary, 
    tpFreeVar,  // index of EXISTS clause 
    tpList,     // list of expressions
    tpVoid
};

enum tokens { 
    tkn_ident,
    tkn_lpar,
    tkn_rpar,
    tkn_lbr,
    tkn_rbr,
    tkn_dot,
    tkn_comma,
    tkn_power,
    tkn_iconst,
    tkn_sconst,
    tkn_fconst,
    tkn_add,
    tkn_sub,
    tkn_mul,
    tkn_div,
    tkn_and,
    tkn_or,
    tkn_not,
    tkn_null,
    tkn_neg,
    tkn_eq,
    tkn_ne,
    tkn_gt,
    tkn_ge,
    tkn_lt,
    tkn_le,
    tkn_between,
    tkn_escape,
    tkn_exists,
    tkn_like,
    tkn_limit,
    tkn_in,
    tkn_length,
    tkn_lower,
    tkn_upper,
    tkn_abs,
    tkn_area,
    tkn_is,
    tkn_integer,
    tkn_real,
    tkn_string,
    tkn_wstring,
    tkn_first,
    tkn_last,
    tkn_current,
    tkn_var,
    tkn_col,
    tkn_true,
    tkn_false,
    tkn_where, 
    tkn_follow,
    tkn_start,
    tkn_from,
    tkn_order,
    tkn_overlaps,
    tkn_by,
    tkn_asc,
    tkn_desc, 
    tkn_eof,
    tkn_insert, 
    tkn_into, 
    tkn_select, 
    tkn_table,
    tkn_error,
    tkn_all, 
    tkn_match,
    tkn_last_token
};    

struct dbStrLiteral { 
    char* str;
    int   len;
};

struct dbWStrLiteral { 
    wchar_t* str;
    int      len;
};


class dbUserFunction;
class dbExprNodeSegment;

class FASTDB_DLL_ENTRY dbExprNodeAllocator { 
  private:
    friend class dbExprNodeSegment;
    dbExprNode*        freeNodeList;
    dbExprNodeSegment* segmentList;
    dbMutex            mutex;
    
  public:  
    dbMutex&    getMutex() {
        return mutex;
    }
    dbExprNode* allocate();
    void        deallocate(dbExprNode* node);
    void        reset();

    ~dbExprNodeAllocator();
    static dbExprNodeAllocator instance;
};

class FASTDB_DLL_ENTRY dbExprNode { 
    friend class dbExprNodeSegment;
  public:
    nat1 cop;
    nat1 type;
    nat2 offs;

    static const nat1  nodeTypes[];
    static const nat1  nodeOperands[];
    static const nat1  commutativeOperator[];

    struct ref_operands { 
        dbExprNode*         base;  // the same as operand[0]
        dbFieldDescriptor*  field;
    };

    struct func_operands { 
        dbExprNode*         arg[3]; 
        void*               fptr;
    };

#ifdef USE_REGEX
    struct regex_operands { 
        dbExprNode*         opd;  
        regex_t             re;
    };
#endif

    union { 
        dbExprNode*    operand[3];
        dbExprNode*    next;
        oid_t          oid;
        db_int8        ivalue;
        real8          fvalue;
        rectangle      rvalue;
        dbStrLiteral   svalue;
        dbWStrLiteral  wsvalue;
        void const*    var;
        ref_operands   ref;
        func_operands  func;
#ifdef USE_REGEX
        regex_operands regex;
#endif
    };

    dbExprNode(dbExprNode* node);

    dbExprNode(int cop, dbExprNode* left = NULL, dbExprNode* right = NULL, 
               dbExprNode* right2 = NULL)
    {
        this->cop = cop;
        type = nodeTypes[cop];
        operand[0] = left;
        operand[1] = right;
        operand[2] = right2;
    }
    dbExprNode(int cop, dbExprNode* expr1, dbExprNode* expr2, int offs) { 
        this->cop = cop;
        this->offs = (nat2)offs;
        type = nodeTypes[cop];
        operand[0] = expr1;
        operand[1] = expr2;
    }
    dbExprNode(int cop, dbExprNode* expr, int offs) { 
        this->cop = cop;
        this->offs = (nat2)offs;
        type = nodeTypes[cop];
        operand[0] = expr;
    }
    dbExprNode(int cop, dbFieldDescriptor* field, dbExprNode* base = NULL) 
    {
        this->cop = cop;
        this->offs = (nat2)field->dbsOffs;
        type = nodeTypes[cop];
        ref.field = field;
        ref.base = base;
    }
    dbExprNode(int cop, db_int8 ivalue) { 
        this->cop = cop;
        this->ivalue = ivalue;
        type = tpInteger;
    }
    dbExprNode(int cop, real8 fvalue) { 
        this->cop = cop;
        this->fvalue = fvalue;
        type = tpReal;
    }
    dbExprNode(int cop, rectangle rvalue) {
        this->cop = cop;
        this->rvalue = rvalue;
        type = tpRectangle;
    }
    dbExprNode(int cop, dbStrLiteral& svalue) { 
        this->cop = cop;
        this->svalue = svalue;
        type = tpString;
    }
    dbExprNode(int cop, dbWStrLiteral& wsvalue) { 
        this->cop = cop;
        this->wsvalue = wsvalue;
        type = tpWString;
    }
    dbExprNode(int cop, void const* var) { 
        this->cop = cop;
        this->var = var;
        type = nodeTypes[cop];
    }
    dbExprNode(int cop, void* fptr, dbExprNode* expr1, dbExprNode* expr2 = NULL, dbExprNode* expr3 = NULL) { 
        this->cop = cop;
        func.arg[0] = expr1;
        func.arg[1] = expr2;
        func.arg[2] = expr3;
        func.fptr = fptr;
        type = nodeTypes[cop];
    }
    ~dbExprNode();

    void* operator new(size_t size EXTRA_DEBUG_NEW_PARAMS) { 
        return dbExprNodeAllocator::instance.allocate();
    }

    void operator delete(void* ptr EXTRA_DEBUG_NEW_PARAMS) { 
        dbExprNodeAllocator::instance.deallocate((dbExprNode*)ptr);
    }
};


class dbExprNodeSegment { 
  public:
    enum { allocationQuantum = 1024};
    char               buf[sizeof(dbExprNode)*allocationQuantum];
    dbExprNodeSegment* next;
};


class dbBinding { 
  public:
    dbBinding*  next;
    char const* name;
    bool        used;
    int         index;
};

class dbOrderByNode { 
  public:
    dbOrderByNode*     next;
    dbFieldDescriptor* field;
    dbTableDescriptor* table;
    dbExprNode*        expr;
    bool               ascent;  // true for ascent order, false for descent 
    
    dbFieldDescriptor* getField() {
        return (field != NULL) ? field
            : ((unsigned)expr->cop - dbvmLoadBool <= dbvmLoadRawBinary - dbvmLoadBool) 
               ? expr->ref.field : NULL;
    }

    ~dbOrderByNode() { 
        delete expr;
    }
};

class dbFollowByNode { 
  public:
    dbFollowByNode*    next;
    dbFieldDescriptor* field;
};

class FASTDB_DLL_ENTRY dbCompiler { 
    friend class dbQuery;
    friend class dbQueryElement;
  public:
    enum { 
        maxSymbolLen    = 4096,
        maxFreeVars  = 4
    };

    dbTableDescriptor* table;
    dbQueryElement*    queryElement;
    int                currPos;
    int                firstPos;
    int                offsetWithinStatement;
    int                bvalue;
    db_int8            ivalue;
    real8              fvalue;
    dbStrLiteral       svalue;
    dbWStrLiteral      wsvalue;
    bool               hasToken;
    int                lex;
    char*              name;
    dbBinding*         bindings;
    int                nFreeVars;
    int                varType;
    void const*        varPtr;
    dbTableDescriptor* varRefTable;

    jmp_buf            abortCompilation;
    static bool        initialized;

    void        compare(dbExprNode* expr, dbExprNode* list);

    int         scan();
    void        ungetToken(int tkn) {
        lex = tkn;
        hasToken = true;
    }
    void        error(const char* msg, int pos = -1);
    dbExprNode* conjunction();    
    dbExprNode* disjunction();
    dbExprNode* comparison();    
    dbExprNode* addition();    
    dbExprNode* multiplication();    
    dbExprNode* power();
    dbExprNode* term();
    dbExprNode* buildList();
    dbExprNode* userDefinedOperator();
    dbExprNode* field(dbExprNode* expr, dbTableDescriptor* refTable,
                      dbFieldDescriptor* fd);

    bool        compile(dbTableDescriptor* table, dbQuery& query);
    dbExprNode* compileExpression(dbTableDescriptor* table,  char const* expr, int startPos);
    void        compileOrderByPart(dbQuery& query);
    void        compileLimitPart(dbQuery& query);
    void        compileStartFollowPart(dbQuery& query);

    void        deleteNode(dbExprNode* node);
    dbExprNode* rectangleConstant(dbExprNode* head);     

    dbCompiler();
};

class dbDatabaseThreadContext : public dbL2List { 
  public:
    int readAccess;
    int writeAccess;
    int concurrentId;
    int mutatorCSLocked;
    int isMutator;

    dbL2List cursors; 
    
    dbCompiler compiler;

    dbProcessId currPid;

    bool     interactive;
    bool     catched;
    bool     commitDelayed;
    bool     removeContext;
    jmp_buf  unwind;
    
    dbDatabaseThreadContext() { 
        concurrentId = 0;
        readAccess = false;
        writeAccess = false;
        mutatorCSLocked = false;
        isMutator = false;
        interactive = false;
        catched = false;
        commitDelayed = false;
        removeContext = false;
        currPid = dbProcessId::getCurrent();
    }
};

union dbSynthesizedAttribute { 
    byte*     base;
    int       bvalue;
    db_int8   ivalue;
    real8     fvalue;
    rectangle rvalue;
    void*     raw;
    oid_t     oid;

    struct { 
        char* base;
        int   size;
        dbUDTComparator comparator;
    } array;
};

struct dbStringValue;

struct FASTDB_DLL_ENTRY dbInheritedAttribute { 
    byte*          record;
    oid_t          oid;
    dbTable*       table;
    dbDatabase*    db;
    dbStringValue* tempStrings;
    size_t         paramBase;
    enum { 
        internalStrBufSize = 8*1024 
    };
    size_t         strBufPos;
    char           strBuf[internalStrBufSize];
    
    struct { 
        int     index;
        jmp_buf unwind;
    } exists_iterator[dbCompiler::maxFreeVars];

    void removeTemporaries();

    dbInheritedAttribute() { 
        tempStrings = NULL;
        strBufPos = 0;
    }

    ~dbInheritedAttribute() { 
        removeTemporaries(); 
    }
};

struct dbStringValue { 
    dbStringValue* next;
    char           str[1];

    static char* create(size_t size, dbInheritedAttribute& attr) { 
        if (attr.strBufPos + size > sizeof(attr.strBuf)) { 
            dbStringValue* sv = 
                (dbStringValue*)new char[offsetof(dbStringValue, str) + size];
            sv->next = attr.tempStrings;
            attr.tempStrings = sv;
            return sv->str;
        } else { 
            char* p = attr.strBuf + attr.strBufPos;
            attr.strBufPos += size;
            return p;
        }
    }

    static char* create(char const* s, dbInheritedAttribute& attr) {
        size_t len = strlen(s) + 1;
        char*  buf;
        if (attr.strBufPos + len > sizeof(attr.strBuf)) { 
            dbStringValue* sv = 
                (dbStringValue*)new char[offsetof(dbStringValue,str)+len];
            sv->next = attr.tempStrings;
            attr.tempStrings = sv;
            buf = sv->str;
        } else { 
            buf = attr.strBuf + attr.strBufPos;
            attr.strBufPos += len;
        }
        return strcpy(buf, s);
    }
};

inline char* findWildcard(char* pattern, char* escape = NULL) 
{
    if (escape == NULL) { 
        while (*pattern != dbMatchAnyOneChar &&
               *pattern != dbMatchAnySubstring)
        {
            if (*pattern++ == '\0') { 
                return NULL;
            }
        }
    } else { 
        char esc = *escape;
        while (*pattern != dbMatchAnyOneChar &&
               *pattern != dbMatchAnySubstring &&
               *pattern != esc)
        {
            if (*pattern++ == '\0') { 
                return NULL;
            }
        }
    }
    return pattern;
}

inline wchar_t* findWildcard(wchar_t* pattern, wchar_t* escape = NULL) 
{
    if (escape == NULL) { 
        while (*pattern != dbMatchAnyOneChar &&
               *pattern != dbMatchAnySubstring)
        {
            if (*pattern++ == '\0') { 
                return NULL;
            }
        }
    } else { 
        wchar_t esc = *escape;
        while (*pattern != dbMatchAnyOneChar &&
               *pattern != dbMatchAnySubstring &&
               *pattern != esc)
        {
            if (*pattern++ == '\0') { 
                return NULL;
            }
        }
    }
    return pattern;
}
        
END_FASTDB_NAMESPACE
    
#endif
