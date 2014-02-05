//-< DATABASE.CPP >--------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 14-Jan-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Database memory management, query execution, scheme evaluation
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "fastdb.h"
#include "compiler.h"
#include "symtab.h"
#include "hashtab.h"
#include "ttree.h"
#include "rtree.h"
#include <ctype.h>
#include <wctype.h>
#include <math.h>

#ifdef VXWORKS
#include "fastdbShim.h"
#endif // VXWORKS

BEGIN_FASTDB_NAMESPACE

#ifdef HANDLE_ASSERTION_FAILURES
#define FASTDB_ASSERT(_cond) do { if (!(_cond)) handleError(AssertionFailed, __FILE__ ":" #_cond, __LINE__); } while (0)
#define FASTDB_ASSERT_EX(_db, _cond) do { if (!(_cond)) (_db)->handleError(AssertionFailed, __FILE__ ":" #_cond, __LINE__); } while (0)
#else
#define FASTDB_ASSERT(_cond) assert(_cond)
#define FASTDB_ASSERT_EX(_db, _cond) assert(_cond)
#endif



dbNullReference null;

char const* const dbMetaTableName = "Metatable";
 
size_t dbDatabase::internalObjectSize[] = {
    0,
    dbPageSize,
    sizeof(dbTtree),
    sizeof(dbTtreeNode),
    sizeof(dbHashTable),
    sizeof(dbHashTableItem),
    sizeof(dbRtree),
    sizeof(dbRtreePage)
};

int dbDatabase::getVersion()
{
    return header->getVersion();
}

FixedSizeAllocator::FixedSizeAllocator() 
{ 
    minSize = 0;
    maxSize = 0;
    bufSize = 0;
    quantum = 0;
    nChains = 0;
    chains = NULL;
    holes = NULL;
    vacant = NULL;
}

FixedSizeAllocator::~FixedSizeAllocator() 
{ 
    TRACE_MSG(("hits=%ld, faults=%ld, retries=%ld\n", (long)hits, (long)faults, (long)retries));
    delete[] chains;
    delete[] holes;
}

void FixedSizeAllocator::reset() 
{
    memset(chains, 0, sizeof(Hole*)*nChains);
    if (bufSize > 0) { 
        for (size_t i = 1; i < bufSize; i++) { 
            holes[i-1].next = &holes[i];
        }
        holes[bufSize-1].next = NULL;
    }
    vacant = holes;

    hits = 0;
    faults = 0;
    retries = 0;
}

void FixedSizeAllocator::init(size_t minSize, size_t maxSize, size_t quantum, size_t bufSize) 
{ 
    delete[] chains;
    delete[] holes;
    this->minSize = minSize;
    this->maxSize = maxSize;
    this->quantum = quantum;
    this->bufSize = bufSize;
    nChains = (maxSize - minSize + quantum - 1) / quantum + 1;
    chains = new Hole*[nChains];
    holes = new Hole[bufSize];
    reset();
}


coord_t FASTDB_DLL_ENTRY distance(rectangle const& r, rectangle const& q)
{
    if (r & q) { 
        return 0;
    }
    coord_t d = 0;;
    for (int i = 0; i < rectangle::dim; i++) { 
        if (r.boundary[i] > q.boundary[rectangle::dim+i]) { 
            coord_t di = r.boundary[i] - q.boundary[rectangle::dim+i];
            d += di*di;
        } else if (q.boundary[i] > r.boundary[rectangle::dim+i]) { 
            coord_t di = q.boundary[i] - r.boundary[rectangle::dim+i];
            d += di*di;
        }
    }
    return (coord_t)sqrt((double)d);
}

inline void convertIntToString(dbInheritedAttribute&   iattr,
                               dbSynthesizedAttribute& sattr)
{
    char buf[32];
    sattr.array.size = sprintf(buf, INT8_FORMAT, sattr.ivalue) + 1;
    sattr.array.base = dbStringValue::create(buf, iattr);
    sattr.array.comparator = NULL;
}

inline void convertRealToString(dbInheritedAttribute&   iattr,
                                dbSynthesizedAttribute& sattr)
{
    char buf[32];
    sattr.array.size = sprintf(buf, "%f", sattr.fvalue) + 1;
    sattr.array.base = dbStringValue::create(buf, iattr);
    sattr.array.comparator = NULL;
}

inline void convertWcsToMbs(dbInheritedAttribute&   iattr,
                            dbSynthesizedAttribute& sattr)
{
    size_t bufSize = sattr.array.size*MAX_MULTIBYTE_CHARACTER_LENGTH;
    char* buf = dbStringValue::create(bufSize, iattr);
    size_t size = wcstombs(buf, (wchar_t*)sattr.array.base, bufSize-1);    
    sattr.array.size = (int)size+1;
    sattr.array.base = buf;
    buf[size] = '\0';
}

inline void convertMbsToWcs(dbInheritedAttribute&   iattr,
                            dbSynthesizedAttribute& sattr)
{
    size_t bufSize = sattr.array.size*sizeof(wchar_t);
    char* buf = dbStringValue::create(bufSize, iattr);
    size_t size = mbstowcs((wchar_t*)buf, sattr.array.base, sattr.array.size-1);    
    sattr.array.size = (int)size+1;
    sattr.array.base = buf;
    *((wchar_t*)buf + size) = '\0';
}

inline void concatenateStrings(dbInheritedAttribute&   iattr,
                               dbSynthesizedAttribute& sattr,
                               dbSynthesizedAttribute& sattr2)
{
    char* str = 
        dbStringValue::create(sattr.array.size + sattr.array.size - 1, iattr);
    memcpy(str, sattr.array.base, sattr.array.size-1);
    memcpy(str + sattr.array.size - 1, sattr2.array.base, sattr2.array.size);
    sattr.array.base = str;
    sattr.array.size += sattr2.array.size-1;
}

inline void concatenateWStrings(dbInheritedAttribute&   iattr,
                                dbSynthesizedAttribute& sattr,
                                dbSynthesizedAttribute& sattr2)
{
    char* str = 
        dbStringValue::create((sattr.array.size + sattr.array.size - 1)*sizeof(wchar_t), iattr);
    memcpy(str, sattr.array.base, (sattr.array.size-1)*sizeof(wchar_t));
    memcpy(str + (sattr.array.size-1)*sizeof(wchar_t), sattr2.array.base, sattr2.array.size*sizeof(wchar_t));
    sattr.array.base = str;
    sattr.array.size += sattr2.array.size-1;    
}

inline bool compareStringsForEquality(dbSynthesizedAttribute& sattr1,
                                     dbSynthesizedAttribute& sattr2)
{
    if (sattr1.array.comparator != NULL) {
        return sattr1.array.comparator(sattr1.array.base, sattr2.array.base, MAX_STRING_LENGTH) == 0;
    } else if (sattr2.array.comparator != NULL) {
        return sattr2.array.comparator(sattr1.array.base, sattr2.array.base, MAX_STRING_LENGTH) == 0;
    } else { 
#ifdef IGNORE_CASE
        return stricmp(sattr1.array.base, sattr2.array.base) == 0;
#else
        return sattr1.array.size == sattr2.array.size && memcmp(sattr1.array.base, sattr2.array.base, sattr1.array.size) == 0;
#endif
    }
}

inline bool compareWStringsForEquality(dbSynthesizedAttribute& sattr1,
                                     dbSynthesizedAttribute& sattr2)
{
    if (sattr1.array.comparator != NULL) {
        return sattr1.array.comparator(sattr1.array.base, sattr2.array.base, MAX_STRING_LENGTH) == 0;
    } else if (sattr2.array.comparator != NULL) {
        return sattr2.array.comparator(sattr1.array.base, sattr2.array.base, MAX_STRING_LENGTH) == 0;
    } else { 
#ifdef IGNORE_CASE
        return wcsicmp((wchar_t*)sattr1.array.base, (wchar_t*)sattr2.array.base) == 0;
#else
        return sattr1.array.size == sattr2.array.size && memcmp(sattr1.array.base, sattr2.array.base, sattr1.array.size*sizeof(wchar_t)) == 0;
#endif
    }
}

inline int compareStrings(dbSynthesizedAttribute& sattr1,
                          dbSynthesizedAttribute& sattr2)
{
    if (sattr1.array.comparator != NULL) {
        return sattr1.array.comparator(sattr1.array.base, sattr2.array.base, MAX_STRING_LENGTH);
    } else if (sattr2.array.comparator != NULL) {
        return sattr2.array.comparator(sattr1.array.base, sattr2.array.base, MAX_STRING_LENGTH);
    } else { 
#ifdef USE_LOCALE_SETTINGS
#ifdef IGNORE_CASE
        return stricoll(sattr1.array.base, sattr2.array.base);
#else
        return strcoll(sattr1.array.base, sattr2.array.base);
#endif
#else
#ifdef IGNORE_CASE
        return stricmp(sattr1.array.base, sattr2.array.base);
#else
        return strcmp(sattr1.array.base, sattr2.array.base);
#endif
#endif
    }
}

inline int compareWStrings(dbSynthesizedAttribute& sattr1,
                           dbSynthesizedAttribute& sattr2)
{
    if (sattr1.array.comparator != NULL) {
        return sattr1.array.comparator(sattr1.array.base, sattr2.array.base, MAX_STRING_LENGTH);
    } else if (sattr2.array.comparator != NULL) {
        return sattr2.array.comparator(sattr1.array.base, sattr2.array.base, MAX_STRING_LENGTH);
    } else { 
#ifdef IGNORE_CASE
        return wcsicmp((wchar_t*)sattr1.array.base, (wchar_t*)sattr2.array.base);
#else
        return wcscmp((wchar_t*)sattr1.array.base, (wchar_t*)sattr2.array.base);
#endif
    }
}

#ifdef IGNORE_CASE
#define GET_CHAR(c) toupper((byte)(c))
#define GET_WCHAR(c) towupper(c)
#else
#define GET_CHAR(c) (c)
#define GET_WCHAR(c) (c)
#endif

dbException::dbException(int p_err_code, char const* p_msg, int p_arg)
: err_code (p_err_code),
  msg (NULL),
  arg (p_arg)
{ 
    if (p_msg != NULL) { 
        msg = new char[strlen(p_msg)+1]; 
        strcpy(msg, p_msg);
    }
}

dbException::dbException(dbException const& ex) 
{ 
    err_code = ex.err_code;
    arg = ex.arg;
    if (ex.msg != NULL) { 
        msg = new char[strlen(ex.msg)+1]; 
        strcpy(msg, ex.msg);
    } else { 
        msg = NULL;
    }
}  

dbException::~dbException() throw()
{ 
    delete[] msg;
}

const char* dbException::what() const throw()
{
    return getMsg();
}



inline bool matchStrings(dbSynthesizedAttribute& sattr1,
                         dbSynthesizedAttribute& sattr2,
                         char escapeChar)
{
    char *str = sattr1.array.base;
    char *pattern = sattr2.array.base;
    char *wildcard = NULL; 
    char *strpos = NULL;
    dbUDTComparator comparator = sattr1.array.comparator;

    if (comparator != NULL) { 
        while (true) { 
            if (*pattern == dbMatchAnySubstring) { 
                wildcard = ++pattern;
                strpos = str;
            } else if (*str == '\0') { 
                return (*pattern == '\0');
            } else if (*pattern == escapeChar && comparator(pattern+1, str, 1) == 0) { 
                str += 1;
                pattern += 2;
            } else if (*pattern != escapeChar
                       && (comparator(pattern, str, 1) == 0 || *pattern == dbMatchAnyOneChar))
            { 
                str += 1;
                pattern += 1;
            } else if (wildcard) { 
                str = ++strpos;
                pattern = wildcard;
            } else { 
                return false;
            }
        }
    } else { 
        while (true) { 
            int ch = GET_CHAR(*str);
            if (*pattern == dbMatchAnySubstring) { 
                wildcard = ++pattern;
                strpos = str;
            } else if (ch == '\0') { 
                return (*pattern == '\0');
            } else if (*pattern == escapeChar && GET_CHAR(pattern[1]) == ch) { 
                str += 1;
                pattern += 2;
            } else if (*pattern != escapeChar
                       && (ch == GET_CHAR(*pattern)
                           || *pattern == dbMatchAnyOneChar))
            { 
                str += 1;
                pattern += 1;
            } else if (wildcard) { 
                str = ++strpos;
                pattern = wildcard;
            } else { 
                return false;
            }
        }
    }
}


inline bool matchStrings(dbSynthesizedAttribute& sattr1,
                         dbSynthesizedAttribute& sattr2)
{
    char *str = sattr1.array.base;
    char *pattern = sattr2.array.base;
    char *wildcard = NULL; 
    char *strpos = NULL;
    dbUDTComparator comparator = sattr1.array.comparator;

    if (comparator != NULL) { 
        while (true) { 
            if (*pattern == dbMatchAnySubstring) { 
                wildcard = ++pattern;
                strpos = str;
            } else if (*str == '\0') { 
                return (*pattern == '\0');
            } else if (comparator(pattern, str, 1) == 0 || *pattern == dbMatchAnyOneChar) {
                str += 1;
                pattern += 1;
            } else if (wildcard) { 
                str = ++strpos;
                pattern = wildcard;
            } else { 
                return false;
            }
        }
    } else { 
        while (true) { 
            int ch = GET_CHAR(*str);
            if (*pattern == dbMatchAnySubstring) { 
                wildcard = ++pattern;
                strpos = str;
            } else if (ch == '\0') { 
                return (*pattern == '\0');
            } else if (ch == GET_CHAR(*pattern) || *pattern == dbMatchAnyOneChar) {
                str += 1;
                pattern += 1;
            } else if (wildcard) { 
                str = ++strpos;
                pattern = wildcard;
            } else { 
                return false;
            }
        }
    }
}

inline void lowercaseString(dbInheritedAttribute&   iattr,
                            dbSynthesizedAttribute& sattr) 
{ 
    char *dst = dbStringValue::create(sattr.array.size, iattr);
    char *src = sattr.array.base;
    sattr.array.base = dst;
    while ((*dst++ = tolower(byte(*src++))) != '\0');
}    

inline void uppercaseString(dbInheritedAttribute&   iattr,
                            dbSynthesizedAttribute& sattr) 
{ 
    char *dst = dbStringValue::create(sattr.array.size, iattr);
    char *src = sattr.array.base;
    sattr.array.base = dst;
    while ((*dst++ = toupper(byte(*src++))) != '\0');
}    

inline void copyString(dbInheritedAttribute&   iattr,
                       dbSynthesizedAttribute& sattr, char* str) 
{ 
    sattr.array.base = dbStringValue::create(str, iattr);
    sattr.array.size = (int)strlen(str) + 1;
    sattr.array.comparator = NULL;
    delete[] str;
}    

inline bool matchWStrings(dbSynthesizedAttribute& sattr1,
                          dbSynthesizedAttribute& sattr2,
                          wchar_t escapeChar)
{
    wchar_t *str = (wchar_t*)sattr1.array.base;
    wchar_t *pattern = (wchar_t*)sattr2.array.base;
    wchar_t *wildcard = NULL; 
    wchar_t *strpos = NULL;
    dbUDTComparator comparator = sattr1.array.comparator;

    if (comparator != NULL) { 
        while (true) { 
            if (*pattern == dbMatchAnySubstring) { 
                wildcard = ++pattern;
                strpos = str;
            } else if (*str == '\0') { 
                return (*pattern == '\0');
            } else if (*pattern == escapeChar && comparator(pattern+1, str, 1) == 0) { 
                str += 1;
                pattern += 2;
            } else if (*pattern != escapeChar
                       && (comparator(pattern, str, 1) == 0 || *pattern == dbMatchAnyOneChar))
            { 
                str += 1;
                pattern += 1;
            } else if (wildcard) { 
                str = ++strpos;
                pattern = wildcard;
            } else { 
                return false;
            }
        }
    } else { 
        while (true) { 
            int ch = GET_WCHAR(*str);
            if (*pattern == dbMatchAnySubstring) { 
                wildcard = ++pattern;
                strpos = str;
            } else if (ch == '\0') { 
            return (*pattern == '\0');
            } else if (*pattern == escapeChar && GET_WCHAR(pattern[1]) == ch) { 
                str += 1;
                pattern += 2;
            } else if (*pattern != escapeChar
                       && (ch == GET_WCHAR(*pattern)
                           || *pattern == dbMatchAnyOneChar))
            { 
                str += 1;
                pattern += 1;
            } else if (wildcard) { 
                str = ++strpos;
                pattern = wildcard;
            } else { 
                return false;
            }
        }
    }
}

inline bool matchWStrings(dbSynthesizedAttribute& sattr1,
                          dbSynthesizedAttribute& sattr2)
{
    wchar_t *str = (wchar_t*)sattr1.array.base;
    wchar_t *pattern = (wchar_t*)sattr2.array.base;
    wchar_t *wildcard = NULL; 
    wchar_t *strpos = NULL;
    dbUDTComparator comparator = sattr1.array.comparator;

    if (comparator != NULL) { 
        while (true) { 
            if (*pattern == dbMatchAnySubstring) { 
                wildcard = ++pattern;
                strpos = str;
            } else if (*str == '\0') { 
                return (*pattern == '\0');
            } else if (comparator(pattern, str, 1) == 0 || *pattern == dbMatchAnyOneChar) {
                str += 1;
                pattern += 1;
            } else if (wildcard) { 
                str = ++strpos;
                pattern = wildcard;
            } else { 
                return false;
            }
        }
    } else { 
        while (true) { 
            int ch = GET_WCHAR(*str);
            if (*pattern == dbMatchAnySubstring) { 
                wildcard = ++pattern;
                strpos = str;
            } else if (ch == '\0') { 
                return (*pattern == '\0');
            } else if (ch == GET_WCHAR(*pattern) || *pattern == dbMatchAnyOneChar) {
                str += 1;
                pattern += 1;
            } else if (wildcard) { 
                str = ++strpos;
                pattern = wildcard;
            } else { 
                return false;
            }
        }
    }
}


inline void lowercaseWString(dbInheritedAttribute&   iattr,
                             dbSynthesizedAttribute& sattr) 
{ 
    wchar_t *dst = (wchar_t*)dbStringValue::create(sattr.array.size*sizeof(wchar_t), iattr);
    wchar_t *src = (wchar_t*)sattr.array.base;
    sattr.array.base = (char*)dst;
    while ((*dst++ = towlower(*src++)) != '\0');
}    

inline void uppercaseWString(dbInheritedAttribute&   iattr,
                             dbSynthesizedAttribute& sattr) 
{ 
    wchar_t *dst = (wchar_t*)dbStringValue::create(sattr.array.size*sizeof(wchar_t), iattr);
    wchar_t *src = (wchar_t*)sattr.array.base;
    sattr.array.base = (char*)dst;
    while ((*dst++ = towupper(*src++)) != '\0');
}    


inline void copyWString(dbInheritedAttribute&  iattr,
                       dbSynthesizedAttribute& sattr, wchar_t* str) 
{ 
    size_t len = wcslen(str);
    sattr.array.base = dbStringValue::create(len + 1, iattr);
    sattr.array.size = (int)len + 1;
    sattr.array.comparator = NULL;
    memcpy(sattr.array.base, str, (len + 1)*sizeof(wchar_t));
    delete[] str;
}    

inline void searchArrayOfBool(dbSynthesizedAttribute& sattr, 
                              dbSynthesizedAttribute& sattr2)
{
    bool *p = (bool*)sattr2.array.base;
    int   n = sattr2.array.size;
    bool  v = (bool)sattr.bvalue;
    while (--n >= 0) { 
        if (v == *p++) { 
            sattr.bvalue = true;
            return;
        }
    }
    sattr.bvalue = false;
}

inline void searchArrayOfInt1(dbSynthesizedAttribute& sattr, 
                              dbSynthesizedAttribute& sattr2)
{
    int1 *p = (int1*)sattr2.array.base;
    int   n = sattr2.array.size;
    int1  v = (int1)sattr.ivalue;
    while (--n >= 0) { 
        if (v == *p++) { 
            sattr.bvalue = true;
            return;
        }
    }
    sattr.bvalue = false;
}

inline void searchArrayOfInt2(dbSynthesizedAttribute& sattr, 
                              dbSynthesizedAttribute& sattr2)
{
    int2 *p = (int2*)sattr2.array.base;
    int   n = sattr2.array.size;
    int2  v = (int2)sattr.ivalue;
    while (--n >= 0) { 
        if (v == *p++) { 
            sattr.bvalue = true;
            return;
        }
    }
    sattr.bvalue = false;
}

inline void searchArrayOfInt4(dbSynthesizedAttribute& sattr, 
                              dbSynthesizedAttribute& sattr2)
{
    int4 *p = (int4*)sattr2.array.base;
    int   n = sattr2.array.size;
    int4  v = (int4)sattr.ivalue;
    while (--n >= 0) { 
        if (v == *p++) { 
            sattr.bvalue = true;
            return;
        }
    }
    sattr.bvalue = false;
}

inline void searchArrayOfInt8(dbSynthesizedAttribute& sattr, 
                              dbSynthesizedAttribute& sattr2)
{
    db_int8 *p = (db_int8*)sattr2.array.base;
    int   n = sattr2.array.size;
    db_int8  v = sattr.ivalue;
    while (--n >= 0) { 
        if (v == *p) { 
            sattr.bvalue = true;
            return;
        }
        p += 1;
    }
    sattr.bvalue = false;
}

inline void searchArrayOfReal4(dbSynthesizedAttribute& sattr, 
                              dbSynthesizedAttribute& sattr2)
{
    real4* p = (real4*)sattr2.array.base;
    int    n = sattr2.array.size;
    real4  v = (real4)sattr.fvalue;
    while (--n >= 0) { 
        if (v == *p++) { 
            sattr.bvalue = true;
            return;
        }
    }
    sattr.bvalue = false;
}

inline void searchArrayOfReal8(dbSynthesizedAttribute& sattr, 
                               dbSynthesizedAttribute& sattr2)
{
    real8 *p = (real8*)sattr2.array.base;
    int    n = sattr2.array.size;
    real8  v = sattr.fvalue;
    while (--n >= 0) { 
        if (v == *p) { 
            sattr.bvalue = true;
            return;
        }
        p += 1;
    }
    sattr.bvalue = false;
}

inline void searchArrayOfReference(dbSynthesizedAttribute& sattr, 
                                   dbSynthesizedAttribute& sattr2)
{
    oid_t *p = (oid_t*)sattr2.array.base;
    int    n = sattr2.array.size;
    oid_t  v = sattr.oid;
    while (--n >= 0) { 
        if (v == *p) { 
            sattr.bvalue = true;
            return;
        }
        p += 1;
    }
    sattr.bvalue = false;
}

inline void searchArrayOfRectangle(dbInheritedAttribute&   iattr,
                                   dbSynthesizedAttribute& sattr,
                                   dbSynthesizedAttribute& sattr2)
{
    rectangle *p = (rectangle*)sattr2.array.base;
    int        n = sattr2.array.size;
    rectangle  v = sattr.rvalue;
    sattr.bvalue = false;
    while (--n >= 0) {
        if (v == *p) {
            sattr.bvalue = true;
            break;
        }
        p += 1;
    }
}

inline void searchArrayOfString(dbSynthesizedAttribute& sattr, 
                                dbSynthesizedAttribute& sattr2)
{
    dbVarying *p = (dbVarying*)sattr2.array.base;
    int        n = sattr2.array.size;
    char*      str = sattr.array.base;
    while (--n >= 0) { 
        if (strcmp((char*)p + p->offs, str) == 0) { 
            sattr.bvalue = true;
            return;
        }
        p += 1;
    }
    sattr.bvalue = false;
}

inline void searchInString(dbSynthesizedAttribute& sattr, 
                           dbSynthesizedAttribute& sattr2)
{
    if (sattr.array.size > sattr2.array.size) { 
        sattr.bvalue = false;
    } else if (sattr2.array.size > dbBMsearchThreshold) { 
        int len = sattr.array.size - 2;
        int n = sattr2.array.size - 1;
        int i, j, k;
        int shift[256];
        byte* pattern = (byte*)sattr.array.base;
        byte* str = (byte*)sattr2.array.base;
        for (i = 0; i < (int)itemsof(shift); i++) { 
            shift[i] = len+1;
        }
        for (i = 0; i < len; i++) { 
            shift[pattern[i]] = len-i;
        }
        for (i = len; i < n; i += shift[str[i]]) { 
            j = len;
            k = i;
            while (pattern[j] == str[k]) { 
                k -= 1;
                if (--j < 0) { 
                    sattr.bvalue = true;
                    return;
                }
            }
        }
        sattr.bvalue = false;
    } else { 
        sattr.bvalue = strstr(sattr2.array.base, sattr.array.base) != NULL;
    }
}

inline void searchArrayOfWString(dbSynthesizedAttribute& sattr, 
                                 dbSynthesizedAttribute& sattr2)
{
    dbVarying *p = (dbVarying*)sattr2.array.base;
    int        n = sattr2.array.size;
    wchar_t*   str = (wchar_t*)sattr.array.base;
    while (--n >= 0) { 
        if (wcscmp((wchar_t*)((char*)p + p->offs), str) == 0) { 
            sattr.bvalue = true;
            return;
        }
        p += 1;
    }
    sattr.bvalue = false;
}

inline void searchInWString(dbSynthesizedAttribute& sattr, 
                            dbSynthesizedAttribute& sattr2)
{
    if (sattr.array.size > sattr2.array.size) { 
        sattr.bvalue = false;
    } else { 
        sattr.bvalue = wcsstr((wchar_t*)sattr2.array.base, (wchar_t*)sattr.array.base) != NULL;
    }
}

inline db_int8 powerIntInt(db_int8 x, db_int8 y) 
{
    db_int8 res = 1;

    if (y < 0) {
        x = 1/x;
        y = -y;
    }
    while (y != 0) {
        if (y & 1) { 
            res *= x;
        }
        x *= x;
        y >>= 1;
    }
    return res;    
}

inline real8 powerRealInt(real8 x, db_int8 y) 
{
    real8 res = 1.0;

    if (y < 0) {
        x = 1/x;
        y = -y;
    }
    while (y != 0) {
        if (y & 1) { 
            res *= x;
        }
        x *= x;
        y >>= 1;
    }
    return res;    
}

bool dbDatabase::evaluate(dbExprNode* expr, oid_t oid, dbTable* table, dbAnyCursor* cursor)
{
    dbInheritedAttribute iattr;
    dbSynthesizedAttribute sattr;
    iattr.db = this;
    iattr.oid = oid;
    iattr.table = table;
    iattr.record = (byte*)getRow(oid);    
    iattr.paramBase = (size_t)cursor->paramBase;
    execute(expr, iattr, sattr);
    return sattr.bvalue != 0;
}

void _fastcall dbDatabase::execute(dbExprNode*             expr, 
                                   dbInheritedAttribute&   iattr, 
                                   dbSynthesizedAttribute& sattr)
{
    dbSynthesizedAttribute sattr2, sattr3;

    switch (expr->cop) {
      case dbvmVoid:
        sattr.bvalue = true; // empty condition
        return;
      case dbvmCurrent:
        sattr.oid = iattr.oid;
        return;
      case dbvmFirst:
        sattr.oid = iattr.table->firstRow;
        return;
      case dbvmLast:
        sattr.oid = iattr.table->lastRow;
        return;
      case dbvmLoadBool:
        execute(expr->operand[0], iattr, sattr);
        sattr.bvalue = *(bool*)(sattr.base+expr->offs);
        return;
      case dbvmLoadInt1:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = *(int1*)(sattr.base+expr->offs);
        return;
      case dbvmLoadInt2:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = *(int2*)(sattr.base+expr->offs);
        return;
      case dbvmLoadInt4:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = *(int4*)(sattr.base+expr->offs);
        return;
      case dbvmLoadInt8:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = *(db_int8*)(sattr.base+expr->offs);
        return;
      case dbvmLoadReal4:
        execute(expr->operand[0], iattr, sattr);
        sattr.fvalue = *(real4*)(sattr.base+expr->offs);
        return;
      case dbvmLoadReal8:
        execute(expr->operand[0], iattr, sattr);
        sattr.fvalue = *(real8*)(sattr.base+expr->offs);
        return;
      case dbvmLoadReference:
        execute(expr->operand[0], iattr, sattr);
        sattr.oid = *(oid_t*)(sattr.base+expr->offs);
        return;
      case dbvmLoadRectangle:
        execute(expr->operand[0], iattr, sattr);
        sattr.rvalue = *(rectangle*)(sattr.base+expr->offs);
        return;
      case dbvmLoadArray:
      case dbvmLoadString:
      case dbvmLoadWString:
        execute(expr->operand[0], iattr, sattr2);
        sattr.array.base = (char*)sattr2.base 
            + ((dbVarying*)(sattr2.base + expr->offs))->offs;
        sattr.array.size = ((dbVarying*)(sattr2.base + expr->offs))->size;
        sattr.array.comparator = expr->ref.field->_comparator;
        return;
      case dbvmLoadRawBinary:
        execute(expr->operand[0], iattr, sattr);
        sattr.raw = (void*)(sattr.base+expr->offs);
        return;

      case dbvmLoadSelfBool:
        sattr.bvalue = *(bool*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfInt1:
        sattr.ivalue = *(int1*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfInt2:
        sattr.ivalue = *(int2*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfInt4:
        sattr.ivalue = *(int4*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfInt8:
        sattr.ivalue = *(db_int8*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfReal4:
        sattr.fvalue = *(real4*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfReal8:
        sattr.fvalue = *(real8*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfReference:
        sattr.oid = *(oid_t*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfRectangle:
        sattr.rvalue = *(rectangle*)(iattr.record+expr->offs);
        return;
      case dbvmLoadSelfArray:
      case dbvmLoadSelfString:
      case dbvmLoadSelfWString:
        sattr.array.base = (char*)iattr.record + 
            ((dbVarying*)(iattr.record + expr->offs))->offs;
        sattr.array.size = ((dbVarying*)(iattr.record + expr->offs))->size;
        sattr.array.comparator = expr->ref.field->_comparator;
        return;
      case dbvmLoadSelfRawBinary:
        sattr.raw = (void*)(iattr.record+expr->offs);
        return;

      case dbvmInvokeMethodBool:
      {
          bool bvalue;
          execute(expr->ref.base, iattr, sattr);
          expr->ref.field->method->invoke(sattr.base, &bvalue);
          sattr.bvalue = bvalue;
          return;
      }
      case dbvmInvokeMethodInt1:
      {
          int1 ivalue;
          execute(expr->ref.base, iattr, sattr);
          expr->ref.field->method->invoke(sattr.base, &ivalue);
          sattr.ivalue = ivalue;
          return;
      }
      case dbvmInvokeMethodInt2:
      {
          int2 ivalue;
          execute(expr->ref.base, iattr, sattr);
          expr->ref.field->method->invoke(sattr.base, &ivalue);
          sattr.ivalue = ivalue;
          return;
      }
      case dbvmInvokeMethodInt4:
      {
          int4 ivalue;
          execute(expr->ref.base, iattr, sattr);
          expr->ref.field->method->invoke(sattr.base, &ivalue);
          sattr.ivalue = ivalue;
          return;
      }
      case dbvmInvokeMethodInt8:
        execute(expr->ref.base, iattr, sattr);
        expr->ref.field->method->invoke(sattr.base, &sattr.ivalue);
        return;
      case dbvmInvokeMethodReal4:
      {
          real4 fvalue;
          execute(expr->ref.base, iattr, sattr);
          expr->ref.field->method->invoke(sattr.base, &fvalue); 
          sattr.fvalue = fvalue;
          return;
      }
      case dbvmInvokeMethodReal8:
        execute(expr->ref.base, iattr, sattr);
        expr->ref.field->method->invoke(sattr.base, &sattr.fvalue);
        return;
      case dbvmInvokeMethodReference:
        execute(expr->ref.base, iattr, sattr);
        expr->ref.field->method->invoke(sattr.base, &sattr.oid);
        return;
      case dbvmInvokeMethodString:
        execute(expr->ref.base, iattr, sattr);
        expr->ref.field->method->invoke(sattr.base, &sattr2.array.base);
        sattr.array.size = (int)strlen(sattr2.array.base) + 1;
        sattr.array.base = dbStringValue::create(sattr2.array.base, iattr);
        sattr.array.comparator = NULL;
        delete[] sattr2.array.base;
        return;
      case dbvmInvokeMethodWString:
        execute(expr->ref.base, iattr, sattr);
        expr->ref.field->method->invoke(sattr.base, &sattr2.array.base);
        sattr.array.size = (int)wcslen((wchar_t*)sattr2.array.base) + 1;
        sattr.array.base = dbStringValue::create(sattr.array.size*sizeof(wchar_t), iattr);
        sattr.array.comparator = NULL;
        memcpy(sattr.array.base, sattr2.array.base, sattr.array.size*sizeof(wchar_t));
        delete[] (wchar_t*)sattr2.array.base;
        return;

      case dbvmInvokeSelfMethodBool:
      {
          bool bvalue;
          expr->ref.field->method->invoke(iattr.record, &bvalue);
          sattr.bvalue = bvalue;
          return;
      }
      case dbvmInvokeSelfMethodInt1:
      {
          int1 ivalue;
          expr->ref.field->method->invoke(iattr.record, &ivalue);
          sattr.ivalue = ivalue;
          return;
      }
      case dbvmInvokeSelfMethodInt2:
      {
          int2 ivalue;
          expr->ref.field->method->invoke(iattr.record, &ivalue);
          sattr.ivalue = ivalue;
          return;
      }
      case dbvmInvokeSelfMethodInt4:
      {
          int4 ivalue;
          expr->ref.field->method->invoke(iattr.record, &ivalue);
          sattr.ivalue = ivalue;
          return;
      }
      case dbvmInvokeSelfMethodInt8:
        expr->ref.field->method->invoke(iattr.record, &sattr.ivalue);
        return;
      case dbvmInvokeSelfMethodReal4:
      {
          real4 fvalue;
          expr->ref.field->method->invoke(iattr.record, &fvalue); 
          sattr.fvalue = fvalue;
          return;
      }
      case dbvmInvokeSelfMethodReal8:
        expr->ref.field->method->invoke(iattr.record, &sattr.fvalue);
        return;
      case dbvmInvokeSelfMethodReference:
        expr->ref.field->method->invoke(iattr.record, &sattr.oid);
        return;
      case dbvmInvokeSelfMethodString:
        expr->ref.field->method->invoke(iattr.record, &sattr2.array.base);
        sattr.array.size = (int)strlen(sattr2.array.base) + 1;
        sattr.array.base = dbStringValue::create(sattr2.array.base, iattr);
        sattr.array.comparator = NULL;
        delete[] sattr2.array.base;
        return;
      case dbvmInvokeSelfMethodWString:
        expr->ref.field->method->invoke(iattr.record, &sattr2.array.base);
        sattr.array.size = (int)wcslen((wchar_t*)sattr2.array.base) + 1;
        sattr.array.base = dbStringValue::create(sattr.array.size*sizeof(wchar_t), iattr);
        sattr.array.comparator = NULL;
        memcpy(sattr.array.base, sattr2.array.base, sattr.array.size*sizeof(wchar_t));
        delete[] (wchar_t*)sattr2.array.base;
        return;

      case dbvmLength:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = sattr.array.size;
        return;
      case dbvmStringLength:
      case dbvmWStringLength:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = sattr.array.size - 1;
        return;

      case dbvmGetAt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if ((nat8)sattr2.ivalue >= (nat8)sattr.array.size) { 
            if (expr->operand[1]->cop == dbvmVariable) { 
                longjmp(iattr.exists_iterator[expr->operand[1]->offs].unwind, 1);
            }
            iattr.removeTemporaries();
            iattr.db->handleError(IndexOutOfRangeError, NULL, 
                                  int(sattr2.ivalue));
        }
        sattr.base = (byte*)sattr.array.base + int(sattr2.ivalue)*expr->offs;
        return;
      case dbvmRectangleCoord:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if ((nat8)sattr2.ivalue >= rectangle::dim*2) {
            if (expr->operand[1]->cop == dbvmVariable) {
                longjmp(iattr.exists_iterator[expr->operand[1]->offs].unwind, 1);
            }
            iattr.removeTemporaries();
            iattr.db->handleError(IndexOutOfRangeError, NULL,
                                  int(sattr2.ivalue));
        }
        sattr.fvalue = sattr.rvalue.boundary[int(sattr2.ivalue)];
        return;
      case dbvmCharAt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if ((nat8)sattr2.ivalue >= (nat8)(sattr.array.size-1)) { 
            if (expr->operand[1]->cop == dbvmVariable) { 
                longjmp(iattr.exists_iterator[expr->operand[1]->offs].unwind, 1);
            }
            iattr.removeTemporaries();
            iattr.db->handleError(IndexOutOfRangeError, NULL, 
                                  int(sattr2.ivalue));
        }
        sattr.ivalue = (byte)sattr.array.base[int(sattr2.ivalue)];
        return;
      case dbvmWCharAt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if ((nat8)sattr2.ivalue >= (nat8)(sattr.array.size-1)) { 
            if (expr->operand[1]->cop == dbvmVariable) { 
                longjmp(iattr.exists_iterator[expr->operand[1]->offs].unwind, 1);
            }
            iattr.removeTemporaries();
            iattr.db->handleError(IndexOutOfRangeError, NULL, 
                                  int(sattr2.ivalue));
        }
        sattr.ivalue = *((wchar_t*)sattr.array.base + int(sattr2.ivalue));
        return;


      case dbvmExists:
        iattr.exists_iterator[expr->offs].index = 0;
        if (setjmp(iattr.exists_iterator[expr->offs].unwind) == 0) { 
            do { 
                execute(expr->operand[0], iattr, sattr);
                iattr.exists_iterator[expr->offs].index += 1;
            } while (!sattr.bvalue);
        } else {
            sattr.bvalue = false;
        }
        return;

      case dbvmVariable:
        sattr.ivalue = iattr.exists_iterator[expr->offs].index;
        return;

      case dbvmLoadVarBool:
        sattr.bvalue = *(bool*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarInt1:
        sattr.ivalue = *(int1*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarInt2:
        sattr.ivalue = *(int2*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarInt4:
        sattr.ivalue = *(int4*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarInt8:
        sattr.ivalue = *(db_int8*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarReal4:
        sattr.fvalue = *(real4*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarReal8:
        sattr.fvalue = *(real8*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarReference:
        sattr.oid = *(oid_t*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarRectangle:
        sattr.rvalue = *(rectangle*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarRectanglePtr:
        sattr.rvalue = **(rectangle**)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarString:
        sattr.array.base = ((char*)expr->var + iattr.paramBase);
        sattr.array.size = (int)strlen((char*)sattr.array.base) + 1;
        sattr.array.comparator = NULL;
        return;
      case dbvmLoadVarWString:
        sattr.array.base = ((char*)expr->var + iattr.paramBase);
        sattr.array.size = (int)wcslen((wchar_t*)sattr.array.base) + 1;
        sattr.array.comparator = NULL;
        return;
      case dbvmLoadVarStringPtr:
        sattr.array.base = *(char**)((char*)expr->var + iattr.paramBase);
        sattr.array.size = (int)strlen((char*)sattr.array.base) + 1;
        sattr.array.comparator = NULL;
        return;
      case dbvmLoadVarWStringPtr:
        sattr.array.base = *(char**)((char*)expr->var + iattr.paramBase);
        sattr.array.size = (int)wcslen((wchar_t*)sattr.array.base) + 1;
        sattr.array.comparator = NULL;
        return;
      case dbvmLoadVarArrayOfOid:
      case dbvmLoadVarArrayOfInt4:
      case dbvmLoadVarArrayOfInt8:
        sattr.array.base = (char*)((dbAnyArray*)((char*)expr->var + iattr.paramBase))->base();
        sattr.array.size = (int)((dbAnyArray*)((char*)expr->var + iattr.paramBase))->length();
        sattr.array.comparator = NULL;
        return;
      case dbvmLoadVarArrayOfOidPtr:
      case dbvmLoadVarArrayOfInt4Ptr:
      case dbvmLoadVarArrayOfInt8Ptr:
        {
            dbAnyArray* arr = *(dbAnyArray**)((char*)expr->var + iattr.paramBase);
            sattr.array.base = (char*)arr->base();
            sattr.array.size = (int)arr->length();
            sattr.array.comparator = NULL;
            return;
        }
      case dbvmLoadVarRawBinary:
        sattr.raw = (void*)((char*)expr->var + iattr.paramBase);
        return;
      case dbvmLoadVarRawBinaryPtr:
        sattr.raw = *(void**)((char*)expr->var + iattr.paramBase);
        return;

#ifdef USE_STD_STRING
      case dbvmLoadVarStdString:
        sattr.array.base = (char*)((std::string*)((char*)expr->var + iattr.paramBase))->c_str();
        sattr.array.size = (int)((std::string*)((char*)expr->var + iattr.paramBase))->length() + 1;
        sattr.array.comparator = NULL;
        return;
      case dbvmLoadVarStdWString:
        sattr.array.base = (char*)((std::wstring*)((char*)expr->var + iattr.paramBase))->c_str();
        sattr.array.size = (int)((std::wstring*)((char*)expr->var + iattr.paramBase))->length() + 1;
        sattr.array.comparator = NULL;
        return;
#endif

      case dbvmLoadTrue:
        sattr.bvalue = true;
        return;
      case dbvmLoadFalse:
        sattr.bvalue = false;
        return;
      case dbvmLoadNull:
        sattr.oid = 0;
        return;
      case dbvmLoadIntConstant:
        sattr.ivalue = expr->ivalue;
        return;
      case dbvmLoadRealConstant:
        sattr.fvalue = expr->fvalue;
        return;
      case dbvmLoadRectangleConstant:
        sattr.rvalue = expr->rvalue;
        return;
      case dbvmLoadStringConstant:
        sattr.array.base = expr->svalue.str;
        sattr.array.size = expr->svalue.len;
        sattr.array.comparator = NULL;
        return;
      case dbvmLoadWStringConstant:
        sattr.array.base = (char*)expr->wsvalue.str;
        sattr.array.size = expr->wsvalue.len;
        sattr.array.comparator = NULL;
        return;
    
      case dbvmOrBool:
        execute(expr->operand[0], iattr, sattr);
        if (sattr.bvalue == 0) { 
            execute(expr->operand[1], iattr, sattr);
        }
        return;
      case dbvmAndBool:
        execute(expr->operand[0], iattr, sattr);
        if (sattr.bvalue != 0) { 
            execute(expr->operand[1], iattr, sattr);
        }
        return;
      case dbvmNotBool:
        execute(expr->operand[0], iattr, sattr);
        sattr.bvalue = !sattr.bvalue; 
        return;

      case dbvmIsNull:
        execute(expr->operand[0], iattr, sattr);
        sattr.bvalue = sattr.oid == 0;
        return;

      case dbvmAddRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.rvalue += sattr2.rvalue;
        return;

      case dbvmNegInt:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = -sattr.ivalue;
        return;
      case dbvmAddInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.ivalue += sattr2.ivalue;
        return;
      case dbvmSubInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.ivalue -= sattr2.ivalue;
        return;
      case dbvmMulInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.ivalue *= sattr2.ivalue;
        return;
      case dbvmDivInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (sattr2.ivalue == 0) { 
            iattr.removeTemporaries();
            iattr.db->handleError(ArithmeticError, "Division by zero");
        } else { 
            sattr.ivalue /= sattr2.ivalue;
        }
        return;
      case dbvmAndInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.ivalue &= sattr2.ivalue;
        return;
      case dbvmOrInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.ivalue |= sattr2.ivalue;
        return;
      case dbvmNotInt:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = ~sattr.ivalue;
        return;
      case dbvmAbsInt:
        execute(expr->operand[0], iattr, sattr);
        if (sattr.ivalue < 0) { 
            sattr.ivalue = -sattr.ivalue;
        }
        return;
      case dbvmPowerInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (sattr.ivalue == 2) { 
            sattr.ivalue = sattr2.ivalue < 64 
                ? (nat8)1 << (int)sattr2.ivalue : 0;
        } else if (sattr.ivalue == 0 && sattr2.ivalue < 0) { 
            iattr.removeTemporaries();
            iattr.db->handleError(ArithmeticError, 
                                  "Raise zero to negative power");          
        } else { 
            sattr.ivalue = powerIntInt(sattr.ivalue, sattr2.ivalue);
        }
        return;

        
      case dbvmEqInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.ivalue == sattr2.ivalue;
        return;
      case dbvmNeInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.ivalue != sattr2.ivalue;
        return;
      case dbvmGtInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.ivalue > sattr2.ivalue;
        return;
      case dbvmGeInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.ivalue >= sattr2.ivalue;
        return;
      case dbvmLtInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.ivalue < sattr2.ivalue;
        return;
      case dbvmLeInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.ivalue <= sattr2.ivalue;
        return;
      case dbvmBetweenInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (sattr.ivalue < sattr2.ivalue) { 
            sattr.bvalue = false;
        } else { 
            execute(expr->operand[2], iattr, sattr2);
            sattr.bvalue = sattr.ivalue <= sattr2.ivalue;
        }
        return;

      case dbvmEqArray:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        {
            dbArray<char> a1(sattr.array.base, sattr.array.size);
            dbArray<char> a2(sattr2.array.base, sattr2.array.size);  
            sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(&a1, &a2, 0) == 0;
        }
        return;
      case dbvmNeArray:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        {
            dbArray<char> a1(sattr.array.base, sattr.array.size);
            dbArray<char> a2(sattr2.array.base, sattr2.array.size);  
            sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(&a1, &a2, 0) != 0;
        }
        return;
      case dbvmLeArray:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        {
            dbArray<char> a1(sattr.array.base, sattr.array.size);
            dbArray<char> a2(sattr2.array.base, sattr2.array.size);              
            sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(&a1, &a2, 0) <= 0;
        }
        return;
      case dbvmLtArray:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        {
            dbArray<char> a1(sattr.array.base, sattr.array.size);
            dbArray<char> a2(sattr2.array.base, sattr2.array.size);              
            sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(&a1, &a2, 0) < 0;
        }
        return;
      case dbvmGeArray:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        {
            dbArray<char> a1(sattr.array.base, sattr.array.size);
            dbArray<char> a2(sattr2.array.base, sattr2.array.size);              
            sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(&a1, &a2, 0) >= 0;
        }
        return;
      case dbvmGtArray:        
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        {
            dbArray<char> a1(sattr.array.base, sattr.array.size);
            dbArray<char> a2(sattr2.array.base, sattr2.array.size);              
            sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(&a1, &a2, 0) > 0;
        }
        return;
      case dbvmBetweenArray:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        {
            dbArray<char> a1(sattr.array.base, sattr.array.size);
            dbArray<char> a2(sattr2.array.base, sattr2.array.size);              
            if ((*(dbUDTComparator)expr->func.fptr)(&a1, &a2, 0) < 0)
            {
                sattr.bvalue = false;
            } else {
                execute(expr->operand[2], iattr, sattr2);
                dbArray<char> a3(sattr2.array.base, sattr2.array.size);              
                sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(&a1, &a3, 0) <= 0;
            }
        }
        return;

      case dbvmEqRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.rvalue == sattr2.rvalue;
        return;
      case dbvmNeRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.rvalue != sattr2.rvalue;
        return;
      case dbvmGtRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.rvalue > sattr2.rvalue;
        return;
      case dbvmGeRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.rvalue >= sattr2.rvalue;
        return;
      case dbvmLtRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.rvalue < sattr2.rvalue;
        return;
      case dbvmLeRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.rvalue <= sattr2.rvalue;
        return;

      case dbvmOverlapsRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.rvalue & sattr2.rvalue;
        return;

      case dbvmRectangleArea:
        execute(expr->operand[0], iattr, sattr);
        sattr.fvalue = (double)area(sattr.rvalue);
        return;

      case dbvmNegReal:
        execute(expr->operand[0], iattr, sattr);
        sattr.fvalue = -sattr.fvalue;
        return;
      case dbvmAddReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.fvalue += sattr2.fvalue;
        return;
      case dbvmSubReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.fvalue -= sattr2.fvalue;
        return;
      case dbvmMulReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.fvalue *= sattr2.fvalue;
        return;
      case dbvmDivReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (sattr2.fvalue == 0.0) {
            iattr.removeTemporaries();
            iattr.db->handleError(ArithmeticError, "Division by zero");
        } else {
            sattr.fvalue /= sattr2.fvalue;
        }
        return;
      case dbvmAbsReal:
        execute(expr->operand[0], iattr, sattr);
        if (sattr.fvalue < 0) { 
            sattr.fvalue = -sattr.fvalue;
        }
        return;
      case dbvmPowerReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (sattr.fvalue < 0) { 
            iattr.removeTemporaries();
            iattr.db->handleError(ArithmeticError, 
                                  "Power operator returns complex result");
        } else if (sattr.fvalue == 0.0 && sattr2.fvalue < 0) { 
            iattr.removeTemporaries();
            iattr.db->handleError(ArithmeticError, 
                                  "Raise zero to negative power");          
        } else { 
            sattr.fvalue = pow(sattr.fvalue, sattr2.fvalue);
        }
        return;
      case dbvmPowerRealInt:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (sattr.fvalue == 0.0 && sattr2.ivalue < 0) { 
            iattr.removeTemporaries();
            iattr.db->handleError(ArithmeticError, 
                                  "Raise zero to negative power");          
        } else { 
            sattr.fvalue = powerRealInt(sattr.fvalue, sattr2.ivalue);
        }
        return;

      case dbvmEqReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.fvalue == sattr2.fvalue;
        return;
      case dbvmNeReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.fvalue != sattr2.fvalue;
        return;
      case dbvmGtReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.fvalue > sattr2.fvalue;
        return;
      case dbvmGeReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.fvalue >= sattr2.fvalue;
        return;
      case dbvmLtReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.fvalue < sattr2.fvalue;
        return;
      case dbvmLeReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.fvalue <= sattr2.fvalue;
        return;
      case dbvmBetweenReal:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (sattr.fvalue < sattr2.fvalue) { 
            sattr.bvalue = false;
        } else { 
            execute(expr->operand[2], iattr, sattr2);
            sattr.bvalue = sattr.fvalue <= sattr2.fvalue;
        }
        return;

     case dbvmEqBinary:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(sattr.raw, sattr2.raw, expr->offs) == 0;
        return;
      case dbvmNeBinary:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(sattr.raw, sattr2.raw, expr->offs) != 0;
        return;
      case dbvmGtBinary:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(sattr.raw, sattr2.raw, expr->offs) > 0;
        return;
      case dbvmGeBinary:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(sattr.raw, sattr2.raw, expr->offs) >= 0;
        return;
      case dbvmLtBinary:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(sattr.raw, sattr2.raw, expr->offs) < 0;
        return;
      case dbvmLeBinary:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(sattr.raw, sattr2.raw, expr->offs) <= 0;
        return;
      case dbvmBetweenBinary:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if ((*(dbUDTComparator)expr->func.fptr)(sattr.raw, sattr2.raw, expr->offs) < 0) {
            sattr.bvalue = false;
        } else {
            execute(expr->operand[2], iattr, sattr2);
            sattr.bvalue = (*(dbUDTComparator)expr->func.fptr)(sattr.raw, sattr2.raw, expr->offs) <= 0;
        }
        return;

      case dbvmIntToReference:
        execute(expr->operand[0], iattr, sattr);
        sattr.oid = (oid_t)sattr.ivalue;
        return;

       case dbvmIntToReal:
        execute(expr->operand[0], iattr, sattr);
        sattr.fvalue = (real8)sattr.ivalue;
        return;
      case dbvmRealToInt:
        execute(expr->operand[0], iattr, sattr);
        sattr.ivalue = (db_int8)sattr.fvalue;
        return;

      case dbvmIntToString:
        execute(expr->operand[0], iattr, sattr);
        convertIntToString(iattr, sattr);
        return;
      case dbvmRealToString:
        execute(expr->operand[0], iattr, sattr);
        convertRealToString(iattr, sattr);
        return;
      case dbvmWcsToMbs:
        execute(expr->operand[0], iattr, sattr);
        convertWcsToMbs(iattr, sattr);
        return;
      case dbvmMbsToWcs:
        execute(expr->operand[0], iattr, sattr);
        convertMbsToWcs(iattr, sattr);
        return;
      case dbvmStringConcat:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        concatenateStrings(iattr, sattr, sattr2);
        return;
      case dbvmUpperString:
        execute(expr->operand[0], iattr, sattr);
        uppercaseString(iattr, sattr);
        return;
      case dbvmLowerString:
        execute(expr->operand[0], iattr, sattr);
        lowercaseString(iattr, sattr);
        return;
      case dbvmWStringConcat:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        concatenateWStrings(iattr, sattr, sattr2);
        return;
      case dbvmUpperWString:
        execute(expr->operand[0], iattr, sattr);
        uppercaseWString(iattr, sattr);
        return;
      case dbvmLowerWString:
        execute(expr->operand[0], iattr, sattr);
        lowercaseWString(iattr, sattr);
        return;

      case dbvmEqString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = compareStringsForEquality(sattr, sattr2);
        return;
      case dbvmNeString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = !compareStringsForEquality(sattr, sattr2);
        return;
      case dbvmGtString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = compareStrings(sattr, sattr2) > 0;
        return;
      case dbvmGeString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = compareStrings(sattr, sattr2) >= 0;
        return;
      case dbvmLtString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = compareStrings(sattr, sattr2) < 0;
        return;
      case dbvmLeString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = compareStrings(sattr, sattr2) <= 0;
        return;

      case dbvmEqWString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = compareWStringsForEquality(sattr, sattr2);
        return;
      case dbvmNeWString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = !compareWStringsForEquality(sattr, sattr2);
        return;
      case dbvmGtWString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = compareWStrings(sattr, sattr2) > 0;
        return;
      case dbvmGeWString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = compareWStrings(sattr, sattr2) >= 0;
        return;
      case dbvmLtWString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = compareWStrings(sattr, sattr2) < 0;
        return;
      case dbvmLeWString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = compareWStrings(sattr, sattr2) <= 0;
        return;

#ifdef USE_REGEX
      case dbvmMatchString:
        execute(expr->regex.opd, iattr, sattr);
        sattr.bvalue = regexec(&expr->regex.re, (char*)sattr.array.base, 0, NULL, 0) == 0;
        return;
#endif
      case dbvmLikeString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = matchStrings(sattr, sattr2);
        return;
      case dbvmLikeEscapeString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        execute(expr->operand[2], iattr, sattr3);
        sattr.bvalue = matchStrings(sattr, sattr2, *sattr3.array.base);
        return;
      case dbvmBetweenString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (compareStrings(sattr, sattr2) < 0) { 
            sattr.bvalue = false;
        } else { 
            execute(expr->operand[2], iattr, sattr2);
            sattr.bvalue = compareStrings(sattr, sattr2) <= 0;
        }
        return;

      case dbvmLikeWString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = matchWStrings(sattr, sattr2);
        return;
      case dbvmLikeEscapeWString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        execute(expr->operand[2], iattr, sattr3);
        sattr.bvalue = matchWStrings(sattr, sattr2, *sattr3.array.base);
        return;
      case dbvmBetweenWString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        if (compareWStrings(sattr, sattr2) < 0) { 
            sattr.bvalue = false;
        } else { 
            execute(expr->operand[2], iattr, sattr2);
            sattr.bvalue = compareWStrings(sattr, sattr2) <= 0;
        }
        return;

      case dbvmEqBool:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.bvalue == sattr2.bvalue;
        return;
      case dbvmNeBool:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.bvalue != sattr2.bvalue;
        return;

      case dbvmEqReference:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.oid == sattr2.oid;
        return;
      case dbvmNeReference:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        sattr.bvalue = sattr.oid != sattr2.oid;
        return;

      case dbvmDeref:
        execute(expr->operand[0], iattr, sattr);
        if (sattr.oid == 0) { 
            iattr.removeTemporaries();
            iattr.db->handleError(NullReferenceError);
        }
        FASTDB_ASSERT_EX(iattr.db, !(iattr.db->currIndex[sattr.oid] 
                 & (dbInternalObjectMarker|dbFreeHandleMarker)));
        sattr.base = iattr.db->baseAddr + iattr.db->currIndex[sattr.oid];
        return;

      case dbvmFuncArg2Bool:
        sattr.bvalue = (*(bool(*)(dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0));
        return;
      case dbvmFuncArg2Int:
        sattr.ivalue = (*(db_int8(*)(dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0));
        return;
      case dbvmFuncArg2Real:
        sattr.fvalue = (*(real8(*)(dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0));
        return;
      case dbvmFuncArg2Str:
        copyString(iattr, sattr, 
                   (*(char*(*)(dbUserFunctionArgument const&))expr->func.fptr)
                   (dbUserFunctionArgument(expr, iattr, sattr, 0)));
        return;
      case dbvmFuncArg2WStr:
        copyWString(iattr, sattr, 
                    (*(wchar_t*(*)(dbUserFunctionArgument const&))expr->func.fptr)
                    (dbUserFunctionArgument(expr, iattr, sattr, 0)));
        return;
      case dbvmFuncArgArg2Bool:
        sattr.bvalue = (*(bool(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0), 
             dbUserFunctionArgument(expr, iattr, sattr, 1));
        return;
      case dbvmFuncArgArg2Int:
        sattr.ivalue = (*(db_int8(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0), 
             dbUserFunctionArgument(expr, iattr, sattr, 1));
        return;
      case dbvmFuncArgArg2Real:
        sattr.fvalue = (*(real8(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0), 
             dbUserFunctionArgument(expr, iattr, sattr, 1));
        return;
      case dbvmFuncArgArg2Str:
        copyString(iattr, sattr, 
                   (*(char*(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
                   (dbUserFunctionArgument(expr, iattr, sattr, 0),
                    dbUserFunctionArgument(expr, iattr, sattr, 1)));
        return;
      case dbvmFuncArgArg2WStr:
        copyWString(iattr, sattr, 
                    (*(wchar_t*(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
                    (dbUserFunctionArgument(expr, iattr, sattr, 0),
                     dbUserFunctionArgument(expr, iattr, sattr, 1)));
        return;
      case dbvmFuncArgArgArg2Bool:
        sattr.bvalue = (*(bool(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0),
             dbUserFunctionArgument(expr, iattr, sattr, 1), 
             dbUserFunctionArgument(expr, iattr, sattr, 2));
        return;
      case dbvmFuncArgArgArg2Int:
        sattr.ivalue = (*(db_int8(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0), 
             dbUserFunctionArgument(expr, iattr, sattr, 1), 
             dbUserFunctionArgument(expr, iattr, sattr, 2));
        return;
      case dbvmFuncArgArgArg2Real:
        sattr.fvalue = (*(real8(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
            (dbUserFunctionArgument(expr, iattr, sattr, 0), 
             dbUserFunctionArgument(expr, iattr, sattr, 1),
             dbUserFunctionArgument(expr, iattr, sattr, 2));
        return;
      case dbvmFuncArgArgArg2Str:
        copyString(iattr, sattr, 
                   (*(char*(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
                   (dbUserFunctionArgument(expr, iattr, sattr, 0), 
                    dbUserFunctionArgument(expr, iattr, sattr, 1), 
                    dbUserFunctionArgument(expr, iattr, sattr, 2)));
        return;
      case dbvmFuncArgArgArg2WStr:
        copyWString(iattr, sattr, 
                    (*(wchar_t*(*)(dbUserFunctionArgument const&, dbUserFunctionArgument const&, dbUserFunctionArgument const&))expr->func.fptr)
                    (dbUserFunctionArgument(expr, iattr, sattr, 0), 
                     dbUserFunctionArgument(expr, iattr, sattr, 1), 
                     dbUserFunctionArgument(expr, iattr, sattr, 2)));
        return;



      case dbvmFuncInt2Bool:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.bvalue = (*(bool(*)(db_int8))expr->func.fptr)(sattr.ivalue);
        return;
      case dbvmFuncReal2Bool:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.bvalue = (*(bool(*)(real8))expr->func.fptr)(sattr.fvalue);
        return;
      case dbvmFuncStr2Bool:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.bvalue = 
            (*(bool(*)(char const*))expr->func.fptr)(sattr.array.base);
        return;
      case dbvmFuncWStr2Bool:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.bvalue = 
            (*(bool(*)(wchar_t const*))expr->func.fptr)((wchar_t*)sattr.array.base);
        return;
      case dbvmFuncInt2Int:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.ivalue = (*(db_int8(*)(db_int8))expr->func.fptr)(sattr.ivalue);
        return;
      case dbvmFuncReal2Int:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.ivalue = (*(db_int8(*)(real8))expr->func.fptr)(sattr.fvalue);
        return;
      case dbvmFuncStr2Int:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.ivalue = 
            (*(db_int8(*)(char const*))expr->func.fptr)(sattr.array.base);
        return;
      case dbvmFuncWStr2Int:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.ivalue = 
            (*(db_int8(*)(wchar_t const*))expr->func.fptr)((wchar_t*)sattr.array.base);
        return;
      case dbvmFuncInt2Real:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.fvalue = (*(real8(*)(db_int8))expr->func.fptr)(sattr.ivalue);
        return;
      case dbvmFuncReal2Real:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.fvalue = (*(real8(*)(real8))expr->func.fptr)(sattr.fvalue);
        return;
      case dbvmFuncStr2Real:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.fvalue = 
            (*(real8(*)(char const*))expr->func.fptr)(sattr.array.base);
        return;
      case dbvmFuncWStr2Real:
        execute(expr->func.arg[0], iattr, sattr);
        sattr.fvalue = 
            (*(real8(*)(wchar_t const*))expr->func.fptr)((wchar_t*)sattr.array.base);
        return;
      case dbvmFuncInt2Str:
        execute(expr->func.arg[0], iattr, sattr);
        copyString(iattr, sattr, 
                   (*(char*(*)(db_int8))expr->func.fptr)(sattr.ivalue));
        return;
      case dbvmFuncReal2Str:
        execute(expr->func.arg[0], iattr, sattr);
        copyString(iattr, sattr, 
                   (*(char*(*)(real8))expr->func.fptr)(sattr.fvalue));
        return;
      case dbvmFuncStr2Str:
        execute(expr->func.arg[0], iattr, sattr);
        copyString(iattr, sattr, 
                  (*(char*(*)(char const*))expr->func.fptr)(sattr.array.base));
        return;
      case dbvmFuncWStr2Str:
        execute(expr->func.arg[0], iattr, sattr);
        copyString(iattr, sattr, 
                  (*(char*(*)(wchar_t const*))expr->func.fptr)((wchar_t*)sattr.array.base));
        return;

      case dbvmFuncInt2WStr:
        execute(expr->func.arg[0], iattr, sattr);
        copyWString(iattr, sattr, 
                   (*(wchar_t*(*)(db_int8))expr->func.fptr)(sattr.ivalue));
        return;
      case dbvmFuncReal2WStr:
        execute(expr->func.arg[0], iattr, sattr);
        copyWString(iattr, sattr, 
                    (*(wchar_t*(*)(real8))expr->func.fptr)(sattr.fvalue));
        return;
      case dbvmFuncStr2WStr:
        execute(expr->func.arg[0], iattr, sattr);
        copyWString(iattr, sattr, 
                    (*(wchar_t*(*)(char const*))expr->func.fptr)(sattr.array.base));
        return;
      case dbvmFuncWStr2WStr:
        execute(expr->func.arg[0], iattr, sattr);
        copyWString(iattr, sattr, 
                    (*(wchar_t*(*)(wchar_t const*))expr->func.fptr)((wchar_t*)sattr.array.base));
        return;

      case dbvmInArrayBool:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfBool(sattr, sattr2);
        return;
      case dbvmInArrayInt1:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfInt1(sattr, sattr2);
        return;
      case dbvmInArrayInt2:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfInt2(sattr, sattr2);
        return;
      case dbvmInArrayInt4:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfInt4(sattr, sattr2);
        return;
      case dbvmInArrayInt8:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfInt8(sattr, sattr2);
        return;
      case dbvmInArrayReal4:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfReal4(sattr, sattr2);
        return;
      case dbvmInArrayReal8:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfReal8(sattr, sattr2);
        return;
      case dbvmInArrayString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfString(sattr, sattr2);
        return;
      case dbvmInArrayWString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfWString(sattr, sattr2);
        return;
      case dbvmInArrayReference:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfReference(sattr, sattr2);
        return;
      case dbvmInArrayRectangle:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchArrayOfRectangle(iattr, sattr, sattr2);
        return;
      case dbvmInString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchInString(sattr, sattr2);
        return;
      case dbvmInWString:
        execute(expr->operand[0], iattr, sattr);
        execute(expr->operand[1], iattr, sattr2);
        searchInWString(sattr, sattr2);
        return;

      case dbvmList:
        return;

      default:
        FASTDB_ASSERT_EX(iattr.db, false);
    }
}

char const* const dbDatabase::errorMessage[] =
{
    "No error", 
    "Query syntax error",
    "Arithmetic exception",
    "Index out of range",
    "Database open error",
    "File access error",
    "Out of memory",
    "Deadlock",
    "Null reference",
    "Lock revoked",
    "File limit exeeded",
    "Inconsistent inverse reference",
    "Attempt to modify read-only database",
    "Assertion failed",
    "Access to deleted object",
    "No current record",
    "Cursor is read-only",
    "Incompatible schema change",
    "Operation is rejected by transaction logger"
};

#ifdef _WIN32
#define snprintf _snprintf
#endif

void dbDatabase::formatErrorMessage(char* buf, size_t bufSize, dbErrorClass error, char const* msg, int arg)
{
    switch (error) { 
      case QueryError:
        snprintf(buf, bufSize, "%s in position %d", msg, arg);
        break;
      case ArithmeticError:
        snprintf(buf, bufSize, "%s", msg);
        break;
      case IndexOutOfRangeError:
        snprintf(buf, bufSize, "Index %d is out of range", arg);
        break;
      case DatabaseOpenError:
        snprintf(buf, bufSize, "%s", msg);
        break;
      case FileError:
        snprintf(buf, bufSize, "%s: %s", msg, dbFile::errorText(arg, buf, sizeof(buf)));
        break;
      case OutOfMemoryError:
        snprintf(buf, bufSize,"Not enough memory: failed to allocate %d bytes", arg);
        break;
      case NullReferenceError:
        snprintf(buf, bufSize, "Null object reference is accessed");
        break;
      case Deadlock:
        snprintf(buf, bufSize, "Deadlock is caused by upgrading shared locks to exclusive");
        break;
      case LockRevoked:
        snprintf(buf, bufSize, "Lock is revoked by some other client");
        break;  
      case InconsistentInverseReference:
        snprintf(buf, bufSize, "%s", msg);
        break;
      case DatabaseReadOnly:
        snprintf(buf, bufSize, "Attempt to modify readonly database");
        break;
      case AssertionFailed:
        snprintf(buf, bufSize, "Assertion failed %s at line %d", msg, arg);
        break;
      default:
        snprintf(buf, bufSize, "Error %d: %s", error, msg);
    }
}

void dbDatabase::fatalError()
{
    abort();
}

void dbDatabase::handleError(dbErrorClass error, char const* msg, int arg)
{
    if (errorHandler != NULL) { 
        (*errorHandler)(error, msg, arg, errorHandlerContext);
    }
#ifdef THROW_EXCEPTION_ON_ERROR
    if (error != NoError) {
        if (msg == NULL) { 
            msg = errorMessage[error];
        }
        if (error == DatabaseOpenError || (error == InconsistentInverseReference && dbTableDescriptor::chain == NULL)) {
            fprintf(stderr, "%s\n", msg);
        } else { 
            throw dbException(error, msg, arg);
        }
    }
#else
    char buf[256];
    switch (error) { 
      case QueryError:
        fprintf(stderr, "%s in position %d\n", msg, arg);
        return;
      case ArithmeticError:
        fprintf(stderr, "%s\n", msg);
        break;
      case IndexOutOfRangeError:
        fprintf(stderr, "Index %d is out of range\n", arg);
        break;
      case DatabaseOpenError:
        fprintf(stderr, "%s\n", msg);
        return;
      case FileError:
        fprintf(stderr, "%s: %s\n", msg, 
                dbFile::errorText(arg, buf, sizeof(buf)));
        break;
      case OutOfMemoryError:
        fprintf(stderr,"Not enough memory: failed to allocate %d bytes\n",arg);
        break;
      case NullReferenceError:
        fprintf(stderr, "Null object reference is accessed\n");
        break;
      case Deadlock:
        fprintf(stderr, "Deadlock is caused by upgrading shared locks to exclusive");
        break;
      case LockRevoked:
        fprintf(stderr, "Lock is revoked by some other client\n");
        break;  
      case InconsistentInverseReference:
        fprintf(stderr, "%s\n", msg);
        break;
      case DatabaseReadOnly:
        fprintf(stderr, "Attempt to modify readonly database\n");
        break;
      case AssertionFailed:
        fprintf(stderr, "Assertion failed %s at line %d\n", error, arg);
        break;
      case RejectedByTransactionLogger:
        fprintf(stderr, "Operation is rejected by transaction logger\n");
        break;
      default:
        return;
    }   
    fflush(stderr);
    fatalError();
#endif
}

bool dbDatabase::isReplicated()
{
    return false;
}

void dbDatabase::initializeMetaTable()
{
    static struct { 
        char const* name;
        int         type;
        int         size;
        int         offs;
    } metaTableFields[] = { 
        { "name", dbField::tpString, sizeof(dbVarying), 
          offsetof(dbTable, name)},
        { "fields", dbField::tpArray, sizeof(dbVarying), 
          offsetof(dbTable, fields)},
        { "fields[]", dbField::tpStructure, sizeof(dbField), 0},
        { "fields[].name", dbField::tpString, sizeof(dbVarying), 
          offsetof(dbField, name)},
        { "fields[].tableName",dbField::tpString,sizeof(dbVarying), 
          offsetof(dbField, tableName)},
        { "fields[].inverse", dbField::tpString, sizeof(dbVarying), 
          offsetof(dbField, inverse)},
//        { "fields[].type", dbField::tpInt4, 4, offsetof(dbField, type)},
        { "fields[].type", dbField::tpInt4, 4, offsetof(dbField, offset)-4},
        { "fields[].offset", dbField::tpInt4, 4, offsetof(dbField, offset)},
        { "fields[].size", dbField::tpInt4, 4, offsetof(dbField, size)},
        { "fields[].hashTable", dbField::tpReference, sizeof(oid_t), 
          offsetof(dbField, hashTable)},
        { "fields[].tTree", dbField::tpReference, sizeof(oid_t), 
          offsetof(dbField, tTree)},
        { "fixedSize", dbField::tpInt4, 4, offsetof(dbTable, fixedSize)},
        { "nRows", dbField::tpInt4, 4, offsetof(dbTable, nRows)},
        { "nColumns", dbField::tpInt4, 4, offsetof(dbTable, nColumns)},
        { "firstRow", dbField::tpReference, sizeof(oid_t), offsetof(dbTable, firstRow)},
        { "lastRow", dbField::tpReference, sizeof(oid_t), offsetof(dbTable, lastRow)}
 #ifdef AUTOINCREMENT_SUPPORT
       ,{ "count", dbField::tpInt4, 4, offsetof(dbTable, count)}
 #endif
    };

    unsigned i;
    size_t varyingSize = strlen(dbMetaTableName)+1;
    for (i = 0; i < itemsof(metaTableFields); i++) { 
        varyingSize += strlen(metaTableFields[i].name) + 3;
        
    }
    offs_t metaTableOffs = allocate(sizeof(dbTable)
                                    + sizeof(dbField)*itemsof(metaTableFields)
                                    + varyingSize);
    index[0][dbMetaTableId] = metaTableOffs;
    dbTable* table = (dbTable*)(baseAddr + metaTableOffs);
    table->size = (nat4)(sizeof(dbTable) + sizeof(dbField)*itemsof(metaTableFields)
                + varyingSize);
    table->next = table->prev = 0;
    int offs = sizeof(dbTable) + sizeof(dbField)*itemsof(metaTableFields);
    table->name.offs = offs;
    table->name.size = (nat4)strlen(dbMetaTableName)+1;
    strcpy((char*)table + offs, dbMetaTableName);
    offs += table->name.size;
    table->fields.offs = sizeof(dbTable);
    table->fields.size = itemsof(metaTableFields);
    table->fixedSize = sizeof(dbTable);
    table->nRows = 0;
    table->nColumns = 5;
    table->firstRow = 0;
    table->lastRow = 0;
#ifdef AUTOINCREMENT_SUPPORT
    table->count = 0;
#endif

    dbField* field = (dbField*)((char*)table + table->fields.offs);
    offs -= sizeof(dbTable);
    for (i = 0; i < itemsof(metaTableFields); i++) { 
        field->name.offs = offs;
        field->name.size = (nat4)strlen(metaTableFields[i].name) + 1;
        strcpy((char*)field + offs, metaTableFields[i].name);
        offs += field->name.size;

        field->tableName.offs = offs;
        field->tableName.size = 1;
        *((char*)field + offs++) = '\0';
        
        field->inverse.offs = offs;
        field->inverse.size = 1;
        *((char*)field + offs++) = '\0';
        
        field->flags = 0;
        field->type = metaTableFields[i].type;
        field->size = metaTableFields[i].size;
        field->offset = metaTableFields[i].offs;
        field->hashTable = 0;
        field->tTree = 0;
        field += 1;
        offs -= sizeof(dbField);
    }
}

void dbDatabase::cleanup(dbInitializationMutex::initializationStatus status, int step) 
{
    switch (step) { 
      case 9:
        if (status == dbInitializationMutex::NotYetInitialized) {         
            file.close();
        }
        // no break
      case 8:
        if (accessType == dbConcurrentUpdate || accessType == dbConcurrentRead) { 
            mutatorCS.close();
        }
        // no break
      case 7:
        cs.close();
        // no break
      case 6:
        if (delayedCommitEventsOpened) { 
            delayedCommitStopTimerEvent.close();
            delayedCommitStartTimerEvent.close();
            commitThreadSyncEvent.close();
            delayedCommitEventsOpened = false;
        }
        backupInitEvent.close();
        delete threadContext.get();    
        // no break
      case 5:
        backupCompletedEvent.close();
        // no break
      case 4:
        upgradeSem.close();
        // no break
      case 3:
        readSem.close();
        // no break
      case 2:
        writeSem.close();
        // no break
      case 1:
        shm.close();
        // no break
      default:
        if (status == dbInitializationMutex::NotYetInitialized) {         
            initMutex.done();
        }
        initMutex.close();
    }
}

bool dbDatabase::open(OpenParameters& params)
{
    accessType = params.accessType;
    fileOpenFlags = params.fileOpenFlags;
    if (accessType == dbReadOnly || accessType == dbConcurrentRead) { 
        fileOpenFlags |= dbFile::read_only;
    }
    extensionQuantum = params.extensionQuantum;
    initIndexSize = params.initIndexSize; 
    initSize = params.initSize;
    freeSpaceReuseThreshold = params.freeSpaceReuseThreshold;
    parallelScanThreshold = params.parallelScanThreshold;
    setConcurrency(params.nThreads);
    return open(params.databaseName, params.databaseFilePath, params.waitLockTimeoutMsec, params.transactionCommitDelay);
}

bool dbDatabase::open(char_t const* dbName, char_t const* fiName, 
                      time_t waitLockTimeoutMsec, time_t commitDelaySec)
{
    waitLockTimeout = (unsigned)waitLockTimeoutMsec;
    delete[] databaseName;
    delete[] fileName;
    commitDelay = 0;
    commitTimeout = 0;
    commitTimerStarted = 0;
    delayedCommitEventsOpened = false;
    backupFileName = NULL;
    backupPeriod = 0;
    opened = false;
    logger = NULL;
    stopDelayedCommitThread = false;
    memset(tableHash, 0, sizeof tableHash);
    databaseNameLen = (int)_tcslen(dbName);
    char_t* name = new char_t[databaseNameLen+16];
    _stprintf(name, _T("%s.in"), dbName);
    databaseName = name;
    if (fiName == NULL) { 
        fileName = new char_t[databaseNameLen + 5];
        _stprintf(fileName, _T("%s.fdb"), dbName);
    } else { 
        fileName = new char_t[_tcslen(fiName)+1];
        _stprintf(fileName, fiName);
    }

    dbInitializationMutex::initializationStatus status = initMutex.initialize(name);
    if (status == dbInitializationMutex::InitializationError) { 
        handleError(DatabaseOpenError, 
                    "Failed to start database initialization");
        return false;
    }
    _stprintf(name, _T("%s.dm"), dbName);
    if (!shm.open(name)) { 
        handleError(DatabaseOpenError, "Failed to open database monitor");
        cleanup(status, 0);
        return false;
    }
    monitor = shm.get();
    _stprintf(name, _T("%s.ws"), dbName);
    if (!writeSem.open(name)) { 
        handleError(DatabaseOpenError, 
                    "Failed to initialize database writers semaphore");
        cleanup(status, 1);
        return false;
    }
    _stprintf(name, _T("%s.rs"), dbName);
    if (!readSem.open(name)) { 
        handleError(DatabaseOpenError, 
                    "Failed to initialize database readers semaphore");
        cleanup(status, 2);
        return false;
    }
    _stprintf(name, _T("%s.us"), dbName);
    if (!upgradeSem.open(name)) { 
        handleError(DatabaseOpenError, 
                    "Failed to initialize database upgrade semaphore");
        cleanup(status, 3);
        return false;
    }
    _stprintf(name, _T("%s.bce"), dbName);
    if (!backupCompletedEvent.open(name)) { 
        handleError(DatabaseOpenError, 
                    "Failed to initialize database backup completed event");
        cleanup(status, 4);
        return false;
    }    
    if (commitDelaySec != 0) { 
        _stprintf(name, _T("%s.dce"), dbName);
        delayedCommitEventsOpened = true;
        if (!delayedCommitStopTimerEvent.open(name)) { 
            handleError(DatabaseOpenError, 
                        "Failed to initialize delayed commit event");
            cleanup(status, 5);
            return false;
        }    
        delayedCommitStartTimerEvent.open();
        commitThreadSyncEvent.open();
    }
    backupInitEvent.open();
    backupFileName = NULL;

    fixedSizeAllocator.reset();
    allocatedSize = 0;
    deallocatedSize = 0;
    size_t indexSize = initIndexSize < dbFirstUserId 
        ? size_t(dbFirstUserId) : initIndexSize;
    indexSize = DOALIGN(indexSize, dbHandlesPerPage);
            
    size_t fileSize = initSize ? initSize : dbDefaultInitDatabaseSize;

    if (fileSize < indexSize*sizeof(offs_t)*4) {
        fileSize = indexSize*sizeof(offs_t)*4;
    }
    fileSize = DOALIGN(fileSize, dbBitmapSegmentSize);

    for (int i = dbBitmapId + dbBitmapPages; --i >= 0;) { 
        bitmapPageAvailableSpace[i] = INT_MAX;
    }
    currRBitmapPage = currPBitmapPage = dbBitmapId;
    currRBitmapOffs = currPBitmapOffs = 0;
    bitmapEnd = dbBitmapId;
    reservedChain = NULL;
    reservedChainLength = 0;
    tables = NULL;
    modified = false;
    selfId = 0;
    maxClientId = 0;
    threadContextList.reset();
    attach();

    if (status == dbInitializationMutex::NotYetInitialized) { 
        _stprintf(name, _T("%s.cs"), dbName);
        if (!cs.create(name, &monitor->sem)) { 
            handleError(DatabaseOpenError, "Failed to initialize database monitor");
            cleanup(status, 6);
            return false;
        }
        if (accessType == dbConcurrentUpdate || accessType == dbConcurrentRead) { 
            _stprintf(name, _T("%s.mcs"), dbName);
            if (!mutatorCS.create(name, &monitor->mutatorSem)) { 
                handleError(DatabaseOpenError,
                            "Failed to initialize database monitor");
                cleanup(status, 7);
                return false;
            }
        }
        readSem.reset();
        writeSem.reset();
        upgradeSem.reset();
        monitor->nReaders = 0;
        monitor->nWriters = 0;
        monitor->nWaitReaders = 0;
        monitor->nWaitWriters = 0;
        monitor->waitForUpgrade = false;
        monitor->version = version = 1;
        monitor->users = 0;
        monitor->backupInProgress = 0;
        monitor->forceCommitCount = 0;
        monitor->lastDeadlockRecoveryTime = 0;
        monitor->delayedCommitContext = NULL;
        monitor->concurrentTransId = 1;
        monitor->commitInProgress = false;
        monitor->uncommittedChanges = false;
        monitor->clientId = 0;
        monitor->upgradeId = 0;
        monitor->modified = false;
        monitor->exclusiveLockOwner = 0;
        memset(monitor->dirtyPagesMap, 0, dbDirtyPageBitmapSize);
        memset(monitor->sharedLockOwner, 0, sizeof(monitor->sharedLockOwner));
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
        monitor->sessionFreeList[0].head = monitor->sessionFreeList[0].tail = 0;
        monitor->sessionFreeList[1].head = monitor->sessionFreeList[1].tail = 0;
#endif            
    
        _stprintf(databaseName, _T("%s.%d"), dbName, version);
        int rc = file.open(fileName, databaseName, fileOpenFlags, fileSize, false);
        if (rc != dbFile::ok)
        {
            char msgbuf[64];
            file.errorText(rc, msgbuf, sizeof msgbuf);
            TRACE_MSG(("File open error: %s\n", msgbuf));
            handleError(DatabaseOpenError, "Failed to create database file");
            cleanup(status, 8);
            return false;
        }
        baseAddr = (byte*)file.getAddr();
        monitor->size = (offs_t)(fileSize = file.getSize());
        header = (dbHeader*)baseAddr;        
        updatedRecordId = 0;
        
        if ((unsigned)header->curr > 1) { 
            handleError(DatabaseOpenError, "Database file was corrupted: "
                        "invalid root index");
            cleanup(status, 9);
            return false;
        }
        if (header->initialized != 1) {
            if (accessType == dbReadOnly || accessType == dbConcurrentRead) { 
                handleError(DatabaseOpenError, "Can not open uninitialized "
                            "file in read only mode");
                cleanup(status, 9);
                return false;
            }
            monitor->curr = header->curr = 0;
            header->size = (offs_t)fileSize;
            size_t used = dbPageSize;
            header->root[0].index = (offs_t)used;
            header->root[0].indexSize = (oid_t)indexSize;
            header->root[0].indexUsed = dbFirstUserId;
            header->root[0].freeList = 0;
            used += indexSize*sizeof(offs_t);
            header->root[1].index = (offs_t)used;
            header->root[1].indexSize = (oid_t)indexSize;
            header->root[1].indexUsed = dbFirstUserId;
            header->root[1].freeList = 0;
            used += indexSize*sizeof(offs_t);

            header->root[0].shadowIndex = header->root[1].index;
            header->root[1].shadowIndex = header->root[0].index;
            header->root[0].shadowIndexSize = (oid_t)indexSize;
            header->root[1].shadowIndexSize = (oid_t)indexSize;
            
            header->majorVersion= FASTDB_MAJOR_VERSION;
            header->minorVersion = FASTDB_MINOR_VERSION;
            header->mode = header->getCurrentMode();
            header->used = used;

            index[0] = (offs_t*)(baseAddr + header->root[0].index);
            index[1] = (offs_t*)(baseAddr + header->root[1].index);
            index[0][dbInvalidId] = dbFreeHandleMarker;

            size_t bitmapPages = 
                (used + dbPageSize*(dbAllocationQuantum*8-1) - 1)
                / (dbPageSize*(dbAllocationQuantum*8-1));
            memset(baseAddr+used, 0xFF, (used + bitmapPages*dbPageSize)
                                        / (dbAllocationQuantum*8));
            memset(baseAddr + used + (used + bitmapPages*dbPageSize) / (dbAllocationQuantum*8), 0, 
                   bitmapPages*dbPageSize - (used + bitmapPages*dbPageSize) / (dbAllocationQuantum*8));
            size_t i;
            for (i = 0; i < bitmapPages; i++) { 
                index[0][dbBitmapId + i] = (offs_t)(used + dbPageObjectMarker);
                used += dbPageSize;
            }
            while (i < dbBitmapPages) { 
                index[0][dbBitmapId+i] = dbFreeHandleMarker;
                i += 1;
            }
            currIndex = index[0];
            currIndexSize = dbFirstUserId;
            committedIndexSize = 0;
            initializeMetaTable();
            header->dirty = true;
            memcpy(index[1], index[0], currIndexSize*sizeof(offs_t));
            file.markAsDirty(0, used);
            file.flush(true);
            header->initialized = true;
            file.markAsDirty(0, sizeof(dbHeader));
            file.flush(true);
        } else {
            if (!header->isCompatible()) { 
                handleError(DatabaseOpenError, "Incompatible database mode");
                cleanup(status, 9);
                return false;
            }        
            monitor->curr = header->curr;
            if (header->dirty) { 
                TRACE_MSG(("Database was not normally closed: start recovery\n"));
                if (accessType == dbReadOnly || accessType == dbConcurrentRead) { 
                    handleError(DatabaseOpenError,
                                "Can not open dirty file in read only mode");
                    cleanup(status, 9);
                    return false;
                }
                recovery();
                TRACE_MSG(("Recovery completed\n"));
            } else { 
                if (file.getSize() != header->size) { 
                    handleError(DatabaseOpenError, "Database file was "
                                "corrupted: file size in header differs "
                                "from actual file size");
                    cleanup(status, 9);
                    return false;
                }
            }       
        }
    } else { 
        _stprintf(name, _T("%s.cs"), dbName);
        if (!cs.open(name, &monitor->sem)) { 
            handleError(DatabaseOpenError, "Failed to open shared semaphore");
            cleanup(status, 6);
            return false;
        }
        if (accessType == dbConcurrentUpdate || accessType == dbConcurrentRead) { 
            _stprintf(name, _T("%s.mcs"), dbName);
            if (!mutatorCS.open(name, &monitor->mutatorSem)) { 
                handleError(DatabaseOpenError, "Failed to open shared semaphore");
                cleanup(status, 7);
                return false;
            }
        }
        version = 0;
    }
    cs.enter();            
    monitor->users += 1;
    selfId = ++monitor->clientId;
#ifdef AUTO_DETECT_PROCESS_CRASH
    _stprintf(databaseName + databaseNameLen, _T(".pid.%ld"), selfId);
    selfWatchDog.create(databaseName);
    watchDogMutex = new dbMutex();
#endif
    cs.leave();

    if (status == dbInitializationMutex::NotYetInitialized) { 
        if (!loadScheme(true)) { 
            cleanup(status, 9);
            return false;
        }
        initMutex.done();
    } else { 
        if (!loadScheme(false)) { 
            cleanup(status, 9);
            return false;
        }
    }
    opened = true;

    if (commitDelaySec != 0) { 
        dbCriticalSection cs(delayedCommitStartTimerMutex);
        commitTimeout = commitDelay = commitDelaySec;
        commitThread.create((dbThread::thread_proc_t)delayedCommitProc, this);
        commitThreadSyncEvent.wait(delayedCommitStartTimerMutex);
    }
    return true;
}

void dbDatabase::scheduleBackup(char_t const* fileName, time_t period)
{
    if (backupFileName == NULL) { 
        backupFileName = new char_t[_tcslen(fileName) + 1];
        _tcscpy(backupFileName, fileName);
        backupPeriod = period;
        backupThread.create((dbThread::thread_proc_t)backupSchedulerProc, this);
    }
}
 
void dbDatabase::backupScheduler() 
{ 
    backupThread.setPriority(dbThread::THR_PRI_LOW);
    attach();
    {
        dbCriticalSection cs(backupMutex); 
        while (true) { 
            if (!opened || backupFileName == NULL) { 
                break;
            }
            time_t timeout = backupPeriod;
            if (backupFileName[_tcslen(backupFileName)-1] != '?') {
#if defined(_WINCE) || defined(_WIN32)
                WIN32_FIND_DATA lData;
                HANDLE lFile = ::FindFirstFile(backupFileName, &lData);
                FILETIME lATime;
                if (::GetFileTime(lFile, 0l, &lATime, 0l) == TRUE)
                {
                    ULARGE_INTEGER lNTime = *(ULARGE_INTEGER*)&lATime;                
                    time_t howOld = time(NULL) - *(time_t*)&lNTime;
                    if (timeout < howOld) { 
                        timeout = 0;
                    } else { 
                        timeout -= howOld;
                    }
                }
                ::FindClose(lFile);
#else    
                STATSTRUCT st;
                if (::_tstat(backupFileName, &st) == 0) { 
                    time_t howOld = time(NULL) - st.st_atime;
                    if (timeout < howOld) { 
                        timeout = 0;
                    } else { 
                        timeout -= howOld;
                    }
                }
#endif
            }
        
            backupInitEvent.wait(backupMutex, timeout*1000);
            
            if (backupFileName != NULL) { 
                if (backupFileName[_tcslen(backupFileName)-1] == _T('?')) {
                    time_t currTime = time(NULL);
                    char_t* fileName = new char_t[_tcslen(backupFileName) + 32];
                    struct tm* t = localtime(&currTime);
                    _stprintf(fileName, _T("%.*s-%04d.%02d.%02d_%02d.%02d.%02d"), 
                              (int)_tcslen(backupFileName)-1, backupFileName,
                              t->tm_year + 1900, t->tm_mon+1, t->tm_mday, 
                              t->tm_hour, t->tm_min, t->tm_sec);
                    backup(fileName, false);
                    delete[] fileName;
                } else { 
                    char_t* newFileName = new char_t[_tcslen(backupFileName) + 5];
                    _stprintf(newFileName,_T("%s.new"), backupFileName);
                    backup(newFileName, false);
#ifdef _WINCE
                    DeleteFile(backupFileName);
                    MoveFile(newFileName, backupFileName);
#else
                    ::_tremove(backupFileName);
                    ::_trename(newFileName, backupFileName);
#endif
                    delete[] newFileName;
                }
            } else { 
                break;
            }
        }
    }
    detach(DESTROY_CONTEXT);
}    


void dbDatabase::recovery()
{
    int curr = header->curr;
    header->size = (offs_t)file.getSize();
    header->root[1-curr].indexUsed = header->root[curr].indexUsed; 
    header->root[1-curr].freeList = header->root[curr].freeList; 
    header->root[1-curr].index = header->root[curr].shadowIndex;
    header->root[1-curr].indexSize = 
        header->root[curr].shadowIndexSize;
    header->root[1-curr].shadowIndex = header->root[curr].index;
    header->root[1-curr].shadowIndexSize = 
        header->root[curr].indexSize;
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
    monitor->sessionFreeList[1-curr] = monitor->sessionFreeList[curr];
#endif
    
    offs_t* dst = (offs_t*)(baseAddr+header->root[1-curr].index);
    offs_t* src = (offs_t*)(baseAddr+header->root[curr].index);
    currIndex = dst;
    for (oid_t i = 0, n = header->root[curr].indexUsed; i < n; i++) { 
        if (dst[i] != src[i]) { 
            dst[i] = src[i];
            file.markAsDirty(header->root[1-curr].index + i*sizeof(offs_t), sizeof(offs_t));
        }
    }
    //
    // Restore consistency of table rows l2-list 
    //
    restoreTablesConsistency();
    file.markAsDirty(0, sizeof(dbHeader));
}

void dbDatabase::restoreTablesConsistency()
{
    dbTable* table = (dbTable*)getRow(dbMetaTableId);
    oid_t lastId = table->lastRow;
    if (lastId != 0) { 
        dbRecord* record = getRow(lastId);
        if (record->next != 0) { 
            record->next = 0;
            file.markAsDirty(currIndex[lastId], sizeof(dbRecord));
        }
    }
    oid_t tableId = table->firstRow;
    while (tableId != 0) { 
        table = (dbTable*)getRow(tableId);
        lastId = table->lastRow;
        if (lastId != 0) { 
            dbRecord* record = getRow(lastId);
            if (record->next != 0) { 
                record->next = 0;
                file.markAsDirty(currIndex[lastId], sizeof(dbRecord));
            }
        }
        tableId = table->next;
    }
}

void dbDatabase::setConcurrency(unsigned nThreads)
{
    if (nThreads == 0) { // autodetect number of processors
        nThreads = dbThread::numberOfProcessors();
    } 
    if (nThreads > dbMaxParallelSearchThreads) { 
        nThreads = dbMaxParallelSearchThreads;
    }
    parThreads = nThreads;
}


bool dbDatabase::loadScheme(bool alter) 
{
    if (!beginTransaction((alter && accessType != dbReadOnly && accessType != dbConcurrentRead) 
                          || accessType == dbConcurrentUpdate
                          ? dbExclusiveLock : dbSharedLock)) 
    { 
        return false;
    }

    dbTable* metaTable = (dbTable*)getRow(dbMetaTableId);
    oid_t first = metaTable->firstRow;
    oid_t last = metaTable->lastRow;
    int nTables = metaTable->nRows;
    oid_t tableId = first;

    if (dbTableDescriptor::chain != NULL) { 
        dbTableDescriptor *desc, *next;
        dbCriticalSection cs(dbTableDescriptor::getChainMutex());
        for (desc = dbTableDescriptor::chain; desc != NULL; desc = next) {
            next = desc->next;
            if (desc->db != NULL && desc->db != DETACHED_TABLE && desc->db != this) { 
                continue;
            }
            if (desc->db == DETACHED_TABLE) {
                desc = desc->clone();
            }
            desc->db = this;
            dbFieldDescriptor* fd;
            for (fd = desc->firstField; fd != NULL; fd = fd->nextField) {
                fd->tTree = 0;
                fd->hashTable = 0;
                fd->attr &= ~dbFieldDescriptor::Updated;
            }

            int n = nTables;
            while (--n >= 0) {
                dbTable* table = (dbTable*)getRow(tableId);
                if (table == NULL) { 
                    handleError(DatabaseOpenError, "Database scheme is corrupted");
                    return false;
                }
                oid_t next = table->next;
                if (strcmp(desc->name, (char*)table + table->name.offs) == 0) {
                    if (!desc->equal(table)) { 
                        if (!alter) { 
                            handleError(DatabaseOpenError, "Incompatible class"
                                        " definition in application");
                            return false;
                        }
                        beginTransaction(dbExclusiveLock);
                        modified = true;
                        if (table->nRows == 0) { 
                            TRACE_MSG(("Replace definition of table '%s'\n",  desc->name));
                            desc->match(table, true, true);
                            updateTableDescriptor(desc, tableId);
                        } else { 
                            reformatTable(tableId, desc);
                        } 
                    } else { 
                        linkTable(desc, tableId);
                    }
                    desc->setFlags();
                    break;
                }
                if (tableId == last) {
                    tableId = first;
                } else { 
                    tableId = next;
                }
            }
            if (n < 0) { // no match found
                if (accessType == dbReadOnly || accessType == dbConcurrentRead) { 
                    TRACE_IMSG(("Table '%s' can not be added to the read-only database\n", desc->name));
                    handleError(DatabaseOpenError, "New table definition can not "
                            "be added to read only database");
                    return false;
                } else {     
                    TRACE_MSG(("Create new table '%s' in database\n", desc->name));
                    addNewTable(desc);
                    modified = true;
                }
            }
            if (accessType != dbReadOnly && accessType != dbConcurrentRead) { 
                if (!addIndices(alter, desc)) { 
                    handleError(DatabaseOpenError, "Failed to alter indices with"
                                " active applications");
                    rollback();
                    return false;
                }
            }
        }   
        for (desc = tables; desc != NULL; desc = desc->nextDbTable) {
            if (desc->cloneOf != NULL) {
                for (dbFieldDescriptor *fd = desc->firstField; fd != NULL; fd = fd->nextField)
                {
                    if (fd->refTable != NULL) {
                        fd->refTable = lookupTable(fd->refTable);
                    }
                }
            }
            desc->checkRelationship();
        }
    }
    commit();
    return true;
} 


void dbDatabase::reformatTable(oid_t tableId, dbTableDescriptor* desc)
{
    dbTable* table = (dbTable*)putRow(tableId);

    if (desc->match(table, confirmDeleteColumns, false)) { 
        TRACE_MSG(("New version of table '%s' is compatible with old one\n", 
                   desc->name));
        updateTableDescriptor(desc, tableId);
    } else { 
        TRACE_MSG(("Reformat table '%s'\n", desc->name));
        oid_t oid = table->firstRow;
        updateTableDescriptor(desc, tableId);
        while (oid != 0) { 
            dbRecord* record = getRow(oid);
            size_t size = 
                desc->columns->calculateNewRecordSize((byte*)record, 
                                                     desc->fixedSize);
            offs_t offs = currIndex[oid];
            record = putRow(oid, size);
            byte* dst = (byte*)record;
            byte* src = baseAddr + offs;
            if (dst == src) { 
                dbSmallBuffer buf(size);
                dst = (byte*)buf.base();
                desc->columns->convertRecord(dst, src, desc->fixedSize);
                memcpy(record+1, dst+sizeof(dbRecord), size-sizeof(dbRecord));
            } else { 
                desc->columns->convertRecord(dst, src, desc->fixedSize);
            }
            oid = record->next;
        }
    }
}
 
void dbDatabase::deleteTable(dbTableDescriptor* desc)
{
    beginTransaction(dbExclusiveLock);
    modified = true;
    dbTable* table = (dbTable*)putRow(desc->tableId);
    oid_t rowId = table->firstRow;
    table->firstRow = table->lastRow = 0;
    table->nRows = 0;
        
    while (rowId != 0) {
        dbRecord* record = getRow(rowId);
        oid_t nextId = record->next;
        size_t size = record->size;
        
        removeInverseReferences(desc, rowId);

        if (rowId < committedIndexSize && index[0][rowId] == index[1][rowId]) {
            cloneBitmap(currIndex[rowId], size);
        } else { 
            deallocate(currIndex[rowId], size);
        }
        freeId(rowId);
        rowId = nextId;
    }
    dbFieldDescriptor* fd;
    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField) { 
        dbHashTable::purge(this, fd->hashTable);
    } 
    for (fd = desc->indexedFields; fd != NULL; fd = fd->nextIndexedField) { 
        if (fd->type == dbField::tpRectangle) { 
            dbRtree::purge(this, fd->tTree);
        } else { 
            dbTtree::purge(this, fd->tTree);
        }
    } 
} 

void dbDatabase::dropHashTable(dbFieldDescriptor* fd)
{
    beginTransaction(dbExclusiveLock);
    modified = true;
    dbHashTable::drop(this, fd->hashTable);
    fd->hashTable = 0;
    fd->indexType &= ~HASHED;

    dbFieldDescriptor** fpp = &fd->defTable->hashedFields;
    while (*fpp != fd) { 
        fpp = &(*fpp)->nextHashedField;
    }
    *fpp = fd->nextHashedField;

    dbTable* table = (dbTable*)putRow(fd->defTable->tableId);
    dbField* field = (dbField*)((char*)table + table->fields.offs);
    field[fd->fieldNo].hashTable = 0;
}

void dbDatabase::dropIndex(dbFieldDescriptor* fd)
{
    beginTransaction(dbExclusiveLock);
    modified = true;
    if (fd->type == dbField::tpRectangle) { 
        dbRtree::drop(this, fd->tTree);
    } else { 
        dbTtree::drop(this, fd->tTree);
    }
    fd->tTree = 0;
    fd->indexType &= ~INDEXED;

    dbFieldDescriptor** fpp = &fd->defTable->indexedFields;
    while (*fpp != fd) { 
        fpp = &(*fpp)->nextIndexedField;
    }
    *fpp = fd->nextIndexedField;

    dbTable* table = (dbTable*)putRow(fd->defTable->tableId);
    dbField* field = (dbField*)((char*)table + table->fields.offs);
    field[fd->fieldNo].tTree = 0;
}

void dbDatabase::createHashTable(dbFieldDescriptor* fd)
{
    beginTransaction(dbExclusiveLock);
    modified = true;
    dbTable* table = (dbTable*)getRow(fd->defTable->tableId);
    size_t nRows = table->nRows;
    fd->hashTable = dbHashTable::allocate(this, 2*nRows);
    fd->attr &= ~dbFieldDescriptor::Updated;
    fd->nextHashedField = fd->defTable->hashedFields;
    fd->defTable->hashedFields = fd;
    fd->indexType |= HASHED;
    table = (dbTable*)putRow(fd->defTable->tableId);
    dbField* field = (dbField*)((char*)table + table->fields.offs);
    field[fd->fieldNo].hashTable = fd->hashTable;

    for (oid_t oid = table->firstRow; oid != 0; oid = getRow(oid)->next) {
        dbHashTable::insert(this, fd, oid, nRows);
    }
}


void dbDatabase::createIndex(dbFieldDescriptor* fd)
{
    beginTransaction(dbExclusiveLock);
    modified = true;
    if (fd->type == dbField::tpRectangle) { 
        fd->tTree = dbRtree::allocate(this);
    } else { 
        fd->tTree = dbTtree::allocate(this); 
    }
    fd->attr &= ~dbFieldDescriptor::Updated;
    fd->nextIndexedField = fd->defTable->indexedFields;
    fd->defTable->indexedFields = fd;
    fd->indexType |= INDEXED;
    dbTable* table = (dbTable*)putRow(fd->defTable->tableId);
    dbField* field = (dbField*)((char*)table + table->fields.offs);
    field[fd->fieldNo].tTree = fd->tTree;
    
    for (oid_t oid = table->firstRow; oid != 0; oid = getRow(oid)->next) {
        if (fd->type == dbField::tpRectangle) { 
            dbRtree::insert(this, fd->tTree, oid, fd->dbsOffs);
        } else { 
            dbTtree::insert(this, fd->tTree, oid, fd->type, (int)fd->dbsSize, fd->_comparator, fd->dbsOffs);
        }
    }
}

void dbDatabase::dropTable(dbTableDescriptor* desc)
{
    deleteTable(desc);
    freeRow(dbMetaTableId, desc->tableId);

    dbFieldDescriptor* fd;
    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField) { 
        dbHashTable::drop(this, fd->hashTable);
    } 
    for (fd = desc->indexedFields; fd != NULL; fd = fd->nextIndexedField) { 
        if (fd->type == dbField::tpRectangle) { 
            dbRtree::drop(this, fd->tTree);
        } else { 
            dbTtree::drop(this, fd->tTree);
        }
    } 
}

#define NEW_INDEX 0x80000000u

bool dbDatabase::addIndices(bool alter, dbTableDescriptor* desc)
{
    dbFieldDescriptor* fd;
    oid_t tableId = desc->tableId;
    dbTable* table = (dbTable*)getRow(tableId);
    int nRows = table->nRows;
    oid_t firstId = table->firstRow;
    int nNewIndices = 0;
    int nDelIndices = 0;
    for (fd = desc->firstField; fd != NULL; fd = fd->nextField) { 
        if ((fd->indexType & HASHED) && fd->type != dbField::tpStructure) { 
            if (fd->hashTable == 0) { 
                if (!alter && tableId < committedIndexSize
                    && index[0][tableId] == index[1][tableId])
                {
                    // If there are some other active applications which 
                    // can use this table, then they will not know
                    // about newly created index, which leads to inconsistency
                    return false;
                }
                fd->indexType |= NEW_INDEX;
                fd->hashTable = dbHashTable::allocate(this, nRows);
                nNewIndices += 1;
                TRACE_MSG(("Create hash table for field '%s'\n", fd->name));
            }
        } else if (fd->hashTable != 0) { 
            if (alter) { 
                TRACE_MSG(("Remove hash table for field '%s'\n", fd->name));
                nDelIndices += 1;
                fd->hashTable = 0;
            } else { 
                return false;
            }
        }
        if ((fd->indexType & INDEXED) && fd->type != dbField::tpStructure) { 
            if (fd->tTree == 0) { 
                if (!alter && tableId < committedIndexSize
                    && index[0][tableId] == index[1][tableId])
                {
                    return false;
                }
                fd->indexType |= NEW_INDEX;
                fd->tTree = (fd->type == dbField::tpRectangle) ? dbRtree::allocate(this) : dbTtree::allocate(this);
                nNewIndices += 1;
                TRACE_MSG(("Create index for field '%s'\n", fd->name));
            }
        } else if (fd->tTree != 0) { 
            if (alter) { 
                nDelIndices += 1;
                TRACE_MSG(("Remove index for field '%s'\n", fd->name));
                fd->tTree = 0;
            } else { 
                return false;
            }
        }
    }
    if (nNewIndices > 0) { 
        modified = true;
        for (oid_t rowId = firstId; rowId != 0; rowId = getRow(rowId)->next) {
            for (fd = desc->hashedFields; fd != NULL; fd=fd->nextHashedField) {
                if (fd->indexType & NEW_INDEX) { 
                    dbHashTable::insert(this, fd, rowId, 2*nRows);
                }
            }
            for (fd=desc->indexedFields; fd != NULL; fd=fd->nextIndexedField) {
                if (fd->indexType & NEW_INDEX) { 
                    if (fd->type == dbField::tpRectangle) { 
                        dbRtree::insert(this, fd->tTree, rowId, fd->dbsOffs);
                    } else { 
                        dbTtree::insert(this, fd->tTree, rowId, 
                                        fd->type, (int)fd->dbsSize, fd->_comparator, fd->dbsOffs);
                    }
                }
            }
        } 
        for (fd = desc->firstField; fd != NULL; fd = fd->nextField) {
            fd->indexType &= ~NEW_INDEX;
        }
    }
    if (nNewIndices + nDelIndices != 0) { 
        table = (dbTable*)putRow(tableId);
        offs_t fieldOffs = currIndex[tableId] + table->fields.offs;
        for (fd = desc->firstField; fd != NULL; fd = fd->nextField) { 
            dbField* field = (dbField*)(baseAddr + fieldOffs);    
            if (field->hashTable != fd->hashTable) { 
                if (field->hashTable != 0) { 
                    FASTDB_ASSERT(fd->hashTable == 0);
                    modified = true;
                    dbHashTable::drop(this, field->hashTable);
                    field = (dbField*)(baseAddr + fieldOffs);
                }
                field->hashTable = fd->hashTable;
            }
            if (field->tTree != fd->tTree) { 
                if (field->tTree != 0) { 
                    FASTDB_ASSERT(fd->tTree == 0);
                    modified = true;
                    if (field->type == dbField::tpRectangle) { 
                        dbRtree::drop(this, field->tTree);
                    } else { 
                        dbTtree::drop(this, field->tTree);
                    }
                    field = (dbField*)(baseAddr + fieldOffs);
                }
                field->tTree = fd->tTree;
            }
            fieldOffs += sizeof(dbField);
        }
    }
    return true;
}

 
void dbDatabase::updateTableDescriptor(dbTableDescriptor* desc, oid_t tableId)
{
    size_t newSize = sizeof(dbTable) + desc->nFields*sizeof(dbField)
        + desc->totalNamesLength();
    linkTable(desc, tableId);

    dbTable* table = (dbTable*)getRow(tableId);
    int   nRows = table->nRows;
    oid_t first = table->firstRow;
    oid_t last = table->lastRow;
#ifdef AUTOINCREMENT_SUPPORT
    desc->autoincrementCount = table->count;
#endif

    int nFields = table->fields.size;
    offs_t fieldOffs = currIndex[tableId] + table->fields.offs;

    while (--nFields >= 0) { 
        dbField* field = (dbField*)(baseAddr + fieldOffs);
        fieldOffs += sizeof(dbField);

        oid_t hashTableId = field->hashTable;
        oid_t tTreeId = field->tTree;
        int fieldType = field->type;
        if (hashTableId != 0) { 
            dbHashTable::drop(this, hashTableId);
        } 
        if (tTreeId != 0) { 
            if (fieldType == dbField::tpRectangle) { 
                dbRtree::drop(this, tTreeId);
            } else { 
                dbTtree::drop(this, tTreeId);
            }
        }
    } 

    table = (dbTable*)putRow(tableId, newSize);
    desc->storeInDatabase(table);
    table->firstRow = first;
    table->lastRow = last;
    table->nRows = nRows;
}

oid_t dbDatabase::addNewTable(dbTableDescriptor* desc)
{
    oid_t tableId = allocateRow(dbMetaTableId, 
                                sizeof(dbTable) + desc->nFields*sizeof(dbField)
                                + desc->totalNamesLength());
#ifdef AUTOINCREMENT_SUPPORT
    desc->autoincrementCount = dbTableDescriptor::initialAutoincrementCount;
#endif
    linkTable(desc, tableId);
    desc->storeInDatabase((dbTable*)getRow(tableId));
    return tableId;
}

void dbDatabase::close()
{
    if (opened) { 
        close0();
    }
}


void dbDatabase::close0()
{
    detach();
    if (backupFileName != NULL) { 
        {
            dbCriticalSection cs(backupMutex); 
            delete[] backupFileName;
            backupFileName = NULL;
            backupInitEvent.signal();
        }
        backupThread.join();
    }
    if (commitDelay != 0) { 
        delayedCommitStopTimerEvent.signal();
        {
            dbCriticalSection cs(delayedCommitStartTimerMutex);
            stopDelayedCommitThread = true;
            delayedCommitStartTimerEvent.signal();
        }
        commitDelay = 0;
        commitThread.join();
        delayedCommitStartTimerEvent.close();
        commitThreadSyncEvent.close();
    }
    {
        dbCriticalSection cs(threadContextListMutex);
        while (!threadContextList.isEmpty()) { 
            delete (dbDatabaseThreadContext*)threadContextList.next;
        }
    }
    backupInitEvent.close();

#ifdef AUTO_DETECT_PROCESS_CRASH
    watchDogMutex->lock();
#endif

    if (accessType == dbConcurrentUpdate) { 
        mutatorCS.enter();
    }
    cs.enter();

#ifdef AUTO_DETECT_PROCESS_CRASH
    dbL2List* wdc = &watchDogThreadContexts;
    while ((wdc = wdc->next) != &watchDogThreadContexts) {
        ((dbWatchDogContext*)wdc)->db = NULL;
    }
    if (watchDogThreadContexts.isEmpty()) {
        watchDogMutex->unlock();
        delete watchDogMutex;
    } else { 
        watchDogThreadContexts.unlink();
        watchDogMutex->unlock();
    }
    selfWatchDog.close();
#endif

    delete[] databaseName;
    delete[] fileName;
    databaseName = NULL;
    fileName = NULL;
    opened = false;

    monitor->users -= 1;
    if (header != NULL && header->dirty && accessType != dbReadOnly && accessType != dbConcurrentRead && monitor->nWriters == 0) 
    {
        file.flush(true);
        header->dirty = false;
        file.markAsDirty(0, sizeof(dbHeader));
    }
    cs.leave();
    if (accessType == dbConcurrentUpdate) { 
        mutatorCS.leave();
    }
    dbTableDescriptor *desc, *next;
    for (desc = tables; desc != NULL; desc = next) {
        next = desc->nextDbTable;
        desc->tableId = 0;
        if (!desc->isStatic) {
            delete desc;
        } else if (!desc->fixedDatabase) {
            desc->db = NULL;
        }
    }

    file.close();
    if (initMutex.close()) {
        cs.erase();
        readSem.erase();
        writeSem.erase();
        upgradeSem.erase();
        backupCompletedEvent.erase();
        file.erase();
        if (commitDelay != 0) {
            delayedCommitStopTimerEvent.erase();
        }
        if (accessType == dbConcurrentUpdate || accessType == dbConcurrentRead) { 
            mutatorCS.erase();
        }
        shm.erase();
        initMutex.erase();
    } else { 
        cs.close();
        shm.close();
        readSem.close();
        writeSem.close();
        upgradeSem.close();
        backupCompletedEvent.close();
        if (commitDelay != 0) { 
            delayedCommitStopTimerEvent.close();        
        }
        if (accessType == dbConcurrentUpdate || accessType == dbConcurrentRead) { 
            mutatorCS.close();
        }
    }
}

dbTableDescriptor* dbDatabase::lookupTable(dbTableDescriptor* origDesc)
{
    for (dbTableDescriptor* desc = tables; desc != NULL; desc = desc->nextDbTable) 
    {
        if (desc == origDesc || desc->cloneOf == origDesc) {
            return desc;
        }
    }
    return NULL;
}


void dbDatabase::attach() 
{
    if (threadContext.get() == NULL) { 
        dbDatabaseThreadContext* ctx = new dbDatabaseThreadContext();
        { 
            dbCriticalSection cs(threadContextListMutex);
            threadContextList.link(ctx);
        }
        threadContext.set(ctx);
    }
}

void dbDatabase::attach(dbDatabaseThreadContext* ctx)
{
    threadContext.set(ctx);
}

void dbDatabase::detach(int flags)
{
    if (flags & COMMIT) { 
        commit();
    } else { 
        monitor->uncommittedChanges = true;
        precommit();
    }
    if (flags & DESTROY_CONTEXT) { 
        dbDatabaseThreadContext* ctx = threadContext.get();    
        if (commitDelay != 0) { 
            dbCriticalSection cs(delayedCommitStopTimerMutex);
            if (monitor->delayedCommitContext == ctx && ctx->commitDelayed) { 
                ctx->removeContext = true;
            } else { 
                dbCriticalSection cs(threadContextListMutex);
                delete ctx;
            }
        } else { 
            dbCriticalSection cs(threadContextListMutex);
            delete ctx;
        }
        threadContext.set(NULL);
    }
}


bool dbDatabase::existsInverseReference(dbExprNode* expr, int nExistsClauses)
{
    while (true) { 
        switch (expr->cop) { 
          case dbvmLoadSelfReference:
          case dbvmLoadSelfArray:
            return expr->ref.field->inverseRef != NULL;
          case dbvmLoadReference:
            if (expr->ref.field->attr & dbFieldDescriptor::ComponentOfArray) { 
                expr = expr->ref.base;
                continue;
            }
            // no break
          case dbvmLoadArray:
            if (expr->ref.field->inverseRef == NULL) { 
                return false;
            }
            expr = expr->ref.base;
            continue; 
          case dbvmGetAt:
            if (expr->operand[1]->cop != dbvmVariable 
                || expr->operand[1]->offs != --nExistsClauses) 
            {
                return false;
            }
            expr = expr->operand[0];
            continue;
          case dbvmDeref:
            expr = expr->operand[0];
            continue;       
          default:
            return false;
        }
    }
}

bool dbDatabase::followInverseReference(dbExprNode* expr, dbExprNode* andExpr, 
                                        dbAnyCursor* cursor, oid_t iref)
{
    while (expr->cop == dbvmGetAt || expr->cop == dbvmDeref ||
           (expr->cop == dbvmLoadReference 
            && (expr->ref.field->attr & dbFieldDescriptor::ComponentOfArray))) 
    { 
        expr = expr->operand[0];        
    } 
    dbTable* table = (dbTable*)getRow(cursor->table->tableId);
    dbFieldDescriptor* fd = expr->ref.field->inverseRef;
    if (fd->type == dbField::tpArray) { 
        byte* rec = (byte*)getRow(iref);
        dbVarying* arr = (dbVarying*)(rec + fd->dbsOffs);
        oid_t* refs = (oid_t*)(rec + arr->offs);
        if (expr->cop >= dbvmLoadSelfReference) {
            for (int n = arr->size; --n >= 0;) { 
                oid_t oid = *refs++;
                if (oid != 0) { 
                    if (andExpr == NULL || evaluate(andExpr, oid, table, cursor)) { 
                        if (!cursor->add(oid)) { 
                            return false;
                        }
                    }
                }
            }
        } else { 
            for (int n = arr->size; --n >= 0;) { 
                oid_t oid = *refs++;
                if (oid != 0) { 
                    if (!followInverseReference(expr->ref.base, andExpr,
                                                cursor, oid))
                    {
                        return false;
                    }
                }
            }
        }
    } else { 
        FASTDB_ASSERT(fd->type == dbField::tpReference);
        oid_t oid = *(oid_t*)((byte*)getRow(iref) + fd->dbsOffs);
        if (oid != 0) { 
            if (expr->cop >= dbvmLoadSelfReference) {
                if (andExpr == NULL || evaluate(andExpr, oid, table, cursor)) { 
                    if (!cursor->add(oid)) { 
                        return false;
                    }
                }
            } else { 
                if (!followInverseReference(expr->ref.base, andExpr, 
                                            cursor, oid)) 
                {
                    return false;
                }
            }
        }
    }
    return true;
}
    


bool dbDatabase::isPrefixSearch(dbAnyCursor* cursor, 
                                dbExprNode* expr, 
                                dbExprNode* andExpr,
                                dbFieldDescriptor* &indexedField)
{
    if (expr->cop == dbvmLikeString  
        && expr->operand[1]->cop ==  dbvmStringConcat
        && expr->operand[1]->operand[0]->cop == dbvmLoadSelfString
        && expr->operand[1]->operand[0]->ref.field->tTree != 0
        && expr->operand[1]->operand[1]->cop == dbvmLoadStringConstant
        && strcmp(expr->operand[1]->operand[1]->svalue.str, "%") == 0)
    {        
        size_t paramBase = (size_t)cursor->paramBase;
        char* sval;
        dbExprNode* opd = expr->operand[0];
        switch (opd->cop) { 
          case dbvmLoadStringConstant:
            sval = (char*)opd->svalue.str;
            break;
          case dbvmLoadVarString:
            sval = (char*)((char*)opd->var + paramBase);
            break;
#ifdef USE_STD_STRING
          case dbvmLoadVarStdString:
            sval = (char*)((std::string*)((char*)opd->var + paramBase))->c_str();
            break;
#endif      
          case dbvmLoadVarStringPtr:
            sval = *(char**)((char*)opd->var + paramBase);
            break;
          default:
            return false;
        }  
        dbFieldDescriptor* field = expr->operand[1]->operand[0]->ref.field;
        dbSearchContext sc;
        sc.db = this;
        sc.type = dbField::tpString;
        sc.offs = field->dbsOffs;
        sc.field = field;
        sc.cursor = cursor;
        sc.prefixLength = 0;
        sc.condition = andExpr ? andExpr->operand[1] : (dbExprNode*)0;
        sc.probes = 0;
        sc.firstKey = sc.lastKey = sval;
        sc.firstKeyInclusion = sc.lastKeyInclusion = true;
        dbTtree::prefixSearch(this, field->tTree, sc);
        TRACE_MSG(("Index prefix search for field %s.%s: %d probes\n", 
                   field->defTable->name, field->longName, sc.probes)); 
        indexedField = field;
        return true;
    } else if (expr->cop == dbvmLikeWString  
        && expr->operand[1]->cop ==  dbvmWStringConcat
        && expr->operand[1]->operand[0]->cop == dbvmLoadSelfWString
        && expr->operand[1]->operand[0]->ref.field->tTree != 0
        && expr->operand[1]->operand[1]->cop == dbvmLoadWStringConstant
        && wcscmp(expr->operand[1]->operand[1]->wsvalue.str, L"%") == 0)
    {        
        size_t paramBase = (size_t)cursor->paramBase;
        wchar_t* sval;
        dbExprNode* opd = expr->operand[0];
        switch (opd->cop) { 
          case dbvmLoadWStringConstant:
            sval = (wchar_t*)opd->wsvalue.str;
            break;
          case dbvmLoadVarWString:
            sval = (wchar_t*)((char*)opd->var + paramBase);
            break;
#ifdef USE_STD_STRING
          case dbvmLoadVarStdWString:
            sval = (wchar_t*)((std::wstring*)((char*)opd->var + paramBase))->c_str();
            break;
#endif      
          case dbvmLoadVarWStringPtr:
            sval = *(wchar_t**)((char*)opd->var + paramBase);
            break;
          default:
            return false;
        }  
        dbFieldDescriptor* field = expr->operand[1]->operand[0]->ref.field;
        dbSearchContext sc;
        sc.db = this;
        sc.type = dbField::tpWString;
        sc.offs = field->dbsOffs;
        sc.field = field;
        sc.cursor = cursor;
        sc.prefixLength = 0;
        sc.condition = andExpr ? andExpr->operand[1] : (dbExprNode*)0;
        sc.probes = 0;
        sc.firstKey = sc.lastKey = (char*)sval;
        sc.firstKeyInclusion = sc.lastKeyInclusion = true;
        dbTtree::prefixSearch(this, field->tTree, sc);
        TRACE_MSG(("Index prefix search for field %s.%s: %d probes\n", 
                   field->defTable->name, field->longName, sc.probes)); 
        indexedField = field;
        return true;
    }
    return false;
}
 
bool dbDatabase::existsIndexedReference(dbExprNode* ref)
{
    while (ref->cop == dbvmDeref) {
        ref = ref->operand[0];
        if ((ref->cop != dbvmLoadSelfReference && ref->cop != dbvmLoadReference) 
            || (ref->ref.field->hashTable == 0 && ref->ref.field->tTree == 0)) 
        {
            return false;
        }
        if (ref->cop == dbvmLoadSelfReference) {
            return true;
        }
        ref = ref->operand[0];
    }
    return false;
}
    

bool dbDatabase::isIndexApplicable(dbAnyCursor* cursor, 
                                   dbExprNode* expr, dbExprNode* andExpr,
                                   dbFieldDescriptor* &indexedField)
{
    int nExistsClauses = 0;
    while (expr->cop == dbvmExists) { 
        expr = expr->operand[0];
        nExistsClauses += 1;
    }
    int cmpCop = expr->cop;

    if (dbExprNode::nodeOperands[cmpCop] < 2 && cmpCop != dbvmIsNull) { 
        return false;
    }
    if (isPrefixSearch(cursor, expr, andExpr, indexedField)) { 
        return true;
    }
    unsigned loadCop = expr->operand[0]->cop;

    if (loadCop - dbvmLoadSelfBool > dbvmLoadSelfRawBinary - dbvmLoadSelfBool
        && loadCop - dbvmLoadBool > dbvmLoadRawBinary - dbvmLoadBool)
    {
        return false;
    }
    dbFieldDescriptor* field = expr->operand[0]->ref.field;
    if (field->hashTable == 0 && field->tTree == 0) { 
        return false;
    }
    if (loadCop >= dbvmLoadSelfBool) { 
        if (isIndexApplicable(cursor, expr, andExpr)) { 
            indexedField = field;
            return true;
        }
    } 
    else if (existsInverseReference(expr->operand[0]->ref.base, nExistsClauses))
    { 
        dbAnyCursor tmpCursor(*field->defTable, dbCursorViewOnly, NULL);
        tmpCursor.paramBase = cursor->paramBase;
        if (isIndexApplicable(&tmpCursor, expr, NULL)) { 
            expr = expr->operand[0]->ref.base;
            indexedField = field;
            cursor->checkForDuplicates();
            if (andExpr != NULL) { 
                andExpr = andExpr->operand[1];
            }
            for (dbSelection::segment* curr = tmpCursor.selection.first; 
                 curr != NULL; 
                 curr = curr->next)
            { 
                for (int i = 0, n = (int)curr->nRows; i < n; i++) { 
                    if (!followInverseReference(expr, andExpr,  
                                                cursor, curr->rows[i]))
                    {
                        return true;
                    }
                } 
            }
            return true;
        }
    } else if (existsIndexedReference(expr->operand[0]->ref.base)) { 
        dbExprNode* ref = expr->operand[0]->ref.base->operand[0];
        dbFieldDescriptor* refField = ref->ref.field;
        FASTDB_ASSERT(refField->type == dbField::tpReference);
        dbAnyCursor tmpCursor[2];
        int currRefCursor = 0;
        tmpCursor[0].setTable(refField->refTable);
        tmpCursor[0].paramBase = cursor->paramBase;
        if (isIndexApplicable(&tmpCursor[0], expr, NULL)) { 
            oid_t oid;
            dbSearchContext sc;
            indexedField = field;
            sc.db = this;
            sc.type = dbField::tpReference;
            sc.prefixLength = 0;
            sc.field = refField;
            sc.condition = andExpr ? andExpr->operand[1] : (dbExprNode*)0;
            sc.firstKey = sc.lastKey = (char*)&oid;
            sc.firstKeyInclusion = sc.lastKeyInclusion = true;

            dbAnyCursor *srcCursor, *dstCursor;
            srcCursor = &tmpCursor[0];

            while (true) {
                sc.offs = refField->dbsOffs;
                if (ref->cop == dbvmLoadSelfReference) {
                    dstCursor = cursor;
                    sc.condition = andExpr;
                } else {
                    dstCursor = &tmpCursor[currRefCursor ^= 1];
                    dstCursor->setTable(refField->defTable);
                    dstCursor->reset();
                }
                sc.cursor = dstCursor;
                for (dbSelection::segment* curr = srcCursor->selection.first; 
                     curr != NULL; 
                     curr = curr->next)
                { 
                    for (int i = 0, n = (int)curr->nRows; i < n; i++) { 
                        oid = curr->rows[i];
                        sc.probes = 0;
                        if (refField->hashTable != 0) { 
                            dbHashTable::find(this, refField->hashTable, sc);
                            TRACE_MSG(("Hash table search for field %s.%s: %d probes\n", 
                                       refField->defTable->name, refField->longName, sc.probes)); 
                        } else { 
                            dbTtree::find(this, refField->tTree, sc);
                            TRACE_MSG(("Index search for field %s.%s: %d probes\n", 
                                       refField->defTable->name, refField->longName, sc.probes)); 
                        }
                    }
                }
                if (ref->cop == dbvmLoadSelfReference) {
                    break;
                }
                srcCursor = dstCursor;
                ref = ref->operand[0]->operand[0];
                refField = ref->ref.field;
                FASTDB_ASSERT(refField->type == dbField::tpReference);
            }
            return true;
        }
    }
    return false;
}

bool dbDatabase::isIndexApplicable(dbAnyCursor* cursor, 
                                   dbExprNode* expr, dbExprNode* andExpr)
{
    int n = dbExprNode::nodeOperands[expr->cop];
    dbFieldDescriptor* field = expr->operand[0]->ref.field;
    dbSearchContext sc;
    size_t paramBase = (size_t)cursor->paramBase;

    union { 
        bool  b;
        int1  i1;
        int2  i2;
        int4  i4;
        db_int8  i8;
        real4 f4;
        real8 f8;
        oid_t oid;
        char* s;
        wchar_t* ws;
        rectangle*  r;
        dbAnyArray* a;
    } literal[2];

    bool strop = false;
    bool binop = false;
    char* s;
    wchar_t* ws;
    literal[0].i8 = 0;
    literal[1].i8 = 0;

    for (int i = 0; i < n-1; i++) { 
        bool  bval = false;
        db_int8  ival = 0;
        real8 fval = 0;
        oid_t oid = 0;
        char* sval = NULL; 
        wchar_t* wsval = NULL; 
        rectangle* rect = NULL;
        dbExprNode* opd = expr->operand[i+1];
        switch (opd->cop) {  
          case dbvmLoadVarArrayOfOid:
          case dbvmLoadVarArrayOfInt4:
          case dbvmLoadVarArrayOfInt8:
            literal[i].a = (dbAnyArray*)((char*)opd->var + paramBase);
            strop = true;
            continue;
          case dbvmLoadVarArrayOfOidPtr:
          case dbvmLoadVarArrayOfInt4Ptr:
          case dbvmLoadVarArrayOfInt8Ptr:
            literal[i].a = *(dbAnyArray**)((char*)opd->var + paramBase);
            strop = true;
            continue;            
          case dbvmLoadVarBool:
            bval = *(bool*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarInt1:
            ival = *(int1*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarInt2:
            ival = *(int2*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarInt4:
            ival = *(int4*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarInt8:
            ival = *(db_int8*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarReference:
            oid = *(oid_t*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarRectangle:
            rect = (rectangle*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarRectanglePtr:
            rect = *(rectangle**)((char*)opd->var + paramBase);
            break;
          case dbvmLoadNull:
            oid = 0;
            break;
          case dbvmLoadVarReal4:
            fval = *(real4*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarReal8:
            fval = *(real8*)((char*)opd->var + paramBase);
            break;
          case dbvmLoadVarString:
            sval = (char*)opd->var + paramBase;
            strop = true;
            break;
#ifdef USE_STD_STRING
          case dbvmLoadVarStdString:
            sval = (char*)((std::string*)((char*)opd->var + paramBase))->c_str();
            strop = true;
            break;
#endif      
          case dbvmLoadVarStringPtr:
            sval = *(char**)((char*)opd->var + paramBase);
            strop = true;
            break;
          case dbvmLoadVarWString:
            wsval = (wchar_t*)((char*)opd->var + paramBase);
            strop = true;
            break;
#ifdef USE_STD_STRING
          case dbvmLoadVarStdWString:
            wsval = (wchar_t*)((std::wstring*)((char*)opd->var + paramBase))->c_str();
            strop = true;
            break;
#endif      
          case dbvmLoadVarWStringPtr:
            wsval = *(wchar_t**)((char*)opd->var + paramBase);
            strop = true;
            break;
          case dbvmLoadTrue:
            bval = true;
            break;
          case dbvmLoadFalse:
            bval = false;
            break;
          case dbvmLoadIntConstant:
            ival = opd->ivalue;
            break;
          case dbvmLoadRealConstant:
            fval = opd->fvalue;
            break;
          case dbvmLoadStringConstant:
            sval = (char*)opd->svalue.str;
            strop = true;
            break;
           case dbvmLoadWStringConstant:
            wsval = opd->wsvalue.str;
            strop = true;
            break;
         case dbvmLoadVarRawBinary:
            sval = (char*)((char*)opd->var + paramBase);
            strop = true;
            binop = true;
            break;
         case dbvmLoadVarRawBinaryPtr:
            sval = *(char**)((char*)opd->var + paramBase);
            strop = true;
            binop = true;
            break;
          default:
            return false;
        } 
        switch (field->type) { 
          case dbField::tpBool:
            literal[i].b = bval;
            break;
          case dbField::tpInt1:
            literal[i].i1 = (int1)ival;
            break;
          case dbField::tpInt2:
            literal[i].i2 = (int2)ival;
            break;
          case dbField::tpInt4:
            literal[i].i4 = (int4)ival;
            break;
          case dbField::tpInt8:
            literal[i].i8 = ival;
            break;
          case dbField::tpReference:
            literal[i].oid = oid;
            break;
          case dbField::tpRectangle:
            literal[i].r = rect;
            break;
          case dbField::tpReal4:
            literal[i].f4 = (real4)fval;
            break;
          case dbField::tpReal8:
            literal[i].f8 = fval;
            break;
          case dbField::tpString:
            literal[i].s = sval;
            break;
          case dbField::tpWString:
            literal[i].ws = wsval;
            break;
          case dbField::tpRawBinary:
            literal[i].s = sval;
            break;
          default:
            FASTDB_ASSERT(false);
        }
    }
    sc.db = this;
    sc.type = field->type;
    sc.offs = field->dbsOffs;
    sc.field = field;
    sc.cursor = cursor;
    sc.prefixLength = 0;
    sc.condition = andExpr ? andExpr->operand[1] : (dbExprNode*)0;
    sc.probes = 0;

    switch (expr->cop) { 
      case dbvmInArrayInt4:
        if (field->type == dbField::tpInt4) { 
            dbAnyArray* arr = literal[0].a;
            db_int4* items = (db_int4*)arr->base();
            sc.firstKeyInclusion = sc.lastKeyInclusion = true;
            cursor->checkForDuplicates();
            if (field->hashTable != 0) { 
                for (int n = (int)arr->length(); --n >= 0; items++) {
                    sc.firstKey = sc.lastKey = (char*)items;
                    dbHashTable::find(this, field->hashTable, sc);                    
                }                    
            } else { 
                for (int n = (int)arr->length(); --n >= 0; items++) {
                    sc.firstKey = sc.lastKey = (char*)items;
                    dbTtree::find(this, field->tTree, sc);
                }                    
            }
            return true;
        }
        return false;
        
      case dbvmInArrayInt8:
        if (field->type == dbField::tpInt8) { 
            dbAnyArray* arr = literal[0].a;
            db_int8* items = (db_int8*)arr->base();
            sc.firstKeyInclusion = sc.lastKeyInclusion = true;
            cursor->checkForDuplicates();
            if (field->hashTable != 0) { 
                for (int n = (int)arr->length(); --n >= 0; items++) {
                    sc.firstKey = sc.lastKey = (char*)items;
                    dbHashTable::find(this, field->hashTable, sc);                    
                }                    
            } else { 
                for (int n = (int)arr->length(); --n >= 0; items++) {
                    sc.firstKey = sc.lastKey = (char*)items;
                    dbTtree::find(this, field->tTree, sc);
                }                    
            }
            return true;
        }
        return false;

      case dbvmInArrayReference:
        if (field->type == dbField::tpReference) { 
            dbAnyArray* arr = literal[0].a;
            oid_t* items = (oid_t*)arr->base();
            sc.firstKeyInclusion = sc.lastKeyInclusion = true;
            cursor->checkForDuplicates();
            if (field->hashTable != 0) { 
                for (int n = (int)arr->length(); --n >= 0; items++) {
                    sc.firstKey = sc.lastKey = (char*)items;
                    dbHashTable::find(this, field->hashTable, sc);                    
                }                    
            } else { 
                for (int n = (int)arr->length(); --n >= 0; items++) {
                    sc.firstKey = sc.lastKey = (char*)items;
                    dbTtree::find(this, field->tTree, sc);
                }                    
            }
            return true;
        }
        return false;

      case dbvmInArrayRectangle:
        if (field->type == dbField::tpRectangle) { 
            dbAnyArray* arr = literal[0].a;
            rectangle* items = (rectangle*)arr->base();
            sc.firstKeyInclusion = dbRtree::SUBSET;
            cursor->checkForDuplicates();
            for (int n = (int)arr->length(); --n >= 0; items++) {
                sc.firstKey = (char*)items;
                dbRtree::find(this, field->tTree, sc);
            }                    
            return true;
        }
        return false;

      case dbvmEqRectangle:
      case dbvmLtRectangle:
      case dbvmLeRectangle:
      case dbvmGtRectangle:
      case dbvmGeRectangle:
        sc.firstKey = (char*)literal[0].r;
        sc.firstKeyInclusion = dbRtree::EQUAL + expr->cop - dbvmEqRectangle;
        dbRtree::find(this, field->tTree, sc);
        TRACE_MSG(("Spatial index search for field %s.%s: %d probes\n", 
                   field->defTable->name, field->longName, sc.probes)); 
        return true;

      case dbvmOverlapsRectangle:
        sc.firstKey = (char*)literal[0].r;
        sc.firstKeyInclusion = dbRtree::OVERLAPS;
        dbRtree::find(this, field->tTree, sc);
        TRACE_MSG(("Spatial index search for field %s.%s: %d probes\n", 
                   field->defTable->name, field->longName, sc.probes)); 
        return true;

      case dbvmIsNull:
      case dbvmEqReference:
      case dbvmEqInt:
      case dbvmEqBool:
      case dbvmEqReal:
      case dbvmEqArray:
      case dbvmEqString:
      case dbvmEqWString:
      case dbvmEqBinary:
        sc.firstKey = sc.lastKey = 
            strop ? literal[0].s : (char*)&literal[0];
        sc.firstKeyInclusion = sc.lastKeyInclusion = true;
        if (field->hashTable != 0) { 
            dbHashTable::find(this, field->hashTable, sc);
            TRACE_MSG(("Hash table search for field %s.%s: %d probes\n", 
                       field->defTable->name, field->longName, sc.probes)); 
        } else { 
            dbTtree::find(this, field->tTree, sc);
            TRACE_MSG(("Index search for field %s.%s: %d probes\n", 
                       field->defTable->name, field->longName, sc.probes)); 
        }
        return true;
      case dbvmGtInt:
      case dbvmGtReal:
      case dbvmGtString:
      case dbvmGtWString:
      case dbvmGtBinary:
      case dbvmGtArray:
        if (field->tTree != 0) {
            sc.firstKey = strop ? literal[0].s : (char*)&literal[0];
            sc.lastKey = NULL;
            sc.firstKeyInclusion = false;
            dbTtree::find(this, field->tTree, sc);
            TRACE_MSG(("Index search for field %s.%s: %d probes\n", 
                       field->defTable->name, field->longName, sc.probes)); 
            return true;
        } 
        return false;
      case dbvmGeInt:
      case dbvmGeReal:
      case dbvmGeString:        
      case dbvmGeWString:        
      case dbvmGeBinary:
      case dbvmGeArray:
        if (field->tTree != 0) {
            sc.firstKey = strop ? literal[0].s : (char*)&literal[0];
            sc.lastKey = NULL;
            sc.firstKeyInclusion = true;
            dbTtree::find(this, field->tTree, sc);
            TRACE_MSG(("Index search for field %s.%s: %d probes\n", 
                       field->defTable->name, field->longName, sc.probes)); 
            return true;
        } 
        return false;
      case dbvmLtInt:
      case dbvmLtReal:
      case dbvmLtString:
      case dbvmLtWString:
      case dbvmLtBinary:
      case dbvmLtArray:
        if (field->tTree != 0) {
            sc.firstKey = NULL;
            sc.lastKey = strop ? literal[0].s : (char*)&literal[0];
            sc.lastKeyInclusion = false;
            dbTtree::find(this, field->tTree, sc);
            TRACE_MSG(("Index search for field %s.%s: %d probes\n", 
                       field->defTable->name, field->longName, sc.probes)); 
            return true;
        } 
        return false;
      case dbvmLeInt:
      case dbvmLeReal:
      case dbvmLeString:
      case dbvmLeWString:
      case dbvmLeBinary:
      case dbvmLeArray:
        if (field->tTree != 0) {
            sc.firstKey = NULL;
            sc.lastKey = strop ? literal[0].s : (char*)&literal[0];
            sc.lastKeyInclusion = true;
            dbTtree::find(this, field->tTree, sc);
            TRACE_MSG(("Index search for field %s.%s: %d probes\n", 
                       field->defTable->name, field->longName, sc.probes)); 
            return true;
        } 
        return false;
      case dbvmBetweenInt:
      case dbvmBetweenReal:
      case dbvmBetweenString:
      case dbvmBetweenWString:
      case dbvmBetweenBinary:
      case dbvmBetweenArray:
        if (field->hashTable != 0 &&
            ((strop && field->_comparator(literal[0].s, literal[1].s, binop ? field->dbsSize : MAX_STRING_LENGTH) == 0)
             || (!strop && literal[0].i8 == literal[1].i8)))
        {
            sc.firstKey = strop ? literal[0].s : (char*)&literal[0];
            dbHashTable::find(this, field->hashTable, sc);
            TRACE_MSG(("Hash table search for field %s.%s: %d probes\n", 
                       field->defTable->name, field->longName, sc.probes)); 
            return true;
        } else if (field->tTree != 0) {
            sc.firstKey = strop ? literal[0].s : (char*)&literal[0];
            sc.firstKeyInclusion = true;
            sc.lastKey = strop ? literal[1].s : (char*)&literal[1];
            sc.lastKeyInclusion = true;
            dbTtree::find(this, field->tTree, sc);
            TRACE_MSG(("Index search for field %s.%s: %d probes\n", 
                       field->defTable->name, field->longName, sc.probes)); 
            return true;
        } 
        return false;
      case dbvmLikeString:
      case dbvmLikeEscapeString:
        if ((s = findWildcard(literal[0].s, literal[1].s)) == NULL 
            || ((s[1] == '\0' || s != literal[0].s) && field->tTree != 0))
        {
            if (s == NULL) { 
                sc.firstKey = sc.lastKey = literal[0].s;
                sc.firstKeyInclusion = sc.lastKeyInclusion = true;
                if (field->hashTable != 0) { 
                    dbHashTable::find(this, field->hashTable, sc);
                    TRACE_MSG(("Hash table search for field %s.%s: "
                               "%d probes\n", field->defTable->name, 
                               field->longName, sc.probes));
                } else { 
                    dbTtree::find(this, field->tTree, sc);
                    TRACE_MSG(("Index search for field %s.%s: %d probes\n", 
                               field->defTable->name, field->longName, 
                               sc.probes));
                }
            } else {            
                int len = (int)(s - literal[0].s);
                if (len == 0) { 
                    if (*s != dbMatchAnySubstring) { 
                        return false;
                    }
                    sc.firstKey = NULL;
                    sc.lastKey = NULL;
                    dbTtree::find(this, field->tTree, sc);
                    TRACE_MSG(("Use index for ordering records by field "
                               "%s.%s\n", field->defTable->name, 
                               field->longName)); 
                } else {            
                    sc.firstKey = new char[len+1];
                    memcpy(sc.firstKey, literal[0].s, len);
                    sc.firstKey[len] = '\0';
                    sc.firstKeyInclusion = true;
                    sc.lastKey = NULL;
                    sc.prefixLength = len;
                    if (s[0] != dbMatchAnySubstring || s[1] != '\0') { 
                        // Records selected by index do not necessarily
                        // match the pattern, so include pattern matching in
                        // condition expression
                        if (andExpr == NULL) { 
                            if (expr->operand[0]->cop != dbvmLoadSelfString) { 
                                dbExprNode load(dbvmLoadSelfString, 
                                                expr->operand[0]->ref.field);
                                dbExprNode like(expr->cop, &load, 
                                                expr->operand[1],
                                                expr->operand[2]);
                                sc.condition = &like;
                                dbTtree::find(this, field->tTree, sc);
                                like.cop = dbvmVoid;// do not destruct operands
                            } else { 
                                sc.condition = expr;
                                dbTtree::find(this, field->tTree, sc);
                            }
                        } else { 
                            sc.condition = andExpr;
                            dbTtree::find(this, field->tTree, sc);
                        }
                    } else { 
                        dbTtree::find(this, field->tTree, sc);
                    }
                    TRACE_MSG(("Index search for prefix in LIKE expression "
                               "for field %s.%s: %d probes\n", 
                               field->defTable->name, field->longName, 
                               sc.probes));
                    delete[] sc.firstKey;
                    delete[] sc.lastKey;
                }
            }   
            return true;
        }
        return false;
      case dbvmLikeWString:
      case dbvmLikeEscapeWString:
        if ((ws = findWildcard(literal[0].ws, literal[1].ws)) == NULL 
            || ((ws[1] == '\0' || ws != literal[0].ws) && field->tTree != 0))
        {
            if (ws == NULL) { 
                sc.firstKey = sc.lastKey = (char*)literal[0].ws;
                sc.firstKeyInclusion = sc.lastKeyInclusion = true;
                if (field->hashTable != 0) { 
                    dbHashTable::find(this, field->hashTable, sc);
                    TRACE_MSG(("Hash table search for field %s.%s: "
                               "%d probes\n", field->defTable->name, 
                               field->longName, sc.probes));
                } else { 
                    dbTtree::find(this, field->tTree, sc);
                    TRACE_MSG(("Index search for field %s.%s: %d probes\n", 
                               field->defTable->name, field->longName, 
                               sc.probes));
                }
            } else {            
                int len = (int)(ws - literal[0].ws);
                if (len == 0) { 
                    if (*ws != dbMatchAnySubstring) { 
                        return false;
                    }
                    sc.firstKey = NULL;
                    sc.lastKey = NULL;
                    dbTtree::find(this, field->tTree, sc);
                    TRACE_MSG(("Use index for ordering records by field "
                               "%s.%s\n", field->defTable->name, 
                               field->longName)); 
                } else {            
                    wchar_t* prefix = new wchar_t[len+1]; 
                    sc.firstKey = (char*)prefix;
                    memcpy(prefix, literal[0].ws, len*sizeof(wchar_t));
                    prefix[len] = '\0';
                    sc.firstKeyInclusion = true;
                    sc.lastKey = NULL;
                    sc.prefixLength = len;
                    if (ws[0] != dbMatchAnySubstring || ws[1] != '\0') { 
                        // Records selected by index do not necessarily
                        // match the pattern, so include pattern matching in
                        // condition expression
                        if (andExpr == NULL) { 
                            if (expr->operand[0]->cop != dbvmLoadSelfWString) { 
                                dbExprNode load(dbvmLoadSelfWString, 
                                                expr->operand[0]->ref.field);
                                dbExprNode like(expr->cop, &load, 
                                                expr->operand[1],
                                                expr->operand[2]);
                                sc.condition = &like;
                                dbTtree::find(this, field->tTree, sc);
                                like.cop = dbvmVoid;// do not destruct operands
                            } else { 
                                sc.condition = expr;
                                dbTtree::find(this, field->tTree, sc);
                            }
                        } else { 
                            sc.condition = andExpr;
                            dbTtree::find(this, field->tTree, sc);
                        }
                    } else { 
                        dbTtree::find(this, field->tTree, sc);
                    }
                    TRACE_MSG(("Index search for prefix in LIKE expression "
                               "for field %s.%s: %d probes\n", 
                               field->defTable->name, field->longName, 
                               sc.probes));
                    delete[] sc.firstKey;
                    delete[] sc.lastKey;
                }
            }   
            return true;
        }
    }
    return false;
}


struct SearchThreadArgument { 
    dbParallelQueryContext* ctx;
    int                     id;
};
                             

static void thread_proc parallelSearch(void* arg) 
{
    SearchThreadArgument* sa = (SearchThreadArgument*)arg;
    sa->ctx->search(sa->id);
}


void dbDatabase::traverse(dbAnyCursor* cursor, dbQuery& query) 
{
    const int defaultStackSize = 1024;
    oid_t buf[defaultStackSize];
    oid_t *stack = buf;
    int   stackSize = defaultStackSize;
    int   sp = 0, len;
    dbAnyArray* arr;
    oid_t oid, *refs;
    dbTable* table = (dbTable*)getRow(cursor->table->tableId);

    void* root = (void*)query.root;
    switch (query.startFrom) { 
      case dbCompiledQuery::StartFromFirst:
        oid = table->firstRow;
        if (oid != 0) { 
            stack[sp++] = oid;
        }
        break;
      case dbCompiledQuery::StartFromLast:
        oid = table->lastRow;
        if (oid != 0) { 
            stack[sp++] = oid;
        }
        break;
      case dbCompiledQuery::StartFromRef:
        oid = *(oid_t*)root;
        if (oid != 0) { 
            stack[sp++] = oid;
        }
        break;
      case dbCompiledQuery::StartFromArrayPtr:
        root = *(dbAnyArray**)root;
        // no break
      case dbCompiledQuery::StartFromArray:
        arr = (dbAnyArray*)root;
        len = (int)arr->length();
        if (len > stackSize) {
            stackSize = len;
            stack = new oid_t[stackSize];
        }
        refs = (oid_t*)arr->base();
        while (--len >= 0) { 
            oid = refs[len];
            if (oid != 0) { 
                stack[sp++] = oid;
            }
        }
        break;
      default:
        FASTDB_ASSERT(false);
    }
    cursor->checkForDuplicates();
    dbExprNode* condition = query.tree;
    dbFollowByNode* follow = query.follow;

    while (sp != 0) { 
        oid_t curr = stack[--sp];
        if (condition->cop == dbvmVoid || evaluate(condition, curr, table, cursor)) {
            if (!cursor->add(curr)) { 
                break;
            }
        } else { 
            cursor->mark(curr);
        }
        byte* record = (byte*)getRow(curr);
        for (dbFollowByNode* fp = follow; fp != NULL; fp = fp->next) { 
            dbFieldDescriptor* fd = fp->field;
            if (fd->type == dbField::tpArray) { 
                dbVarying* vp = (dbVarying*)(record + fd->dbsOffs);
                len = vp->size;
                if (sp + len > stackSize) { 
                    int newSize = len > stackSize ? len*2 : stackSize*2;
                    oid_t* newStack = new oid_t[newSize];
                    memcpy(newStack, stack, stackSize*sizeof(oid_t));
                    stackSize = newSize;
                    if (stack != buf) { 
                        delete[] stack;
                    }
                    stack = newStack;
                }
                refs = (oid_t*)(record + vp->offs);
                while (--len >= 0) { 
                    oid = refs[len];
                    if (oid != 0 && !cursor->isMarked(oid)) { 
                        stack[sp++] = oid;
                    }
                }
            } else { 
                FASTDB_ASSERT(fd->type == dbField::tpReference);
                if (sp == stackSize) { 
                    int newSize = stackSize*2;
                    oid_t* newStack = new oid_t[newSize];
                    memcpy(newStack, stack, stackSize*sizeof(oid_t));
                    stackSize = newSize;
                    if (stack != buf) { 
                        delete[] stack;
                    }
                    stack = newStack;
                }
                oid = *(oid_t*)(record + fd->dbsOffs);
                if (oid != 0 && !cursor->isMarked(oid)) { 
                    stack[sp++] = oid;
                }
            }
        }
    }
    if (stack != buf) { 
        delete[] stack;
    }
    if (query.order != NULL) { 
        cursor->selection.sort(this, query.order);
    }
}


bool dbDatabase::prepareQuery(dbAnyCursor* cursor, dbQuery& query)
{
    if (cursor == NULL) {
        return false;
    }
    FASTDB_ASSERT(opened);
    dbDatabaseThreadContext* ctx = threadContext.get();
    FASTDB_ASSERT(ctx != NULL);
    { 
        dbCriticalSection cs(query.mutex);  
        query.mutexLocked = true; 
        if (!query.compiled() || cursor->table != query.table || schemeVersion != query.schemeVersion) { 
            query.schemeVersion = schemeVersion;
            if (!ctx->compiler.compile(cursor->table, query)) { 
                query.mutexLocked = false;
                return false;
            }
        }
        query.mutexLocked = false;
        return true;
    }
}

#ifdef PROFILE

const size_t PROFILER_HASH_SIZE = 1013;
const size_t PROFILER_MAX_QUERY_LENGTH = 1024;

class Profiler {
    dbMutex mutex;
    
    struct QueryStat { 
        QueryStat* next;
        char*      query;
        size_t     count;
        size_t     hashCode;
        time_t     maxTime;
        time_t     totalTime;
        bool       sequential;

        ~QueryStat() { 
            delete[] query;
        }
    };
    
    QueryStat* table[PROFILER_HASH_SIZE];
    size_t nQueries;


    static int orderByTime(void const* p1, void const* p2) { 
        QueryStat* q1 = *(QueryStat**)p1;
        QueryStat* q2 = *(QueryStat**)p2;
        return q1->totalTime < q2->totalTime ? -1 
            : q1->totalTime > q2->totalTime ? 1 : q1->count < q2->count ? -1 : q1->count == q2->count ? 0 : 1;
    }

    static size_t hashFunction(char const* s) { 
        size_t h;
        for (h = 0; *s != '\0'; h = h*31 + (unsigned char)*s++);
        return h;
    }

  public:
    void add(char const* query, time_t elapsed, bool sequential) 
    { 
        dbCriticalSection cs(mutex);
        size_t hash = hashFunction(query);
        size_t h = hash  % PROFILER_HASH_SIZE;
        QueryStat* s;
        for (s = table[h]; s != NULL; s = s->next)
        {
            if (s->hashCode == hash && strcmp(query, s->query) == 0) { 
                s->count += 1;
                if (elapsed > s->maxTime) { 
                    s->maxTime = elapsed;
                }
                s->totalTime += elapsed;
                s->sequential |= sequential;
                return;
            }
        }
        s = new QueryStat();
        s->query = new char[strlen(query) + 1];
        strcpy(s->query, query);
        s->next = table[h];
        table[h] = s;
        s->totalTime = elapsed;
        s->maxTime = elapsed;
        s->count = 1;
        s->hashCode = hash;
        s->sequential = sequential;
        nQueries += 1;
    }
    
    void dump(char const* filePath);

    void dumpToStream(FILE* f) 
    { 
        dbCriticalSection cs(mutex);
        QueryStat** sa = new QueryStat*[nQueries];
        QueryStat** pp = sa;
        time_t total = 0;
        for (size_t i = 0; i < PROFILER_HASH_SIZE; i++) { 
            for (QueryStat* sp = table[i]; sp != NULL; sp = sp->next) { 
                *pp++ = sp;
                total += sp->totalTime;
            }
        }
        qsort(sa, nQueries, sizeof(QueryStat*), &orderByTime);
        fprintf(f, "S     Total      Count Maximum Average Percent Query\n");
        while (pp != sa) { 
            QueryStat* s = *--pp;
            fprintf(f, "%c%10ld %10ld %7d %7d %6d%% %s\n", 
                    s->sequential ? '!' : ' ', 
                    (long)s->totalTime, (long)s->count, (int)s->maxTime, (s->count != 0 ? (int)(s->totalTime/s->count) : 0), 
                    (total != 0 ? (int)(s->totalTime*100/total) : 0),
                    s->query
                );
        }
        delete[] sa;    
    }

    ~Profiler() { 
        dumpToStream(stdout);
        for (size_t i = 0; i < PROFILER_HASH_SIZE; i++) { 
            QueryStat* next;
            for (QueryStat* sp = table[i]; sp != NULL; sp = next) { 
                next = sp->next;
                delete sp;
            }
        }
    }                

    struct Measure
    {
        Profiler& profiler;
        dbAnyCursor* cursor;
        dbQuery*  query;
        unsigned  start;
        bool      sequential;

        Measure(Profiler& p, dbAnyCursor* c, dbQuery* q = NULL) 
        : profiler(p), cursor(c), query(q), start(dbSystem::getCurrentTimeMsec()), sequential(false) {}

        ~Measure() { 
            char buf[PROFILER_MAX_QUERY_LENGTH];
            int n = sprintf(buf, "SELECT FROM %s", cursor->getTable()->getName());
            if (query != NULL) { 
                n += sprintf(buf + n, " WHERE ");
                query->dump(buf + n);
            }
            profiler.add(buf, dbSystem::getCurrentTimeMsec() - start, sequential);
        }
    };
};

void Profiler::dump(char const* filePath)
{
    FILE* f = fopen(filePath, "w");
    dumpToStream(f);
    fclose(f);
}

Profiler profiler;
#endif        

void dbDatabase::select(dbAnyCursor* cursor, dbQuery& query) 
{    
#ifdef PROFILE
    Profiler::Measure measure(profiler, cursor, &query);
#endif        
    FASTDB_ASSERT(opened);
    dbDatabaseThreadContext* ctx = threadContext.get();
    dbFieldDescriptor* indexedField = NULL;
    FASTDB_ASSERT(ctx != NULL);
    { 
        dbCriticalSection cs(query.mutex);  
        query.mutexLocked = true; 
        if (!query.compiled() || cursor->table != query.table || schemeVersion != query.schemeVersion) { 
            query.schemeVersion = schemeVersion;
            if (!ctx->compiler.compile(cursor->table, query)) { 
                query.mutexLocked = false;
                return;
            }
        }
        query.mutexLocked = false;
    }
#if FASTDB_DEBUG == DEBUG_TRACE
    char buf[4096];
    if (query.elements != NULL) { 
        TRACE_MSG(("Query:  select * from %s where %s\n", query.table->name,  query.dump(buf)));
    } else { 
        TRACE_MSG(("Query:  select * from %s\n", query.table->name));
    }
#endif 
    beginTransaction(cursor->type == dbCursorForUpdate 
                     ? dbDatabase::dbExclusiveLock : dbDatabase::dbSharedLock);
    if (query.limitSpecified && query.order == NULL) { 
        cursor->setStatementLimit(query);
    }    

    if (query.startFrom != dbCompiledQuery::StartFromAny) { 
        ctx->cursors.link(cursor);
        traverse(cursor, query);
        if (query.limitSpecified && query.order != NULL) { 
            cursor->setStatementLimit(query); 
            cursor->truncateSelection();
        }
        return;
    }

    dbExprNode* condition = query.tree;
    if (condition->cop == dbvmVoid && query.order == NULL && !query.limitSpecified) { 
        // Empty select condition: select all records in the table
        select(cursor);
        return;
    } 
    if (condition->cop == dbvmEqReference) {
        if (condition->operand[0]->cop == dbvmCurrent) { 
            if (condition->operand[1]->cop == dbvmLoadVarReference) { 
                cursor->setCurrent(*(dbAnyReference*)((char*)condition->operand[1]->var + (size_t)cursor->paramBase));
                return;
            } else if (condition->operand[1]->cop == dbvmIntToReference
                       && condition->operand[1]->operand[0]->cop == dbvmLoadIntConstant) 
            {
                oid_t oid = (oid_t)condition->operand[1]->operand[0]->ivalue;
                cursor->setCurrent(*(dbAnyReference*)&oid);
                return;
            }
        }
        if (condition->operand[1]->cop == dbvmCurrent) { 
            if (condition->operand[0]->cop == dbvmLoadVarReference) { 
                cursor->setCurrent(*(dbAnyReference*)((char*)condition->operand[0]->var + (size_t)cursor->paramBase));
                return;
            } else if (condition->operand[0]->cop == dbvmIntToReference
                       && condition->operand[0]->operand[0]->cop == dbvmLoadIntConstant) 
            {
                oid_t oid = (oid_t)condition->operand[0]->operand[0]->ivalue;
                cursor->setCurrent(*(dbAnyReference*)&oid);
                return;
            }
        }
    }
    ctx->cursors.link(cursor);

    if (condition->cop == dbvmAndBool) { 
        if (isIndexApplicable(cursor, condition->operand[0], 
                              condition, indexedField)) 
        { 
            if (query.order != NULL) { 
                if (/*indexedField->defTable != query.table
                      || */query.order->next != NULL 
                           || query.order->getField() != indexedField)
                { 
                    cursor->selection.sort(this, query.order);
                } else if (!query.order->ascent) { 
                    cursor->selection.reverse();
                }
                if (query.limitSpecified) {
                    cursor->setStatementLimit(query); 
                    cursor->truncateSelection();
                }
            }
            return;
        }
    } else { 
        if (condition->cop == dbvmOrBool) { 
            cursor->checkForDuplicates();
        }
        while (condition->cop == dbvmOrBool 
               && !cursor->isLimitReached()
               && isIndexApplicable(cursor, condition->operand[0], NULL, 
                                    indexedField))
        {
            condition = condition->operand[1];
        }
        if (!cursor->isLimitReached()
            && isIndexApplicable(cursor, condition, NULL, indexedField)) 
        { 
            if (query.order != NULL) {
                if (/*indexedField->defTable != query.table
                      || */condition != query.tree
                    || query.order->next != NULL 
                    || query.order->getField() != indexedField)
                { 
                    cursor->selection.sort(this, query.order);
                } else if (!query.order->ascent) { 
                    cursor->selection.reverse();                
                }
                if (query.limitSpecified) {
                    cursor->setStatementLimit(query); 
                    cursor->truncateSelection();
                }
            }
            return;
        }
    }
    if (query.order != NULL && query.order->next == NULL 
        && query.order->field != NULL && query.order->field->type != dbField::tpRectangle 
        && query.order->field->tTree != 0) 
    { 
        dbFieldDescriptor* field = query.order->field;
        TRACE_MSG(("Use index for ordering records by field %s.%s\n", 
                   query.table->name, field->longName)); 
        if (query.limitSpecified) {
            cursor->setStatementLimit(query); 
        }
        if (condition->cop == dbvmVoid) { 
            if (query.order->ascent) { 
                dbTtree::traverseForward(this, field->tTree, cursor); 
            } else { 
                dbTtree::traverseBackward(this, field->tTree, cursor);
            }
        } else { 
            if (query.order->ascent) { 
                dbTtree::traverseForward(this,field->tTree,cursor,condition); 
            } else { 
                dbTtree::traverseBackward(this,field->tTree,cursor,condition);
            }
        }
        return;
    }
#ifdef PROFILE
    measure.sequential = true;
#endif

    dbTable* table = (dbTable*)getRow(cursor->table->tableId);
    int n = parThreads-1;
    if (cursor->getNumberOfRecords() == 0
        && n > 0 && table->nRows >= parallelScanThreshold 
        && cursor->limit >= dbDefaultSelectionLimit)
    {
        dbPooledThread* thread[dbMaxParallelSearchThreads];
        SearchThreadArgument sa[dbMaxParallelSearchThreads];
        dbParallelQueryContext par(this, table, &query, cursor);
        int i;
        for (i = 0; i < n; i++) { 
            sa[i].id = i;
            sa[i].ctx = &par;
            thread[i] = threadPool.create((dbThread::thread_proc_t)parallelSearch, &sa[i]);
        }
        par.search(i);
        for (i = 0; i < n; i++) { 
            threadPool.join(thread[i]);
        }
        if (query.order != NULL) { 
            oid_t rec[dbMaxParallelSearchThreads];
            for (i = 0; i <= n; i++) { 
                if (par.selection[i].first != NULL) { 
                    rec[i] = par.selection[i].first->rows[0];
                } else { 
                    rec[i] = 0;
                }
            }
            while (true) { 
                int min = -1;
                for (i = 0; i <= n; i++) { 
                    if (rec[i] != 0
                        && (min < 0 || dbSelection::compare(rec[i], rec[min], 
                                                            query.order) < 0))
                    {
                        min = i;
                    }
                }
                if (min < 0) { 
                    return;
                }
                oid_t oid = 
                    par.selection[min].first->rows[par.selection[min].pos];
                cursor->selection.add(oid);
                par.selection[min].pos += 1;
                if (par.selection[min].pos == par.selection[min].first->nRows){
                    par.selection[min].pos = 0;
                    dbSelection::segment* next=par.selection[min].first->next;
                    delete par.selection[min].first;
                    if ((par.selection[min].first = next) == NULL) { 
                        rec[min] = 0;
                        continue;
                    }
                }
                oid = par.selection[min].first->rows[par.selection[min].pos];
                rec[min] = oid;
            }
        } else { 
            for (i = 0; i <= n; i++) { 
                if (par.selection[i].first != NULL) { 
                    par.selection[i].first->prev = cursor->selection.last;
                    if (cursor->selection.last == NULL) { 
                        cursor->selection.first = par.selection[i].first;
                    } else { 
                        cursor->selection.last->next = par.selection[i].first;
                    }
                    cursor->selection.last = par.selection[i].last;
                    cursor->selection.nRows += par.selection[i].nRows;  
                }
            }
        }               
    } else { 
        oid_t oid = table->firstRow;
        if (!cursor->isLimitReached()) { 
            while (oid != 0) { 
                if (evaluate(condition, oid, table, cursor)) { 
                    if (!cursor->add(oid)) { 
                        break;
                    }
                }
                oid = getRow(oid)->next;
            }
        }
        if (query.order != NULL) { 
            cursor->selection.sort(this, query.order);
        }
    }
    if (query.limitSpecified && query.order != NULL) {
        cursor->setStatementLimit(query); 
        cursor->truncateSelection();
    }
}

void dbDatabase::select(dbAnyCursor* cursor) 
{
#ifdef PROFILE
    Profiler::Measure measure(profiler, cursor);
#endif        
    FASTDB_ASSERT(opened);
    beginTransaction(cursor->type == dbCursorForUpdate ? dbExclusiveLock : dbSharedLock);
    dbTable* table = (dbTable*)getRow(cursor->table->tableId);
    cursor->firstId = table->firstRow;
    cursor->lastId = table->lastRow;
    cursor->selection.nRows = table->nRows;
    cursor->allRecords = true;
    threadContext.get()->cursors.link(cursor);
}


void dbDatabase::remove(dbTableDescriptor* desc, oid_t delId) 
{
    modified = true;
    beginTransaction(dbExclusiveLock);
    dbTransactionLogger* tl = logger;
    if (tl != NULL) { 
        if (!tl->remove(desc, delId)) { 
            handleError(RejectedByTransactionLogger);
        }
        logger = NULL; // do not log cascade deletes
    }            
    if (inverseReferencesUpdate) { 
        removeInverseReferences(desc, delId);
    }
    dbFieldDescriptor* fd;
    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField){
        dbHashTable::remove(this, fd, delId);
    }
    for (fd = desc->indexedFields; fd != NULL; fd = fd->nextIndexedField) { 
        if (fd->type == dbField::tpRectangle) { 
            dbRtree::remove(this, fd->tTree, delId, fd->dbsOffs);
        } else { 
            dbTtree::remove(this, fd->tTree, delId, fd->type, (int)fd->dbsSize, fd->_comparator, fd->dbsOffs);
        }
    }

    freeRow(desc->tableId, delId);
    updateCursors(delId, true);
    logger = tl;
}


dbRecord* dbDatabase::putRow(oid_t oid, size_t newSize) 
{  
    offs_t offs = currIndex[oid];
    if (oid < committedIndexSize && index[0][oid] == index[1][oid]) { 
        size_t pageNo = oid/dbHandlesPerPage;
        monitor->dirtyPagesMap[pageNo >> 5] |= 1 << (pageNo & 31);
        cloneBitmap(offs, getRow(oid)->size);
        offs_t pos = allocate(newSize);
        currIndex[oid] = pos;
    } else { 
        size_t oldSize = getRow(oid)->size;
        if (oldSize != newSize) {
            offs_t pos = allocate(newSize);
            currIndex[oid] = pos;
            cloneBitmap(offs, oldSize);
            deallocate(offs, oldSize);
        }
    }
    dbRecord* record = (dbRecord*)(baseAddr + currIndex[oid]);
    record->next = ((dbRecord*)(baseAddr + offs))->next;
    record->prev = ((dbRecord*)(baseAddr + offs))->prev;
    record->size = (nat4)newSize;
    return record;
}

void dbDatabase::allocateRow(oid_t tableId, oid_t oid, size_t size) 
{ 
    offs_t pos = allocate(size);
    currIndex[oid] = pos;
    dbTable* table = (dbTable*)putRow(tableId);
    dbRecord* record = getRow(oid);
    record->size = (nat4)size;
    record->next = 0;
    record->prev = table->lastRow;
    if (table->lastRow != 0) { 
        if (accessType == dbConcurrentUpdate) { 
            putRow(table->lastRow)->next = oid;  
            table = (dbTable*)getRow(tableId);
        } else { 
            //
            // Optimisation hack: avoid cloning of the last record. 
            // Possible inconsistency in L2-list will be eliminated by recovery
            // procedure.
            //
            getRow(table->lastRow)->next = oid;
            file.markAsDirty(currIndex[table->lastRow], sizeof(dbRecord));
        }
    } else { 
        table->firstRow = oid;
    }
    table->lastRow = oid;
    table->nRows += 1;
#ifdef AUTOINCREMENT_SUPPORT
    table->count += 1;
#endif
}

void dbDatabase::freeRow(oid_t tableId, oid_t oid)
{
    dbTable* table = (dbTable*)putRow(tableId);
    dbRecord* del = getRow(oid);
    size_t size = del->size;
    oid_t next = del->next;
    oid_t prev = del->prev;
    table->nRows -= 1;
    if (prev == 0) {
        table->firstRow = next;
    }
    if (next == 0) {
        table->lastRow = prev;
    }
    if (prev != 0) {
        putRow(prev)->next = next;
    }
    if (next != 0) {
        putRow(next)->prev = prev;
    }
    if (oid < committedIndexSize && index[0][oid] == index[1][oid]) { 
        cloneBitmap(currIndex[oid], size);
    } else { 
        deallocate(currIndex[oid], size);
    }
    freeId(oid);
}

void dbDatabase::freeObject(oid_t oid) 
{ 
    offs_t marker = currIndex[oid] & dbInternalObjectMarker;
    if (oid < committedIndexSize && index[0][oid] == index[1][oid]) { 
        cloneBitmap(currIndex[oid] - marker, internalObjectSize[marker]);
    } else { 
        deallocate(currIndex[oid] - marker, internalObjectSize[marker]);
    } 
    freeId(oid);
}
 

void dbDatabase::update(oid_t oid, dbTableDescriptor* desc, void const* record)
{
    FASTDB_ASSERT(opened);
    beginTransaction(dbExclusiveLock);
    size_t size = 
        desc->columns->calculateRecordSize((byte*)record, desc->fixedSize);

    byte* src = (byte*)record;
    desc->columns->markUpdatedFields((byte*)getRow(oid), src);

    dbFieldDescriptor* fd;
    if (inverseReferencesUpdate) { 
        updatedRecordId = oid;
        for (fd = desc->inverseFields; fd != NULL; fd = fd->nextInverseField) { 
            if (fd->type == dbField::tpArray) { 
                dbAnyArray* arr = (dbAnyArray*)(src + fd->appOffs);
                int n = (int)arr->length();
                oid_t* newrefs = (oid_t*)arr->base();
                
                byte* old = (byte*)getRow(oid);
                int m = ((dbVarying*)(old + fd->dbsOffs))->size;
                int offs = ((dbVarying*)(old + fd->dbsOffs))->offs;
                int i, j, k;
                old += offs;
                
                for (i = j = 0; i < m; i++) {
                    oid_t oldref = *((oid_t*)old + i); 
                    if (oldref != 0) { 
                        for (k = j; j < n && newrefs[j] != oldref; j++);
                        if (j == n) { 
                            for (j = k--; k >= 0 && newrefs[k] != oldref; k--);
                            if (k < 0) { 
                                removeInverseReference(fd, oid, oldref);
                                old = (byte*)getRow(oid) + offs;
                            }
                        } else { 
                            j += 1;
                        }
                    }
                }
                for (i = j = 0; i < n; i++) { 
                    if (newrefs[i] != 0) { 
                        for(k=j; j < m && newrefs[i] != *((oid_t*)old+j); j++);
                        if (j == m) { 
                            for (j=k--; k >= 0 && newrefs[i] != *((oid_t*)old+k); k--);
                            if (k < 0) { 
                                insertInverseReference(fd, oid, newrefs[i]);
                                old = (byte*)getRow(oid) + offs;
                            }
                        } else { 
                            j += 1;
                        }
                    }
                }
            } else { 
                oid_t newref = *(oid_t*)(src + fd->appOffs);
                byte* old = (byte*)getRow(oid);
                oid_t oldref = *(oid_t*)(old + fd->dbsOffs); 
                if (newref != oldref) {
                    if (oldref != 0) { 
                        removeInverseReference(fd, oid, oldref);
                    }
                    if (newref != 0) { 
                        insertInverseReference(fd, oid, newref);
                    }
                }
            }
        }           
        updatedRecordId = 0;
    }
    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField) {
        if (fd->attr & dbFieldDescriptor::Updated) { 
            dbHashTable::remove(this, fd, oid);
        }
    } 
    for (fd = desc->indexedFields; fd != NULL; fd = fd->nextIndexedField) { 
        if (fd->attr & dbFieldDescriptor::Updated) { 
            if (fd->type == dbField::tpRectangle) { 
                dbRtree::remove(this, fd->tTree, oid, fd->dbsOffs);
            } else { 
                dbTtree::remove(this, fd->tTree, oid, fd->type, (int)fd->dbsSize, fd->_comparator, fd->dbsOffs);
            }
        }
    }

    byte* old = (byte*)getRow(oid);    
    byte* dst = (byte*)putRow(oid, size);    
    if (dst == old) { 
        dbSmallBuffer buf(size);
        byte* temp = (byte*)buf.base();
        desc->columns->storeRecordFields(temp, src, desc->fixedSize, dbFieldDescriptor::Update);
        memcpy(dst+sizeof(dbRecord), temp+sizeof(dbRecord), size-sizeof(dbRecord));
    } else { 
        desc->columns->storeRecordFields(dst, src, desc->fixedSize, dbFieldDescriptor::Update);
    }
    modified = true;
    if (logger != NULL) { 
        if (!logger->update(desc, oid, (dbRecord*)dst, record)) { 
            handleError(RejectedByTransactionLogger);
        }
    }            

    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField) {
        if (fd->attr & dbFieldDescriptor::Updated) { 
            dbHashTable::insert(this, fd, oid, 0);
        }
    } 
    for (fd = desc->indexedFields; fd != NULL; fd = fd->nextIndexedField) { 
        if (fd->attr & dbFieldDescriptor::Updated) { 
            if (fd->type == dbField::tpRectangle) { 
                dbRtree::insert(this, fd->tTree, oid, fd->dbsOffs);
            } else { 
                dbTtree::insert(this, fd->tTree, oid, fd->type, (int)fd->dbsSize, fd->_comparator, fd->dbsOffs);
            }
            fd->attr &= ~dbFieldDescriptor::Updated;
        }
    }
    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField) {
        fd->attr &= ~dbFieldDescriptor::Updated;        
    }
    updateCursors(oid); 
}


void dbDatabase::insertRecord(dbTableDescriptor* desc, dbAnyReference* ref, 
                              void const* record) 
{ 
    FASTDB_ASSERT(opened);
    beginTransaction(dbExclusiveLock);
    modified = true;
    size_t size = 
        desc->columns->calculateRecordSize((byte*)record, desc->fixedSize);

    dbTable* table = (dbTable*)getRow(desc->tableId);
#ifdef AUTOINCREMENT_SUPPORT
    desc->autoincrementCount = table->count + 1;
#endif
    size_t nRows = table->nRows+1;
    oid_t oid = allocateRow(desc->tableId, size);
    byte* src = (byte*)record;
    byte* dst = (byte*)getRow(oid);    
    desc->columns->storeRecordFields(dst, src, desc->fixedSize, dbFieldDescriptor::Insert);
    if (logger != NULL) { 
        if (!logger->insert(desc, oid, (dbRecord*)dst, record)) { 
            freeRow(desc->tableId, oid);                 
            handleError(RejectedByTransactionLogger);
        }
    }            
    ref->oid = oid;
    dbFieldDescriptor* fd;
    if (inverseReferencesUpdate) { 
        for (fd = desc->inverseFields; fd != NULL; fd = fd->nextInverseField) { 
            if (fd->type == dbField::tpArray) { 
                dbAnyArray* arr = (dbAnyArray*)(src + fd->appOffs);
                int n = (int)arr->length();
                oid_t* refs = (oid_t*)arr->base();
                while (--n >= 0) { 
                    if (refs[n] != 0) {
                        insertInverseReference(fd, oid, refs[n]);
                    }
                }
            } else { 
                oid_t ref = *(oid_t*)(src + fd->appOffs);
                if (ref != 0) { 
                    insertInverseReference(fd, oid, ref);
                }
            }
        }    
    }       
    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField) {
        dbHashTable::insert(this, fd, oid, nRows);
    }
    for (fd = desc->indexedFields; fd != NULL; fd = fd->nextIndexedField) { 
        if (fd->type == dbField::tpRectangle) { 
            dbRtree::insert(this, fd->tTree, oid, fd->dbsOffs);
        } else { 
            dbTtree::insert(this, fd->tTree, oid, fd->type, (int)fd->dbsSize, fd->_comparator, fd->dbsOffs);
        }
    }
}


inline void dbDatabase::extend(offs_t size)
{
    size_t oldSize = header->size;

    if (size > header->used) { 
        header->used = size;
    }
    if (size > oldSize) { 
#ifdef DISKLESS_CONFIGURATION
        handleError(FileLimitExeeded);
#endif
        if (fileSizeLimit != 0 && size > fileSizeLimit) {
            handleError(FileLimitExeeded);
        }
        dbDatabaseThreadContext* ctx = threadContext.get();
        FASTDB_ASSERT(ctx != NULL);
        if (ctx->mutatorCSLocked && !ctx->writeAccess) { 
            beginTransaction(dbCommitLock);
        }
        if (oldSize*2 > size) { 
            size_t newSize = 64*1024;
            while (newSize < size) { 
                newSize <<= 1;
            }
            if (fileSizeLimit == 0 || newSize <= fileSizeLimit) { 
                if (offs_t(newSize) == 0) { // overflow
                    handleError(FileLimitExeeded);
                }
                size = (offs_t)newSize;
            } else if (fileSizeLimit > size) { 
                size = (offs_t)fileSizeLimit;
            }
        } 
        TRACE_MSG(("Extend database file from %ld to %ld bytes\n",
                   (long)header->size, (long)size));
        header->size = size;
        version = ++monitor->version;
        _stprintf(databaseName + databaseNameLen, _T(".%d"), version);
        int status = file.setSize(size, databaseName);
        // file.markAsDirty(oldSize, size - oldSize);
        byte* addr = (byte*)file.getAddr();
        size_t shift = addr - baseAddr;
        if (shift != 0) { 
            size_t base = (size_t)baseAddr; 
            for (dbL2List* cursor = ctx->cursors.next; 
                 cursor != &ctx->cursors;
                 cursor = cursor->next)
            {
                ((dbAnyCursor*)cursor)->adjustReferences(base, oldSize, shift);
            }
            baseAddr = addr;
            index[0] = (offs_t*)((char*)index[0] + shift);
            index[1] = (offs_t*)((char*)index[1] + shift);
            currIndex = (offs_t*)((char*)currIndex + shift);
            header = (dbHeader*)addr;
        }
        if (status != dbFile::ok) {
            header->size = oldSize; 
            handleError(FileError, "Failed to extend file", status);
        }
    }
}


inline bool dbDatabase::wasReserved(offs_t pos, size_t size) 
{
    for (dbLocation* location = reservedChain; location != NULL; location = location->next) { 
        if (pos - location->pos < location->size || location->pos - pos < size) {
            return true;
        }
    }
    return false;
}

inline dbDatabase::dbLocation::dbLocation(dbDatabase* dbs, offs_t locPos, size_t locSize)
  : pos(locPos), size(locSize), next(dbs->reservedChain), db(dbs)
{
    db->reservedChain = this;
    db->reservedChainLength += 1;
}

inline dbDatabase::dbLocation::~dbLocation()
{
    assert(db->reservedChain == this);
    db->reservedChain = next;
    db->reservedChainLength -= 1;
}

inline int ilog2(offs_t val) 
{ 
    int    log;
    size_t pow;
    for (log = dbAllocationQuantumBits, pow = dbAllocationQuantum; pow <= val; pow <<= 1, log += 1);
    return log-1;
}    

void dbDatabase::getMemoryStatistic(dbMemoryStatistic& stat)
{
    stat.free = 0;
    stat.used = 0;
    stat.nHoles = 0;
    stat.minHoleSize = (offs_t)header->size;
    stat.maxHoleSize = 0;
    for (int l = 0; l < dbDatabaseOffsetBits; l++) { 
        stat.nHolesOfSize[l] = 0;
    }
    offs_t holeSize = 0;

    for (oid_t i = dbBitmapId; i < dbBitmapId + dbBitmapPages && currIndex[i] != dbFreeHandleMarker; i++){
        register byte* bitmap = get(i);
        for (size_t j = 0; j < dbPageSize; j++) { 
            unsigned mask = bitmap[j];
            int count = 0;
            while (mask != 0) { 
                while ((mask & 1) == 0) { 
                    holeSize += 1;
                    mask >>= 1;
                    count += 1;
                }
                if (holeSize > 0) { 
                    offs_t size = holeSize << dbAllocationQuantumBits;
                    if (size > stat.maxHoleSize) {
                        stat.maxHoleSize = size;
                    }
                    if (size < stat.minHoleSize) {
                        stat.minHoleSize = size;
                    }
                    stat.nHolesOfSize[ilog2(size)] += 1;
                    stat.free += size;
                    stat.nHoles += 1;
                    holeSize = 0;
                }
                while ((mask & 1) != 0) { 
                    stat.used += dbAllocationQuantum;
                    count += 1;
                    mask >>= 1;
                }
            }
            holeSize += 8 - count;
        }
    }
    if (holeSize > 0) { 
        offs_t size = holeSize << dbAllocationQuantumBits;
        if (size > stat.maxHoleSize) {
            stat.maxHoleSize = size;
        }
        if (size < stat.minHoleSize) {
            stat.minHoleSize = size;
        }
        stat.nHolesOfSize[ilog2(size)] += 1;
        stat.free += size;
        stat.nHoles += 1;
    }       
}

bool dbDatabase::isFree(offs_t pos, int objBitSize)
{
    size_t quantNo = pos / dbAllocationQuantum;
    oid_t  pageId = (oid_t)(dbBitmapId + quantNo / (dbPageSize*8));
    int    offs = quantNo % (dbPageSize*8) / 8;
    byte*  p = put(pageId) + offs;
    int    bitOffs = quantNo & 7;
            
    if (objBitSize > 8 - bitOffs) { 
        objBitSize -= 8 - bitOffs;
        if ((*p++ & (-1 << bitOffs)) != 0) { 
            return false;
        }
        offs += 1;
        while ((size_t)(objBitSize + offs*8) > dbPageSize*8) { 
            int n = dbPageSize - offs;
            while (--n >= 0) { 
                if (*p++ != 0) { 
                    return false;
                }
            }
            p = put(++pageId);
            objBitSize -= (dbPageSize - offs)*8;
            offs = 0;
        }
        while ((objBitSize -= 8) > 0) { 
            if (*p++ != 0) { 
                return false;
            }
        }
        return (*p & ((1 << (objBitSize + 8)) - 1)) == 0;
    } else { 
        return (*p & (((1 << objBitSize) - 1) << bitOffs)) == 0;
    }
}

void dbDatabase::markAsAllocated(offs_t pos, int objBitSize)
{
    size_t quantNo = pos / dbAllocationQuantum;
    oid_t  pageId = (oid_t)(dbBitmapId + quantNo / (dbPageSize*8));
    int    offs = quantNo % (dbPageSize*8) / 8;
    byte*  p = put(pageId) + offs;
    int    bitOffs = quantNo & 7;
    
    if (objBitSize > 8 - bitOffs) { 
        objBitSize -= 8 - bitOffs;
        *p++ |= -1 << bitOffs;
        offs += 1;
        while ((size_t)(objBitSize + offs*8) > dbPageSize*8) { 
            memset(p, 0xFF, dbPageSize - offs);
            p = put(++pageId);
            objBitSize -= (dbPageSize - offs)*8;
            offs = 0;
        }
        while ((objBitSize -= 8) > 0) { 
            *p++ = 0xFF;
        }
        *p |= (1 << (objBitSize + 8)) - 1;
    } else { 
        *p |= ((1 << objBitSize) - 1) << bitOffs; 
    }
}


offs_t dbDatabase::allocate(size_t size, oid_t oid)
{
     static byte const firstHoleSize [] = {
        8,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
        5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
        6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
        5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
        7,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
        5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
        6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,
        5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0
    };
    static byte const lastHoleSize [] = {
        8,7,6,6,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };
    static byte const maxHoleSize [] = {
        8,7,6,6,5,5,5,5,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
        5,4,3,3,2,2,2,2,3,2,2,2,2,2,2,2,4,3,2,2,2,2,2,2,3,2,2,2,2,2,2,2,
        6,5,4,4,3,3,3,3,3,2,2,2,2,2,2,2,4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,1,
        5,4,3,3,2,2,2,2,3,2,1,1,2,1,1,1,4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,1,
        7,6,5,5,4,4,4,4,3,3,3,3,3,3,3,3,4,3,2,2,2,2,2,2,3,2,2,2,2,2,2,2,
        5,4,3,3,2,2,2,2,3,2,1,1,2,1,1,1,4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,1,
        6,5,4,4,3,3,3,3,3,2,2,2,2,2,2,2,4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,1,
        5,4,3,3,2,2,2,2,3,2,1,1,2,1,1,1,4,3,2,2,2,1,1,1,3,2,1,1,2,1,1,0
    };
    static byte const maxHoleOffset [] = {
        0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,0,1,5,5,5,5,5,5,0,5,5,5,5,5,5,5,
        0,1,2,2,0,3,3,3,0,1,6,6,0,6,6,6,0,1,2,2,0,6,6,6,0,1,6,6,0,6,6,6,
        0,1,2,2,3,3,3,3,0,1,4,4,0,4,4,4,0,1,2,2,0,1,0,3,0,1,0,2,0,1,0,5,
        0,1,2,2,0,3,3,3,0,1,0,2,0,1,0,4,0,1,2,2,0,1,0,3,0,1,0,2,0,1,0,7,
        0,1,2,2,3,3,3,3,0,4,4,4,4,4,4,4,0,1,2,2,0,5,5,5,0,1,5,5,0,5,5,5,
        0,1,2,2,0,3,3,3,0,1,0,2,0,1,0,4,0,1,2,2,0,1,0,3,0,1,0,2,0,1,0,6,
        0,1,2,2,3,3,3,3,0,1,4,4,0,4,4,4,0,1,2,2,0,1,0,3,0,1,0,2,0,1,0,5,
        0,1,2,2,0,3,3,3,0,1,0,2,0,1,0,4,0,1,2,2,0,1,0,3,0,1,0,2,0,1,0,0
    };

    setDirty();

    size = DOALIGN(size, dbAllocationQuantum);
    int objBitSize = (int)(size >> dbAllocationQuantumBits);
    offs_t pos;    
    oid_t i, firstPage, lastPage;
    int holeBitSize = 0;
    register int    alignment = size & (dbPageSize-1);
    const size_t    inc = dbPageSize/dbAllocationQuantum/8;
    register size_t offs;

    const int  pageBits = dbPageSize*8;
    int        holeBeforeFreePage  = 0;
    oid_t      freeBitmapPage = 0;

    lastPage = dbBitmapId + dbBitmapPages;
    allocatedSize += (offs_t)size;

    if (alignment == 0) {
        if (reservedChainLength > dbAllocRecursionLimit) { 
            firstPage = bitmapEnd;    
            if (firstPage >= lastPage) { 
                firstPage = lastPage;
            } else { 
                while (firstPage < lastPage && currIndex[firstPage] != dbFreeHandleMarker) { 
                    firstPage += 1;
                }
            }
            bitmapEnd = --firstPage; 
            offs = 0;
        } else { 
            firstPage = (oid_t)currPBitmapPage;
            offs = DOALIGN(currPBitmapOffs, inc);
        }
    } else {
        int retries = -1;
        do { 
            retries += 1;
            pos = fixedSizeAllocator.allocate(size);
        } while (pos != 0 && (wasReserved(pos, size) || !isFree(pos, objBitSize)));
        fixedSizeAllocator.retries = retries;

        if (pos != 0) { 
            {
                dbLocation location(this, pos, size);
                if (oid != 0) {
                    offs_t prev = currIndex[oid];
                    memcpy(baseAddr+pos, 
                       baseAddr+(prev&~dbInternalObjectMarker), size);
                    currIndex[oid] = (prev & dbInternalObjectMarker) + pos;
                }
                markAsAllocated(pos, objBitSize);
            }
            file.markAsDirty(pos, size);
            return pos;
        }

        firstPage = (oid_t)currRBitmapPage;
        offs = currRBitmapOffs;
    }
        
    while (true) { 
        if (alignment == 0) {
            // allocate page object
            for (i = firstPage; i < lastPage && currIndex[i] != dbFreeHandleMarker; i++){
                int spaceNeeded = objBitSize - holeBitSize < pageBits
                    ? objBitSize - holeBitSize : pageBits;
                if (bitmapPageAvailableSpace[i] <= spaceNeeded) {
                    holeBitSize = 0;
                    offs = 0;
                    continue;
                }
                register byte* begin = get(i);
                size_t startOffs = offs;
                while (offs < dbPageSize) {
                    if (begin[offs++] != 0) {
                        offs = DOALIGN(offs, inc);
                        holeBitSize = 0;
                    } else if ((holeBitSize += 8) == objBitSize) {
                        pos = (offs_t)(((offs_t(i-dbBitmapId)*dbPageSize + offs)*8
                               - holeBitSize) << dbAllocationQuantumBits);
                        if (wasReserved(pos, size)) { 
                            offs += objBitSize >> 3;
                            startOffs = offs = DOALIGN(offs, inc);
                            holeBitSize = 0;
                            continue;
                        }       
                        extend(pos + (offs_t)size);
                        {
                            dbLocation location(this, pos, size);
                            currPBitmapPage = i;
                            currPBitmapOffs = offs;
                            if (oid != 0) {
                                offs_t prev = currIndex[oid];
                                memcpy(baseAddr+pos, 
                                   baseAddr+(prev&~dbInternalObjectMarker), size);
                                currIndex[oid] = (prev & dbInternalObjectMarker) + pos;
                            }
                            begin = put(i);
                            size_t holeBytes = holeBitSize >> 3;
                            if (holeBytes > offs) {
                                memset(begin, 0xFF, offs);
                                holeBytes -= offs;
                                begin = put(--i);
                                offs = dbPageSize;
                            }
                            while (holeBytes > dbPageSize) {
                                memset(begin, 0xFF, dbPageSize);
                                holeBytes -= dbPageSize;
                                bitmapPageAvailableSpace[i] = 0;
                                begin = put(--i);
                            }
                            memset(&begin[offs-holeBytes], 0xFF, holeBytes);
                        }
                        file.markAsDirty(pos, size);
                        return pos;
                    }
                }
                if (startOffs == 0 && holeBitSize == 0
                    && spaceNeeded < bitmapPageAvailableSpace[i])
                {
                    bitmapPageAvailableSpace[i] = spaceNeeded;
                }
                offs = 0;
            }
        } else {
            for (i=firstPage; i<lastPage && currIndex[i] != dbFreeHandleMarker; i++){
                int spaceNeeded = objBitSize - holeBitSize < pageBits 
                    ? objBitSize - holeBitSize : pageBits;
                if (bitmapPageAvailableSpace[i] <= spaceNeeded) {
                    holeBitSize = 0;
                    offs = 0;
                    continue;
                }
                register byte* begin = get(i);
                size_t startOffs = offs;

                while (offs < dbPageSize) { 
                    int mask = begin[offs]; 
                    if (holeBitSize + firstHoleSize[mask] >= objBitSize) { 
                        pos = (offs_t)(((offs_t(i-dbBitmapId)*dbPageSize + offs)*8 
                               - holeBitSize) << dbAllocationQuantumBits);
                        if (wasReserved(pos, size)) {                       
                            startOffs = offs += (objBitSize + 7) >> 3;
                            holeBitSize = 0;
                            continue;
                        }       
                        extend(pos + (offs_t)size);
                        {
                            dbLocation location(this, pos, size);
                            currRBitmapPage = i;
                            currRBitmapOffs = offs;
                            if (oid != 0) { 
                                offs_t prev = currIndex[oid];
                                memcpy(baseAddr+pos, 
                                       baseAddr+(prev&~dbInternalObjectMarker), size);
                                currIndex[oid] = (prev & dbInternalObjectMarker) + pos;
                            }
                            begin = put(i);
                            begin[offs] |= (1 << (objBitSize - holeBitSize)) - 1; 
                            if (holeBitSize != 0) { 
                                if (size_t(holeBitSize) > offs*8) { 
                                    memset(begin, 0xFF, offs);
                                    holeBitSize -= (int)offs*8;
                                    begin = put(--i);
                                    offs = dbPageSize;
                                }
                                while (holeBitSize > pageBits) { 
                                    memset(begin, 0xFF, dbPageSize);
                                    holeBitSize -= pageBits;
                                    bitmapPageAvailableSpace[i] = 0;
                                    begin = put(--i);
                                }
                                while ((holeBitSize -= 8) > 0) { 
                                    begin[--offs] = 0xFF; 
                                }
                                begin[offs-1] |= ~((1 << -holeBitSize) - 1);
                            }
                        }
                        file.markAsDirty(pos, size);
                        return pos;
                    } else if (maxHoleSize[mask] >= objBitSize) { 
                        int holeBitOffset = maxHoleOffset[mask];
                        pos = (offs_t)(((offs_t(i-dbBitmapId)*dbPageSize + offs)*8 + 
                               holeBitOffset) << dbAllocationQuantumBits);
                        if (wasReserved(pos, size)) { 
                            startOffs = offs += (objBitSize + 7) >> 3;
                            holeBitSize = 0;
                            continue;
                        }       
                        extend(pos + (offs_t)size);
                        {
                            dbLocation location(this, pos, size);
                            currRBitmapPage = i;
                            currRBitmapOffs = offs;
                            if (oid != 0) { 
                                offs_t prev = currIndex[oid];
                                memcpy(baseAddr+pos, 
                                       baseAddr+(prev&~dbInternalObjectMarker), size);
                                currIndex[oid] = (prev & dbInternalObjectMarker) + pos;
                            }
                            begin = put(i);
                            begin[offs] |= ((1 << objBitSize) - 1) << holeBitOffset;
                        }
                        file.markAsDirty(pos, size);
                        return pos;
                    }
                    offs += 1;
                    if (lastHoleSize[mask] == 8) { 
                        holeBitSize += 8;
                    } else { 
                        holeBitSize = lastHoleSize[mask];
                    }
                }
                if (startOffs == 0 && holeBitSize == 0 
                    && spaceNeeded < bitmapPageAvailableSpace[i]) 
                { 
                    bitmapPageAvailableSpace[i] = spaceNeeded;
                }
                offs = 0;
            }
        }
        if (firstPage == dbBitmapId || reservedChainLength > dbAllocRecursionLimit) { 
            if (freeBitmapPage > i) { 
                i = freeBitmapPage;
                holeBitSize = holeBeforeFreePage;
            }
            if (i == dbBitmapId + dbBitmapPages) { 
                handleError(OutOfMemoryError, NULL, (int)size);
            }
            FASTDB_ASSERT(currIndex[i] == dbFreeHandleMarker);

            size_t extension = (size > extensionQuantum) 
                             ? size : extensionQuantum;
            int morePages = 
                (int)((extension + dbPageSize*(dbAllocationQuantum*8-1) - 1)
                / (dbPageSize*(dbAllocationQuantum*8-1)));

            if (size_t(i + morePages) > dbBitmapId + dbBitmapPages) { 
                morePages =  
                    (int)((size + dbPageSize*(dbAllocationQuantum*8-1) - 1)
                          / (dbPageSize*(dbAllocationQuantum*8-1)));
                if (size_t(i + morePages) > dbBitmapId + dbBitmapPages) { 
                    handleError(OutOfMemoryError, NULL, (int)size);
                }
            }
            objBitSize -= holeBitSize;
            int skip = DOALIGN(objBitSize, dbPageSize/dbAllocationQuantum);
            pos = (offs_t(i-dbBitmapId) << (dbPageBits+dbAllocationQuantumBits+3)) 
                + (skip << dbAllocationQuantumBits);
            extend(pos + morePages*dbPageSize);
            file.markAsDirty(pos, morePages*dbPageSize);
            memset(baseAddr + pos, 0, morePages*dbPageSize);
            memset(baseAddr + pos, 0xFF, objBitSize>>3);
            *(baseAddr + pos + (objBitSize>>3)) = (1 << (objBitSize&7))-1;
            memset(baseAddr + pos + (skip>>3), 0xFF, 
                   morePages*(dbPageSize/dbAllocationQuantum/8));

            oid_t j = i;
            while (--morePages >= 0) { 
                monitor->dirtyPagesMap[j/dbHandlesPerPage/32] 
                    |= 1 << int(j/dbHandlesPerPage & 31);
                currIndex[j++] = pos + dbPageObjectMarker;
                pos += dbPageSize;
            }
            j = i + objBitSize / pageBits; 
            if (alignment != 0) {
                currRBitmapPage = j;
                currRBitmapOffs = 0;
            } else { 
                currPBitmapPage = j;
                currPBitmapOffs = 0;
            }
            while (j > i) { 
                bitmapPageAvailableSpace[--j] = 0;
            }
            
            pos = (offs_t(i-dbBitmapId)*dbPageSize*8 - holeBitSize)
                << dbAllocationQuantumBits;
            if (oid != 0) { 
                offs_t prev = currIndex[oid];
                memcpy(baseAddr + pos, 
                       baseAddr + (prev & ~dbInternalObjectMarker), size);
                currIndex[oid] = (prev & dbInternalObjectMarker) + pos;
            }

            if (holeBitSize != 0) { 
                dbLocation location(this, pos, size);
                while (holeBitSize > pageBits) { 
                    holeBitSize -= pageBits;
                    memset(put(--i), 0xFF, dbPageSize);
                    bitmapPageAvailableSpace[i] = 0;
                }
                byte* cur = (byte*)put(--i) + dbPageSize;
                while ((holeBitSize -= 8) > 0) { 
                    *--cur = 0xFF; 
                }
                *(cur-1) |= ~((1 << -holeBitSize) - 1);
            }
            file.markAsDirty(pos, size);
            return pos;
        } 
        freeBitmapPage = i;
        holeBeforeFreePage = holeBitSize;
        holeBitSize = 0;
        lastPage = firstPage + 1;
        firstPage = dbBitmapId;
        offs = 0;
    }
} 

void dbDatabase::deallocate(offs_t pos, size_t size)
{
    FASTDB_ASSERT(pos != 0 && (pos & (dbAllocationQuantum-1)) == 0);
    size_t quantNo = pos / dbAllocationQuantum;
    int    objBitSize = (int)((size+dbAllocationQuantum-1) / dbAllocationQuantum);
    oid_t  pageId = (oid_t)(dbBitmapId + quantNo / (dbPageSize*8));
    size_t offs = quantNo % (dbPageSize*8) / 8;
    byte*  p = put(pageId) + offs;
    int    bitOffs = quantNo & 7;

    allocatedSize -= objBitSize*dbAllocationQuantum;
    if ((deallocatedSize += objBitSize*dbAllocationQuantum) >= freeSpaceReuseThreshold)
    {
        deallocatedSize = 0;
        currRBitmapPage = currPBitmapPage = dbBitmapId;
        currRBitmapOffs = currPBitmapOffs = 0;
    } else { 
        if ((size_t(pos) & (dbPageSize-1)) == 0 && size >= dbPageSize) {
            if (pageId == currPBitmapPage && offs < currPBitmapOffs) {
                currPBitmapOffs = offs;
            }
        } else {
            if (fixedSizeAllocator.deallocate(pos, size)) { 
                deallocatedSize -= objBitSize*dbAllocationQuantum;
            } else if (pageId == currRBitmapPage && offs < currRBitmapOffs) { 
                currRBitmapOffs = offs;
            } 
        }
    }
    bitmapPageAvailableSpace[pageId] = INT_MAX;
 
    if (objBitSize > 8 - bitOffs) { 
        objBitSize -= 8 - bitOffs;
        *p++ &= (1 << bitOffs) - 1;
        offs += 1;
        while (objBitSize + offs*8 > dbPageSize*8) { 
            memset(p, 0, dbPageSize - offs);
            p = put(++pageId);
            bitmapPageAvailableSpace[pageId] = INT_MAX;
            objBitSize -= (int)((dbPageSize - offs)*8);
            offs = 0;
        }
        while ((objBitSize -= 8) > 0) { 
            *p++ = 0;
        }
        *p &= ~((1 << (objBitSize + 8)) - 1);
    } else { 
        *p &= ~(((1 << objBitSize) - 1) << bitOffs); 
    }
}

void dbDatabase::cloneBitmap(offs_t pos, size_t size)
{
    size_t quantNo = pos / dbAllocationQuantum;
    int    objBitSize = (int)((size+dbAllocationQuantum-1) / dbAllocationQuantum);
    oid_t  pageId = (oid_t)(dbBitmapId + quantNo / (dbPageSize*8));
    size_t offs = quantNo % (dbPageSize*8) / 8;
    int    bitOffs = quantNo & 7;

    put(pageId); 
    if (objBitSize > 8 - bitOffs) { 
        objBitSize -= 8 - bitOffs;
        offs += 1;
        while (objBitSize + offs*8 > dbPageSize*8) { 
            put(++pageId);
            objBitSize -= (int)((dbPageSize - offs)*8);
            offs = 0;
        }
    }
}


void dbDatabase::setDirty() 
{
    if (!header->dirty) { 
        if (accessType == dbReadOnly) { 
            handleError(DatabaseReadOnly, "Attempt to modify readonly database"); 
        }
        header->dirty = true;
        file.markAsDirty(0, sizeof(dbHeader));
        file.flush(true);
    }
    monitor->modified = true;
    modified = true;
}

void dbDatabase::recoverFreeOidList()
{
    beginTransaction(dbExclusiveLock);
    setDirty();
    
    oid_t next = 0;
    for (oid_t oid = dbFirstUserId; oid < currIndexSize; oid++) { 
        if (currIndex[oid] & dbFreeHandleMarker) { 
            size_t i = oid / dbHandlesPerPage;
            monitor->dirtyPagesMap[i >> 5] |= 1 << (i & 31);
            currIndex[oid] = next + dbFreeHandleMarker;
            next = oid;
        } 
    }
    header->root[1-header->curr].freeList = next;
}

oid_t dbDatabase::allocateId(int n) 
{
    setDirty();

    oid_t oid;
    int curr = 1-header->curr;
    if (n == 1) { 
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
        if (monitor->sessionFreeList[curr].tail != 0) { 
            if ((oid = monitor->sessionFreeList[curr].head) != 0) { 
                currIndex[monitor->sessionFreeList[curr].tail] = currIndex[oid];
                unsigned i = monitor->sessionFreeList[curr].tail / dbHandlesPerPage;
                monitor->dirtyPagesMap[i >> 5] |= 1 << (i & 31);
        
                monitor->sessionFreeList[curr].head = (oid_t)(currIndex[oid] - dbFreeHandleMarker);
                i = oid / dbHandlesPerPage;
                monitor->dirtyPagesMap[i >> 5] |= 1 << (i & 31);                
                return oid;
            }
        }
 #endif
        if ((oid = header->root[curr].freeList) != 0) { 
            header->root[curr].freeList = (oid_t)(currIndex[oid] - dbFreeHandleMarker);
            size_t i = oid / dbHandlesPerPage;
            monitor->dirtyPagesMap[i >> 5] |= 1 << (i & 31);
            return oid;
        }
    }
    if (currIndexSize + n > header->root[curr].indexSize) {
        size_t oldIndexSize = header->root[curr].indexSize;
        size_t newIndexSize = oldIndexSize * 2;
        while (newIndexSize < oldIndexSize + n) { 
            newIndexSize = newIndexSize*2;
        }
        TRACE_MSG(("Extend index size from %ld to %ld\n", 
                   oldIndexSize, newIndexSize));
        offs_t newIndex = allocate(newIndexSize*sizeof(offs_t));
        offs_t oldIndex = header->root[curr].index;
        memcpy(baseAddr + newIndex, currIndex, currIndexSize*sizeof(offs_t));
        currIndex = index[curr] = (offs_t*)(baseAddr + newIndex);
        header->root[curr].index = newIndex;
        header->root[curr].indexSize = (oid_t)newIndexSize;
        deallocate(oldIndex, oldIndexSize*sizeof(offs_t));
    }
    oid = (oid_t)currIndexSize;
    header->root[curr].indexUsed = (oid_t)(currIndexSize += n);
    return oid;
}
            
void dbDatabase::freeId(oid_t oid, int n) 
{
    int curr = 1-header->curr;
    oid_t freeList = header->root[curr].freeList;
    while (--n >= 0) {
        size_t i = oid / dbHandlesPerPage;
        monitor->dirtyPagesMap[i >> 5] |= 1 << (i & 31);
        currIndex[oid] = freeList + dbFreeHandleMarker;
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
        if (monitor->sessionFreeList[curr].tail == 0) { 
            monitor->sessionFreeList[curr].tail = oid;
            monitor->sessionFreeList[curr].head = freeList;
        }
#endif
        freeList = oid++;
    }
    header->root[curr].freeList = freeList;
}
    
#ifdef AUTO_DETECT_PROCESS_CRASH

void dbDatabase::watchDogThread(dbWatchDogContext* ctx) { 
    dbMutex* mutex = ctx->mutex;
    if (ctx->watchDog.watch()) { 
        mutex->lock();
        if (ctx->db != NULL) { 
            ctx->db->cs.enter();
            ctx->db->revokeLock(ctx->clientId);
            ctx->db->cs.leave();
        }
        ctx->watchDog.close();
    } else {
        mutex->lock();
    }         
    bool isEmpty = false;
    dbDatabase* db = ctx->db;
    if (db != NULL) { 
        db->cs.enter();  
        delete ctx;
        db->cs.leave();                
    } else { 
        isEmpty = ctx->isEmpty();
        delete ctx;
    }
    mutex->unlock();
    if (isEmpty) { 
        delete mutex;
    }
}

void dbDatabase::revokeLock(long clientId) 
{
    TRACE_MSG(("Revoke lock: writers %d, readers %d, lock owner %d, crashed process %ld\n",
               monitor->nWriters, monitor->nReaders, monitor->exclusiveLockOwner, clientId));
    if (monitor->nWriters != 0 && monitor->exclusiveLockOwner == clientId) { 
        if (accessType != dbReadOnly && accessType != dbConcurrentRead) { 
            TRACE_MSG(("Revoke exclusive lock, start recovery\n"));
            checkVersion();
            recovery();
            TRACE_MSG(("Recovery completed\n"));
            monitor->exclusiveLockOwner = 0;
            monitor->nWriters -= 1;
            monitor->ownerPid.clear();
            FASTDB_ASSERT(monitor->nWriters == 0 && !monitor->waitForUpgrade);
            if (monitor->nWaitWriters != 0) { 
                monitor->nWaitWriters -= 1;
                monitor->nWriters = 1;
                writeSem.signal();
            } else if (monitor->nWaitReaders != 0) { 
                monitor->nReaders = monitor->nWaitReaders;
                monitor->nWaitReaders = 0;
                readSem.signal(monitor->nReaders);
            }
        } else { 
            handleError(Deadlock, "Owner of exclusive database lock is crashed");
        }
    } else { 
        int nReaders = monitor->nReaders; 
        for (int i = 0; i < nReaders; i++) { 
            if (monitor->sharedLockOwner[i] == clientId) { 
                TRACE_MSG(("Revoke shared lock\n"));
                while (++i < nReaders) { 
                    monitor->sharedLockOwner[i-1] = monitor->sharedLockOwner[i];
                }
                monitor->sharedLockOwner[i-1] = 0;
                monitor->nReaders -= 1;
                if (monitor->nReaders == 1 && monitor->waitForUpgrade) { 
                    FASTDB_ASSERT(monitor->nWriters == 0);
#if defined(AUTO_DETECT_PROCESS_CRASH) || DEBUG_LOCKS
                    removeLockOwner(monitor->upgradeId);
                    monitor->upgradeId = 0;
#endif
                    monitor->waitForUpgrade = false;
                    monitor->nWaitWriters -= 1;
                    monitor->nWriters = 1;
                    monitor->nReaders = 0;
                    upgradeSem.signal();
                } else if (monitor->nReaders == 0) { 
                    if (monitor->nWaitWriters != 0) { 
                        FASTDB_ASSERT(monitor->nWriters == 0 && !monitor->waitForUpgrade);
                        monitor->nWaitWriters -= 1;
                        monitor->nWriters = 1;
                        writeSem.signal();
                    }
                } 
                break;
            }
        }
    }
}


void dbDatabase::startWatchDogThreads()
{
    while (maxClientId < monitor->clientId) { 
        long id = ++maxClientId;
        if (id != selfId) { 
            _stprintf(databaseName + databaseNameLen, _T(".pid.%ld"), id);
            dbWatchDogContext* ctx = new dbWatchDogContext();
            if (ctx->watchDog.open(databaseName)) {
                watchDogThreadContexts.link(ctx);
                ctx->clientId = id;
                ctx->db = this;
                ctx->mutex = watchDogMutex;
                ctx->thread.create((dbThread::thread_proc_t)watchDogThread, ctx);
            } else { 
                revokeLock(id);
                delete ctx;
            }
        }
    }
}
#endif


void dbDatabase::addLockOwner() 
{
    int nReaders = monitor->nReaders;
    FASTDB_ASSERT(nReaders <= dbMaxReaders && nReaders > 0);    
    while (monitor->sharedLockOwner[--nReaders] != 0) { 
        FASTDB_ASSERT(nReaders != 0);
    }
#if DEBUG_LOCKS
    long selfId = dbThread::getCurrentThreadId();
#endif
    monitor->sharedLockOwner[nReaders] = selfId;
    monitor->sharedLockOwner[nReaders] = selfId;
}

void dbDatabase::removeLockOwner(long selfId) 
{
    long id = 0;
    int i = monitor->nReaders;
    do { 
        FASTDB_ASSERT(i > 0);
        long nextId = monitor->sharedLockOwner[--i];
        monitor->sharedLockOwner[i] = id;
        id = nextId;
    } while (id != selfId);            
}

bool dbDatabase::isInWriteTransaction()
{
    dbDatabaseThreadContext* ctx = threadContext.get();
    if (ctx == NULL ||
           (!ctx->writeAccess && !ctx->readAccess && !ctx->mutatorCSLocked))
    {
		return false;
	}
	if (accessType != dbConcurrentUpdate) {
		return ctx->writeAccess != 0;
    }
	return true;
}

bool dbDatabase::isCommitted()
{
    dbDatabaseThreadContext* ctx = threadContext.get();
    return ctx == NULL ||
           (!ctx->writeAccess && !ctx->readAccess && !ctx->mutatorCSLocked);
}

bool dbDatabase::isUpdateTransaction()
{
    dbDatabaseThreadContext* ctx = threadContext.get();
    return ctx != NULL && ctx->isMutator;
}

bool dbDatabase::isAttached() 
{
    return threadContext.get() != NULL;
}

bool dbDatabase::beginTransaction(dbLockType lockType)
{
    dbDatabaseThreadContext* ctx = threadContext.get();
    bool delayedCommitForced = false;
#if DEBUG_LOCKS
    long selfId = dbThread::getCurrentThreadId();
#endif
    if (commitDelay != 0 && lockType != dbCommitLock) { 
        dbCriticalSection cs(delayedCommitStopTimerMutex);
        if (monitor->delayedCommitContext == ctx && ctx->commitDelayed) {
            // skip delayed transaction because this thread is starting new transaction
            monitor->delayedCommitContext = NULL;
            ctx->commitDelayed = false;
            if (commitTimerStarted != 0) { 
                time_t elapsed = time(NULL) - commitTimerStarted;
                if (commitTimeout < elapsed) { 
                    commitTimeout = 0;
                } else { 
                    commitTimeout -= elapsed;               
                }
            }
            delayedCommitStopTimerEvent.signal();
        } else { 
            monitor->forceCommitCount += 1; 
            delayedCommitForced = true;
        }
    }
    
    if (lockType != dbSharedLock) { 
        ctx->isMutator = true;
    }
    if (accessType == dbConcurrentUpdate && lockType != dbCommitLock) { 
        if (!ctx->mutatorCSLocked) { 
            mutatorCS.enter();
            ctx->mutatorCSLocked = true;
#ifdef RECOVERABLE_CRITICAL_SECTION
            if (monitor->modified) {
                // mutatorCS lock was revoked
                monitor->modified = false;
                checkVersion();
                recovery();
            }
#endif
        } else if (!delayedCommitForced) {
            return true;
        }
    } else if (lockType != dbSharedLock) { 
        if (!ctx->writeAccess) { 
//            FASTDB_ASSERT(accessType != dbReadOnly && accessType != dbConcurrentRead);
            cs.enter();
#ifdef AUTO_DETECT_PROCESS_CRASH
            startWatchDogThreads();
#endif
            if (ctx->readAccess) { 
                FASTDB_ASSERT(monitor->nWriters == 0);
                TRACE_MSG(("Attempt to upgrade lock from shared to exclusive can cause deadlock\n"));
                
                if (monitor->nReaders != 1) {
                    if (monitor->waitForUpgrade) { 
                        handleError(Deadlock);
                    }
                    monitor->waitForUpgrade = true;
                    monitor->upgradeId = selfId;
                    monitor->nWaitWriters += 1;
                    cs.leave();
                    
                    if (commitDelay != 0) { 
                        delayedCommitStopTimerEvent.signal();
                    }
                    while (!upgradeSem.wait(waitLockTimeout)
                           || !(monitor->nWriters == 1 && monitor->nReaders == 0)) 
                    { 
                        // There are no writers, so some reader was died
                        cs.enter();
                        unsigned currTime = dbSystem::getCurrentTimeMsec();
                        if (currTime - monitor->lastDeadlockRecoveryTime
                            >= waitLockTimeout)
                        {
                            // Ok, let's try to "remove" this reader
                            monitor->lastDeadlockRecoveryTime = currTime;
                            if (--monitor->nReaders == 1) { 
                                // Looks like we are recovered
#if defined(AUTO_DETECT_PROCESS_CRASH) || DEBUG_LOCKS
                                removeLockOwner(selfId);
#endif
                                monitor->nWriters = 1;
                                monitor->nReaders = 0;
                                monitor->nWaitWriters -= 1;
                                monitor->waitForUpgrade = false;
                                cs.leave();
                                break;
                            }
                        }
                        cs.leave();
                    }
                } else { 
#if defined(AUTO_DETECT_PROCESS_CRASH) || DEBUG_LOCKS
                    removeLockOwner(selfId);
#endif
                    monitor->nWriters = 1;                  
                    monitor->nReaders = 0;
                    cs.leave();
                } 
            } else { 
                if (monitor->nWriters != 0 || monitor->nReaders != 0) { 
                    monitor->nWaitWriters += 1;
                    cs.leave();
                    if (commitDelay != 0) { 
                        delayedCommitStopTimerEvent.signal();
                    }
                    while (!writeSem.wait(waitLockTimeout)
                           || !(monitor->nWriters == 1 && monitor->nReaders == 0))
                    { 
                        cs.enter();
                        unsigned currTime = dbSystem::getCurrentTimeMsec();
                        if (currTime - monitor->lastDeadlockRecoveryTime
                            >= waitLockTimeout)
                        {
                            monitor->lastDeadlockRecoveryTime = currTime;
                            if (monitor->nWriters != 0) { 
                                // writer was died      
                                checkVersion();
                                recovery();
                                monitor->nWriters = 1;
                                monitor->nWaitWriters -= 1;
                                cs.leave();
                                break;
                            } else { 
                                // some reader was died
                                // Ok, let's try to "remove" this reader
                                if (--monitor->nReaders == 0) { 
                                    // Looks like we are recovered
                                    monitor->nWriters = 1;
                                    monitor->nWaitWriters -= 1;
                                    cs.leave();
                                    break;
                                } 
                            }
                        }
                        cs.leave();
                    }
                } else { 
                    monitor->nWriters = 1;
                    cs.leave();
                }
            }
            monitor->ownerPid = ctx->currPid;
#if defined(AUTO_DETECT_PROCESS_CRASH) || DEBUG_LOCKS
            monitor->exclusiveLockOwner = selfId;
#endif
            ctx->writeAccess = true;
        } else { 
            if (monitor->ownerPid != ctx->currPid) { 
                handleError(LockRevoked);
            }
            if (!delayedCommitForced) {
                return true;
            }
        }
    } else { 
        if (!ctx->readAccess && !ctx->writeAccess) { 
            cs.enter();
#ifdef AUTO_DETECT_PROCESS_CRASH
            startWatchDogThreads();
#endif
            if (monitor->nWriters + monitor->nWaitWriters != 0) {
                monitor->nWaitReaders += 1;
                cs.leave();
                if (commitDelay != 0) { 
                    delayedCommitStopTimerEvent.signal();
                }
                while (!readSem.wait(waitLockTimeout)
                       || !(monitor->nWriters == 0 && monitor->nReaders > 0))
                { 
                    cs.enter();
                    unsigned currTime = dbSystem::getCurrentTimeMsec();
                    if (currTime - monitor->lastDeadlockRecoveryTime
                        >= waitLockTimeout)
                    {
                        monitor->lastDeadlockRecoveryTime = currTime;
                        if (monitor->nWriters != 0) { 
                            // writer was died
                            checkVersion();
                            recovery();
                            monitor->nWriters = 0;
                        } else {
                            // some potential writer was died
                            monitor->nWaitWriters -= 1;
                        }
                        monitor->nReaders += 1;
                        monitor->nWaitReaders -= 1;
                        cs.leave();
                        break;
                    }
                    cs.leave();
                }
#if defined(AUTO_DETECT_PROCESS_CRASH) || DEBUG_LOCKS
                cs.enter();
                addLockOwner();
                cs.leave();
#endif
            } else { 
                monitor->nReaders += 1;
#if defined(AUTO_DETECT_PROCESS_CRASH) || DEBUG_LOCKS
                addLockOwner();
#endif
                cs.leave();
            }
            ctx->readAccess = true;
        } else if (!delayedCommitForced) {
            return true;
        }
    }
    if (lockType != dbCommitLock) { 
        if (delayedCommitForced) { 
            dbCriticalSection cs(delayedCommitStopTimerMutex);
            monitor->forceCommitCount -= 1;
        }
        if (!checkVersion()) { 
            return false;
        }
        cs.enter();
        index[0] = (offs_t*)(baseAddr + header->root[0].index);
        index[1] = (offs_t*)(baseAddr + header->root[1].index);
        int curr = monitor->curr;
        if (accessType != dbConcurrentRead) { 
            currIndex = index[1-curr];
            currIndexSize = header->root[1-curr].indexUsed;
            committedIndexSize = header->root[curr].indexUsed;
        } else { 
            currIndex = index[curr];
            currIndexSize = header->root[curr].indexUsed;
            committedIndexSize = header->root[curr].indexUsed;
        }
        cs.leave();
    }
    return true;
}

bool dbDatabase::checkVersion() 
{
    if (version != monitor->version) {
        _stprintf(databaseName+databaseNameLen, _T(".%d"), monitor->version);
        if (version == 0) { 
            if (file.open(fileName, databaseName, fileOpenFlags, monitor->size, false)
                != dbFile::ok) 
            {
                handleError(DatabaseOpenError, "Failed to open database file");
                endTransaction(); // release locks
                return false;
            }
        } else { 
            int status = file.setSize(header->size, databaseName, false);
            if (status != dbFile::ok) { 
                handleError(FileError, "Failed to reopen database file", status);
                endTransaction(); // release locks
                return false;
            }
        }
        version = monitor->version;
        baseAddr = (byte*)file.getAddr();
        header = (dbHeader*)baseAddr;
        if (file.getSize() != header->size) { 
            handleError(FileError, "File size is not matched");
            endTransaction(); // release locks
            return false;
        }
    }
    return true;
}

void dbDatabase::precommit() 
{
    //FASTDB_ASSERT(accessType != dbConcurrentUpdate);
    dbDatabaseThreadContext* ctx = threadContext.get();     
    if (ctx != NULL && (ctx->writeAccess || ctx->readAccess)) { 
        ctx->concurrentId = monitor->concurrentTransId; 
        endTransaction(ctx);
    }
}


void dbDatabase::delayedCommit()
{
    dbCriticalSection cs(delayedCommitStartTimerMutex);
    commitThreadSyncEvent.signal();
    while (!stopDelayedCommitThread) { 
        delayedCommitStartTimerEvent.wait(delayedCommitStartTimerMutex); 
        delayedCommitStartTimerEvent.reset();
        bool deferredCommit = false;
        { 
            dbCriticalSection cs2(delayedCommitStopTimerMutex);
            if (stopDelayedCommitThread || monitor->delayedCommitContext == NULL) {
                continue;
            } else if (monitor->forceCommitCount == 0) {           
                commitTimerStarted = time(NULL);
                deferredCommit = true;
            }
        }
        if (deferredCommit) { 
            delayedCommitStopTimerEvent.wait((unsigned)(commitTimeout*1000));
            delayedCommitStopTimerEvent.reset();
        }
        { 
            dbCriticalSection cs2(delayedCommitStopTimerMutex);
            dbDatabaseThreadContext* ctx = monitor->delayedCommitContext;
            if (ctx != NULL) { 
                commitTimeout = commitDelay;
                monitor->delayedCommitContext = NULL;
                threadContext.set(ctx);
                commit(ctx);
                ctx->commitDelayed = false;
                if (ctx->removeContext) { 
                    dbCriticalSection cs(threadContextListMutex);
                    delete ctx;
                }
            }
        }
    }
}   

void dbDatabase::waitTransactionAcknowledgement()
{
}

void dbDatabase::commit() 
{
    dbDatabaseThreadContext* ctx = threadContext.get();     
    if (ctx != NULL && !ctx->commitDelayed) {
        if (ctx->writeAccess) { 
            if (monitor->ownerPid != ctx->currPid) { 
                handleError(LockRevoked);
            }
        }
        cs.enter();
        bool hasSomethingToCommit = modified && !monitor->commitInProgress 
            && (monitor->uncommittedChanges || ctx->writeAccess || ctx->mutatorCSLocked || ctx->concurrentId == monitor->concurrentTransId);
        cs.leave();
        if (hasSomethingToCommit) { 
            if (!ctx->writeAccess) {
                beginTransaction(ctx->mutatorCSLocked ? dbCommitLock : dbExclusiveLock);
            }
            if (commitDelay != 0) {
                dbCriticalSection cs(delayedCommitStartTimerMutex); 
                monitor->delayedCommitContext = ctx;
                ctx->commitDelayed = true;
                delayedCommitStopTimerEvent.reset();
                delayedCommitStartTimerEvent.signal();
            } else { 
                commit(ctx);
            }
        } else {
            if (ctx->writeAccess || ctx->readAccess || ctx->mutatorCSLocked) { 
                endTransaction(ctx);
            }
        }
    }
}

void dbDatabase::commit(dbDatabaseThreadContext* ctx)
{
    if (logger != NULL) { 
        if (!logger->commitPhase1()) { 
            handleError(RejectedByTransactionLogger);
        }
    }            
    //
    // commit transaction 
    //
    int curr = header->curr;
    int4 *map = monitor->dirtyPagesMap;
    size_t oldIndexSize = header->root[curr].indexSize;
    size_t newIndexSize = header->root[1-curr].indexSize;
    if (newIndexSize > oldIndexSize) { 
        offs_t newIndex = allocate(newIndexSize*sizeof(offs_t));
        header->root[1-curr].shadowIndex = newIndex;
        header->root[1-curr].shadowIndexSize = (oid_t)newIndexSize;
        cloneBitmap(header->root[curr].index, oldIndexSize*sizeof(offs_t));
        deallocate(header->root[curr].index, oldIndexSize*sizeof(offs_t));
    }

    //
    // Enable read access to the database 
    //
    cs.enter();
    FASTDB_ASSERT(ctx->writeAccess);
    monitor->commitInProgress = true;
    monitor->sharedLockOwner[0] = monitor->exclusiveLockOwner;
    monitor->exclusiveLockOwner = 0;
    monitor->nWriters -= 1;
    monitor->nReaders += 1;
    monitor->ownerPid.clear();
    if (accessType == dbConcurrentUpdate) { 
        // now readers will see updated data
        monitor->curr ^= 1;
    }
    if (monitor->nWaitReaders != 0) { 
        monitor->nReaders += monitor->nWaitReaders;
        readSem.signal(monitor->nWaitReaders);
        monitor->nWaitReaders = 0;
    }
    ctx->writeAccess = false;
    ctx->readAccess = true;

    // Copy values of this fields to local variables since them can be changed by read-only transaction in concurrent update mode
    size_t   committedIndexSize = this->committedIndexSize;
    offs_t*  currIndex = this->currIndex; 
    size_t   currIndexSize = this->currIndexSize;

    cs.leave();

    size_t   nPages = committedIndexSize / dbHandlesPerPage;
    offs_t*  srcIndex = currIndex; 
    offs_t*  dstIndex = index[curr];         

    for (size_t i = 0; i < nPages; i++) { 
        if (map[i >> 5] & (1 << (i & 31))) { 
            file.markAsDirty(header->root[1-curr].index + i*dbPageSize, dbPageSize);
            for (size_t j = 0; j < dbHandlesPerPage; j++) {
                offs_t offs = dstIndex[j];
                if (srcIndex[j] != offs) { 
                    if (!(offs & dbFreeHandleMarker)) {
                        size_t marker = offs & dbInternalObjectMarker;
                        if (marker != 0) { 
                            deallocate(offs-(offs_t)marker, internalObjectSize[marker]);
                        } else { 
                            deallocate(offs, ((dbRecord*)(baseAddr+offs))->size);
                        }
                    }
                }
            }
        }
        dstIndex += dbHandlesPerPage;
        srcIndex += dbHandlesPerPage;
    }
    file.markAsDirty(header->root[1-curr].index + nPages*dbPageSize, 
                     (currIndexSize - nPages*dbHandlesPerPage)*sizeof(offs_t));
    offs_t* end = index[curr] + committedIndexSize;
    while (dstIndex < end) { 
        offs_t offs = *dstIndex;
        if (*srcIndex != offs) { 
            if (!(offs & dbFreeHandleMarker)) {
                size_t marker = offs & dbInternalObjectMarker;
                if (marker != 0) { 
                    deallocate(offs-(offs_t)marker, internalObjectSize[marker]);
                } else { 
                    deallocate(offs, ((dbRecord*)(baseAddr+offs))->size);
                }
            }
        }
        dstIndex += 1;
        srcIndex += 1;
    }
    file.markAsDirty(0, sizeof(dbHeader));
    file.flush();
    
    cs.enter();
    while (monitor->backupInProgress) { 
        cs.leave();
        backupCompletedEvent.wait();
        cs.enter();
    }
    header->curr = curr ^= 1;
    cs.leave();
    
    file.markAsDirty(0, sizeof(dbHeader));
#ifdef SYNCHRONOUS_REPLICATION
    waitTransactionAcknowledgement();
#else
    file.flush();    
#endif
    
    file.markAsDirty(0, sizeof(dbHeader));
    header->root[1-curr].indexUsed = (oid_t)currIndexSize; 
    header->root[1-curr].freeList  = header->root[curr].freeList; 
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
    monitor->sessionFreeList[1-curr] = monitor->sessionFreeList[curr];
#endif
    
    if (newIndexSize != oldIndexSize) {
        header->root[1-curr].index=header->root[curr].shadowIndex;
        header->root[1-curr].indexSize=header->root[curr].shadowIndexSize;
        header->root[1-curr].shadowIndex=header->root[curr].index;
        header->root[1-curr].shadowIndexSize=header->root[curr].indexSize;
        file.markAsDirty(header->root[1-curr].index, currIndexSize*sizeof(offs_t));
        memcpy(baseAddr + header->root[1-curr].index, currIndex, 
               currIndexSize*sizeof(offs_t));
        memset(map, 0, 4*((currIndexSize+dbHandlesPerPage*32-1)
                          / (dbHandlesPerPage*32)));
    } else { 
        byte* srcIndex = (byte*)currIndex; 
        byte* dstIndex = (byte*)index[1-curr]; 
        
        for (size_t i = 0; i < nPages; i++) { 
            if (map[i >> 5] & (1 << (i & 31))) { 
                map[i >> 5] -= (1 << (i & 31));
                memcpy(dstIndex, srcIndex, dbPageSize);
                file.markAsDirty(header->root[1-curr].index + i*dbPageSize, dbPageSize);
            }
            srcIndex += dbPageSize;
            dstIndex += dbPageSize;
        }
        if (currIndexSize > nPages*dbHandlesPerPage) {
            memcpy(dstIndex, srcIndex,                         
                   sizeof(offs_t)*(currIndexSize-nPages*dbHandlesPerPage));
            file.markAsDirty(header->root[1-curr].index + nPages*dbPageSize, 
                             sizeof(offs_t)*(currIndexSize-nPages*dbHandlesPerPage));
            memset(map + (nPages>>5), 0, 
                   ((currIndexSize + dbHandlesPerPage*32 - 1)
                    / (dbHandlesPerPage*32) - (nPages>>5))*4);
        }
    }
    cs.enter();
    modified = false;
    monitor->modified = false;
    monitor->uncommittedChanges = false;
    monitor->commitInProgress = false;
    if (accessType != dbConcurrentUpdate) { 
        monitor->curr = curr;
    }
    monitor->concurrentTransId += 1;
    cs.leave();

    if (logger != NULL) { 
        logger->commitPhase2();
    }

    if (ctx->writeAccess || ctx->readAccess || ctx->mutatorCSLocked) { 
        endTransaction(ctx);
    }
}

void dbDatabase::rollback() 
{
    if (logger != NULL) { 
        logger->rollback();
    }
    dbDatabaseThreadContext* ctx = threadContext.get();
    if (commitDelay != 0) {
        beginTransaction(dbExclusiveLock);
    }
    if (modified
        && (monitor->uncommittedChanges || ctx->writeAccess || ctx->mutatorCSLocked || ctx->concurrentId == monitor->concurrentTransId))
    { 
        if (!ctx->writeAccess && !ctx->mutatorCSLocked) {
            beginTransaction(dbExclusiveLock);
        }
        int curr = header->curr;
        byte* dstIndex = baseAddr + header->root[curr].shadowIndex; 
        byte* srcIndex = (byte*)index[curr]; 

        currRBitmapPage = currPBitmapPage = dbBitmapId;
        currRBitmapOffs = currPBitmapOffs = 0;

        size_t nPages = 
            (committedIndexSize + dbHandlesPerPage - 1) / dbHandlesPerPage;
        int4 *map = monitor->dirtyPagesMap;
        if (header->root[1-curr].index != header->root[curr].shadowIndex) { 
            memcpy(dstIndex, srcIndex,  nPages*dbPageSize);
            file.markAsDirty( header->root[curr].shadowIndex, nPages*dbPageSize);
        } else { 
            for (size_t i = 0; i < nPages; i++) { 
                if (map[i >> 5] & (1 << (i & 31))) { 
                    memcpy(dstIndex, srcIndex, dbPageSize);
                    file.markAsDirty(header->root[1-curr].index + i*dbPageSize, dbPageSize);
                }
                srcIndex += dbPageSize;
                dstIndex += dbPageSize;
            }
        }

        header->root[1-curr].indexSize = header->root[curr].shadowIndexSize;
        header->root[1-curr].indexUsed = header->root[curr].indexUsed;
        header->root[1-curr].freeList  = header->root[curr].freeList; 
        header->root[1-curr].index = header->root[curr].shadowIndex;
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
        monitor->sessionFreeList[1-curr] = monitor->sessionFreeList[curr];
#endif
        currIndex = index[1-curr] = (offs_t*)(baseAddr + header->root[1-curr].index);
        memset(map, 0,  
               size_t((currIndexSize+dbHandlesPerPage*32-1) / (dbHandlesPerPage*32))*4);

        file.markAsDirty(0, sizeof(dbHeader));
        modified = false;
        monitor->uncommittedChanges = false;
        monitor->concurrentTransId += 1;
        restoreTablesConsistency();
    } 
    if (monitor->users != 0) { // if not abandon        
        endTransaction(ctx);
    }
}

void dbDatabase::updateCursors(oid_t oid, bool removed) 
{ 
    dbDatabaseThreadContext* ctx = threadContext.get();
    if (ctx != NULL) { 
        for (dbAnyCursor* cursor = (dbAnyCursor*)ctx->cursors.next;
             cursor != &ctx->cursors; 
             cursor = (dbAnyCursor*)cursor->next) 
        { 
            if (cursor->currId == oid) { 
                if (removed) { 
                    cursor->currId = 0;
                } else if (cursor->record != NULL/* && !cursor->updateInProgress*/) { 
                    cursor->fetch();
                }
            }
        }
    }
}     


void dbDatabase::endTransaction(dbDatabaseThreadContext* ctx) 
{
    if (!ctx->commitDelayed) {
        while (!ctx->cursors.isEmpty()) { 
            ((dbAnyCursor*)ctx->cursors.next)->reset();
        }
    }
    if (ctx->writeAccess) { 
        cs.enter();
        ctx->isMutator = false;
        monitor->nWriters -= 1;
        monitor->exclusiveLockOwner = 0;
        monitor->ownerPid.clear();
        FASTDB_ASSERT(monitor->nWriters == 0 && !monitor->waitForUpgrade);
        if (monitor->nWaitWriters != 0) { 
            monitor->nWaitWriters -= 1;
            monitor->nWriters = 1;
            writeSem.signal();
        } else if (monitor->nWaitReaders != 0) { 
            monitor->nReaders = monitor->nWaitReaders;
            monitor->nWaitReaders = 0;
            readSem.signal(monitor->nReaders);
        }
        cs.leave();
    } else if (ctx->readAccess) { 
        cs.enter();
#if defined(AUTO_DETECT_PROCESS_CRASH) || DEBUG_LOCKS
#ifdef DEBUG_LOCKS
        long selfId = dbThread::getCurrentThreadId();
#endif
        removeLockOwner(selfId);
#endif
        monitor->nReaders -= 1;
        if (monitor->nReaders == 1 && monitor->waitForUpgrade) { 
            FASTDB_ASSERT(monitor->nWriters == 0);
#if defined(AUTO_DETECT_PROCESS_CRASH) || DEBUG_LOCKS
            removeLockOwner(monitor->upgradeId);
            monitor->upgradeId = 0;
#endif
            monitor->waitForUpgrade = false;
            monitor->nWaitWriters -= 1;
            monitor->nWriters = 1;
            monitor->nReaders = 0;
            upgradeSem.signal();
        } else if (monitor->nReaders == 0) { 
            if (monitor->nWaitWriters != 0) { 
                FASTDB_ASSERT(monitor->nWriters == 0 && !monitor->waitForUpgrade);
                monitor->nWaitWriters -= 1;
                monitor->nWriters = 1;
                writeSem.signal();
            }
        } 
        cs.leave();
    }
    ctx->writeAccess = false;
    ctx->readAccess = false;
    if (ctx->mutatorCSLocked) { 
        ctx->mutatorCSLocked = false;
        mutatorCS.leave();
    }            
}


void dbDatabase::linkTable(dbTableDescriptor* table, oid_t tableId)
{
    FASTDB_ASSERT(((void)"Table can be used only in one database", 
            table->tableId == 0));
    table->db = this;
    table->nextDbTable = tables;
    table->tableId = tableId;
    tables = table;

    size_t h = (size_t)table->name % dbTableHashSize;
    table->collisionChain = tableHash[h];
    tableHash[h] = table;
}

void dbDatabase::unlinkTable(dbTableDescriptor* table)
{
    dbTableDescriptor** tpp;
    for (tpp = &tables; *tpp != table; tpp = &(*tpp)->nextDbTable);
    *tpp = table->nextDbTable;
    table->tableId = 0;

    size_t h = (size_t)table->name % dbTableHashSize;
    for (tpp = &tableHash[h]; *tpp != table; tpp = &(*tpp)->collisionChain);
    *tpp = table->collisionChain;

    if (!table->fixedDatabase) { 
        table->db = NULL;
    }
}

dbTableDescriptor* dbDatabase::findTableByName(char const* name)
{
    char* sym = (char*)name;
    dbSymbolTable::add(sym, tkn_ident);
    return findTable(sym);
}

    
dbTableDescriptor* dbDatabase::findTable(char const* name)
{
    size_t h = (size_t)name % dbTableHashSize;
    for (dbTableDescriptor* desc = tableHash[h]; desc != NULL; desc = desc->collisionChain) { 
        if (desc->name == name) { 
            return desc;
        }
    }
    return NULL;
}

dbTableDescriptor* dbDatabase::findTableByID(oid_t id)
{
    for (dbTableDescriptor* desc = tables; desc != NULL; desc = desc->nextDbTable) { 
        if (desc->tableId == id) { 
            return desc;
        }
    }
    return NULL;
}
    
void dbDatabase::insertInverseReference(dbFieldDescriptor* fd, oid_t inverseId,
                                        oid_t targetId)
{
    byte buf[1024];
    if (inverseId == targetId) { 
        return;
    }
    fd = fd->inverseRef;
    if (fd->type == dbField::tpArray) { 
        dbTableDescriptor* desc = fd->defTable;
        dbRecord* rec = getRow(targetId);
        dbVarying* arr = (dbVarying*)((byte*)rec + fd->dbsOffs);
        size_t arrSize = arr->size;
        size_t arrOffs = arr->offs;
        offs_t oldOffs = currIndex[targetId];
        size_t newSize = desc->fixedSize;
        size_t lastOffs = desc->columns->sizeWithoutOneField(fd, (byte*)rec, newSize);
        size_t newArrOffs = DOALIGN(newSize, sizeof(oid_t));
        size_t oldSize = rec->size;
        newSize = newArrOffs + (arrSize + 1)*sizeof(oid_t);
        if (newSize > oldSize) { 
            newSize = newArrOffs + (arrSize+1)*sizeof(oid_t)*2;
        } else { 
            newSize = oldSize;
        }

        byte* dst = (byte*)putRow(targetId, newSize);
        byte* src = baseAddr + oldOffs;
        byte* tmp = NULL;

        if (dst == src) { 
            if (arrOffs == newArrOffs && newArrOffs > lastOffs) { 
                *((oid_t*)((byte*)rec + newArrOffs) + arrSize) = inverseId;
                arr->size += 1;
                updateCursors(targetId); 
                return;
            }
            if (oldSize > sizeof(buf)) { 
                src = tmp = dbMalloc(oldSize);
            } else { 
                src = buf;
            }
            memcpy(src, rec, oldSize);
        }
        desc->columns->copyRecordExceptOneField(fd, dst, src, desc->fixedSize);
        
        arr = (dbVarying*)(dst + fd->dbsOffs);
        arr->size = (nat4)arrSize + 1;
        arr->offs = (int)newArrOffs;
        memcpy(dst + newArrOffs, src + arrOffs, arrSize*sizeof(oid_t));
        *((oid_t*)(dst + newArrOffs) + arrSize) = inverseId;
        if (tmp != NULL) { 
            dbFree(tmp);
        }
    } else { 
        if (fd->indexType & INDEXED) { 
            dbTtree::remove(this, fd->tTree, targetId, fd->type, (int)fd->dbsSize, fd->_comparator, fd->dbsOffs);
        }         
        oid_t oldRef = *(oid_t*)((byte*)getRow(targetId) + fd->dbsOffs);
        if (oldRef != 0) {
            removeInverseReference(fd, targetId, oldRef);
        }
         *(oid_t*)((byte*)putRow(targetId) + fd->dbsOffs) = inverseId;

        if (fd->indexType & INDEXED) { 
            dbTtree::insert(this, fd->tTree, targetId, fd->type, (int)fd->dbsSize, fd->_comparator, fd->dbsOffs);
        }
    }
    updateCursors(targetId); 
}


void dbDatabase::removeInverseReferences(dbTableDescriptor* desc, oid_t oid)
{
    dbVisitedObject* chain = visitedChain;
    dbVisitedObject  vo(oid, chain);
    visitedChain = &vo;
        
    dbFieldDescriptor* fd;
    for (fd = desc->inverseFields; fd != NULL; fd = fd->nextInverseField) { 
        if (fd->type == dbField::tpArray) { 
            dbVarying* arr = (dbVarying*)((byte*)getRow(oid) + fd->dbsOffs);
            int n = arr->size;
            int offs = arr->offs + n*sizeof(oid_t);
            while (--n >= 0) { 
                offs -= sizeof(oid_t);
                oid_t ref = *(oid_t*)((byte*)getRow(oid) + offs);
                if (ref != 0) {
                    removeInverseReference(fd, oid, ref);
                }
            }
        } else { 
            oid_t ref = *(oid_t*)((byte*)getRow(oid) + fd->dbsOffs);
            if (ref != 0) { 
                removeInverseReference(fd, oid, ref);
            }
        }
    }

    visitedChain = chain;    
}


void dbDatabase::removeInverseReference(dbFieldDescriptor* fd, 
                                        oid_t inverseId,
                                        oid_t targetId)
{
    if (inverseId == targetId || targetId == updatedRecordId ||
        (currIndex[targetId] & dbFreeHandleMarker) != 0) 
    { 
        return;
    }
    for (dbVisitedObject* vo = visitedChain; vo != NULL; vo = vo->next) { 
        if (vo->oid == targetId) { 
            return;
        }
    }
    byte* rec = (byte*)putRow(targetId);
    if (cascadeDelete && (fd->indexType & DB_FIELD_CASCADE_DELETE)
        && ((fd->inverseRef->type != dbField::tpArray) ||
            ((dbVarying*)(rec + fd->inverseRef->dbsOffs))->size <= 1)) 
    { 
        remove(fd->inverseRef->defTable, targetId);
        return;
    } 

    fd = fd->inverseRef;
    if (fd->type == dbField::tpArray) { 
        dbVarying* arr = (dbVarying*)(rec + fd->dbsOffs);
        oid_t* p = (oid_t*)(rec + arr->offs);
        for (int n = arr->size, i = n; --i >= 0;) { 
            if (p[i] == inverseId) { 
                while (++i < n) { 
                    p[i-1] = p[i];
                }
                arr->size -= 1;
                break;
            }
        }
    } else { 
        if (*(oid_t*)(rec + fd->dbsOffs) == inverseId) { 
            if (fd->indexType & INDEXED) { 
                dbTtree::remove(this, fd->tTree, targetId, fd->type, (int)fd->dbsSize, fd->_comparator, fd->dbsOffs);
            }            

            *(oid_t*)((byte*)putRow(targetId) + fd->dbsOffs) = 0;

            if (fd->indexType & INDEXED) { 
                dbTtree::insert(this, fd->tTree, targetId, fd->type, (int)fd->dbsSize, fd->_comparator, fd->dbsOffs);
            }
        }
    }
    updateCursors(targetId); 
}

bool dbDatabase::completeDescriptorsInitialization()
{
    bool result = true;
    for (dbTableDescriptor* desc = tables; desc != NULL; desc = desc->nextDbTable) { 
        dbFieldDescriptor* fd;
        for (fd = desc->firstField; fd != NULL; fd = fd->nextField) { 
            if (fd->refTableName != NULL) { 
                fd->refTable = findTable(fd->refTableName);
            }
        }
        result &= desc->checkRelationship();
    }
    return result;
}


bool dbDatabase::backup(char_t const* file, bool compactify)
{
    dbFile f;
    if (f.create(file, compactify ? 0 : dbFile::no_buffering) != dbFile::ok) {
        return false;
    }  
    bool result = backup(&f, compactify);
    f.close();
    return result;
}


bool dbDatabase::backup(dbFile* f, bool compactify)
{
    bool result = true;
    cs.enter();
    if (monitor->backupInProgress) { 
        cs.leave();
        return false; // no two concurrent backups are possible
    }
    backupCompletedEvent.reset(); 
    monitor->backupInProgress = true;
    cs.leave();
    if (compactify) { 
        int     curr = header->curr;
        size_t  nObjects = header->root[1-curr].indexUsed;
        size_t  i;
        size_t  nIndexPages = (header->root[1-curr].indexSize + dbHandlesPerPage - 1) / dbHandlesPerPage;
        offs_t* newIndex = new offs_t[nIndexPages*dbHandlesPerPage];

        memset(newIndex, 0, nIndexPages*dbPageSize);
        offs_t used = (offs_t)((nIndexPages*2 + 1)*dbPageSize);
        offs_t start = used;
        used += DOALIGN(getRow(dbMetaTableId)->size, dbAllocationQuantum);

        for (i = dbFirstUserId; i < nObjects; i++) {
            offs_t offs = currIndex[i];
            if (!(offs & dbFreeHandleMarker)) { 
                offs_t marker = offs & dbInternalObjectMarker;
                newIndex[i] = used | marker;
                used += (offs_t)(DOALIGN(marker ? internalObjectSize[marker] : (size_t)getRow((oid_t)i)->size, 
                                 dbAllocationQuantum));
            } else { 
                newIndex[i] = offs;
            }
        } 
        size_t bitmapPages = 
            (used + dbPageSize*(dbAllocationQuantum*8-1) - 1)
            / (dbPageSize*(dbAllocationQuantum*8-1));
        size_t bitmapSize = bitmapPages*dbPageSize;
        for (i = dbFirstUserId; i < nObjects; i++) {
            if (!(newIndex[i] & dbFreeHandleMarker)) { 
                newIndex[i] += (offs_t)bitmapSize;
            }
        }
        used += (offs_t)bitmapSize;

        for (i = 0; i < bitmapPages; i++) {
            newIndex[dbBitmapId+i] = start | dbPageObjectMarker;
            start += dbPageSize;
        }
        while (i < dbBitmapPages) { 
            newIndex[dbBitmapId+i] = dbFreeHandleMarker;
            i += 1;
        }
        newIndex[dbMetaTableId] = start;
        newIndex[dbInvalidId] =  dbFreeHandleMarker;

        byte page[dbPageSize];
        memset(page, 0, sizeof page);
        dbHeader* newHeader = (dbHeader*)page;
        offs_t newFileSize = DOALIGN(used, dbPageSize);
        newHeader->size = newFileSize;
        newHeader->curr = 0;
        newHeader->dirty = 0;
        newHeader->initialized = true;
        newHeader->majorVersion = header->majorVersion;
        newHeader->minorVersion = header->minorVersion;
        newHeader->mode = header->mode;
        newHeader->used = used;
        newHeader->root[0].index = newHeader->root[1].shadowIndex = (offs_t)dbPageSize;
        newHeader->root[0].shadowIndex = newHeader->root[1].index = (offs_t)(dbPageSize + nIndexPages*dbPageSize);
        newHeader->root[0].shadowIndexSize = newHeader->root[0].indexSize = 
            newHeader->root[1].shadowIndexSize = newHeader->root[1].indexSize = (oid_t)(nIndexPages*dbHandlesPerPage);
        newHeader->root[0].indexUsed = newHeader->root[1].indexUsed = (oid_t)nObjects;
        newHeader->root[0].freeList = newHeader->root[1].freeList = header->root[1-curr].freeList;
        result &= f->write(page, dbPageSize);
        
        result &= f->write(newIndex, nIndexPages*dbPageSize);
        result &= f->write(newIndex, nIndexPages*dbPageSize);

        int bits = (int)((used >> dbAllocationQuantumBits) - (offs_t)(bitmapPages-1)*dbPageSize*8);
        memset(page, 0xFF, sizeof page);
        while (--bitmapPages != 0) {
            result &= f->write(page, dbPageSize);
        }    
        if (size_t(bits >> 3) < dbPageSize) { 
            memset(page + (bits >> 3) + 1, 0, dbPageSize - (bits >> 3) - 1);
            page[bits >> 3] = (1 << (bits & 7)) - 1;
        }
        result &= f->write(page, dbPageSize);

        result &= f->write(baseAddr + currIndex[dbMetaTableId], DOALIGN(getRow(dbMetaTableId)->size, dbAllocationQuantum));

        for (i = dbFirstUserId; i < nObjects; i++) {
            offs_t offs = newIndex[i];
            if (!(offs & dbFreeHandleMarker)) { 
                offs_t marker = (int)(offs & dbInternalObjectMarker);
                size_t size = DOALIGN(marker ? internalObjectSize[marker] : getRow((oid_t)i)->size, 
                                      dbAllocationQuantum);
                result &= f->write(baseAddr + currIndex[i] - marker, size);
            }
        }
        if (used != newFileSize) {          
            FASTDB_ASSERT(newFileSize - used < dbPageSize);
            size_t align = (size_t)(newFileSize - used);
            memset(page, 0, align);
            result &= f->write(page, align);
        }
        delete[] newIndex;
    } else { // end if compactify 
        const size_t segmentSize = 64*1024;
        byte* p = baseAddr;
        size_t size = (size_t)header->size;
        result = true;
        while (size > segmentSize && result) { 
            result = f->write(p, segmentSize);
            p += segmentSize;
            size -= segmentSize;
        }
        if (result) { 
            result = f->write(p, size);
        }
    }
    cs.enter();
    monitor->backupInProgress = false;
    backupCompletedEvent.signal(); 
    cs.leave();
    return result;
}

dbDatabase::dbDatabase(dbAccessType type, size_t dbInitSize, 
                       size_t dbExtensionQuantum, size_t dbInitIndexSize,
                       int nThreads,
                       int appMode)
: accessType(type), 
    initSize(dbInitSize), 
    extensionQuantum(dbExtensionQuantum),
    initIndexSize(dbInitIndexSize),
    freeSpaceReuseThreshold((offs_t)dbExtensionQuantum),
    parallelScanThreshold(dbDefaultParallelScanThreshold)
{
    int libMode = 0
#ifdef NO_PTHREADS
               | dbHeader::MODE_NO_PTHREADS
#endif
#ifdef REPLICATION_SUPPORT
               | dbHeader::MODE_REPLICATION
#endif
#ifdef ALIGN_HEADER      
               | dbHeader::MODE_ALIGN_HEADER
#endif        
#ifdef PAD_HEADER      
               | dbHeader::MODE_PAD_HEADER
#endif        
#if dbDatabaseOffsetBits > 32
               | dbHeader::MODE_OFFS_64
#endif        
#if dbDatabaseOidBits > 32
               | dbHeader::MODE_OID_64
#endif        
#ifdef AUTOINCREMENT_SUPPORT    
               | dbHeader::MODE_AUTOINCREMENT
#endif
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
               | dbHeader::MODE_DO_NOT_REUSE_OID
#endif
                     ;
    if (appMode != libMode) {
        fprintf(stderr, "Incompatibility between headers and library: %x vs. %x\n", appMode, libMode);
        exit(1);
    }
#ifdef AUTO_DETECT_PROCESS_CRASH
    FASTDB_ASSERT(type != dbConcurrentUpdate);
#endif
#if dbDatabaseOffsetBits > 32 ||  dbDatabaseOidBits > 32
    FASTDB_ASSERT(sizeof(size_t) == 8);
#endif
    bitmapPageAvailableSpace = new int[dbBitmapId + dbBitmapPages];
    setConcurrency(nThreads);
    tables = NULL;
    commitDelay = 0;
    commitTimeout = 0;
    commitTimerStarted = 0;
    backupFileName = NULL;
    backupPeriod = 0;
    databaseName = NULL;
    fileName = NULL;
    opened = false;
    fileSizeLimit = 0;
    errorHandler = NULL;
    logger = NULL;
    confirmDeleteColumns = false;
    inverseReferencesUpdate = true;
    cascadeDelete = true;
    schemeVersion = 0;
    visitedChain = NULL;
    header = NULL;
    xmlContext = NULL;
    fileOpenFlags = 0;
}      

dbDatabase::~dbDatabase() 
{
    delete[] bitmapPageAvailableSpace;
    delete[] databaseName;
    delete[] fileName;
}


void dbDatabase::setTransactionLogger(dbTransactionLogger* logger)
{
    this->logger = logger;
}

dbDatabase::dbErrorHandler dbDatabase::setErrorHandler(dbDatabase::dbErrorHandler newHandler, void* context)
{
    dbErrorHandler prevHandler = errorHandler;
    errorHandler = newHandler;
    errorHandlerContext = context;
    return prevHandler;
}

dbTableDescriptor* dbDatabase::loadMetaTable()
{
    dbTable* table = (dbTable*)getRow(dbMetaTableId);
    dbTableDescriptor* metatable = new dbTableDescriptor(this, table);
    linkTable(metatable, dbMetaTableId);
    oid_t tableId = table->firstRow;
    while (tableId != 0) {                      
        table = (dbTable*)getRow(tableId);
        dbTableDescriptor* desc;
        for (desc = tables; desc != NULL && desc->tableId != tableId; desc = desc->nextDbTable);
        if (desc == NULL) { 
            desc = new dbTableDescriptor(this, table);
            linkTable(desc, tableId);
            desc->setFlags();
        }
        tableId = table->next;
    }
    completeDescriptorsInitialization();
    return metatable;
}

#ifdef REPLICATION_SUPPORT

dbConnection::~dbConnection() 
{ 
    readyEvent.close();
    useEvent.close();
    statusEvent.close();
    committedEvent.close();
    delete reqSock;
    delete respSock;
}


#define MAX_LOST_TRANSACTIONS 100

char const* statusText[] = {
    "OFFLINE", 
    "ONLINE", 
    "ACTIVE",
    "STANDBY",
    "RECOVERED"
};

char const* requestText[] = {
    "CONNECT", 
    "RECOVERY",
    "GET_STATUS",
    "STATUS",
    "UPDATE_PAGE",
    "RECOVER_PAGE",
    "NEW_ACTIVE_NODE",
    "CHANGE_ACTIVE_NODE",
    "CLOSE",
    "READY",
    "COMMITTED"
};

bool dbReplicatedDatabase::isReplicated()
{
    return true;
}

dbReplicatedDatabase::dbReplicatedDatabase(dbAccessType type,
                                           size_t dbInitSize,
                                           size_t dbExtensionQuantum,
                                           size_t dbInitIndexSize,
                                           int nThreads)
: dbDatabase(type, dbInitSize, dbExtensionQuantum, dbInitIndexSize, nThreads)
{
    pollInterval = dbDefaultPollInterval;
    waitReadyTimeout = dbWaitReadyTimeout;
    waitStatusTimeout = dbWaitStatusTimeout;
    recoveryConnectionAttempts = dbRecoveryConnectionAttempts; 
    startupConnectionAttempts = dbStartupConnectionAttempts;
    replicationWriteTimeout = dbReplicationWriteTimeout;
    maxAsyncRecoveryIterations = dbMaxAsyncRecoveryIterations;   
}

bool dbReplicatedDatabase::open(OpenParameters& params)
{
    accessType = params.accessType;
    fileOpenFlags = params.fileOpenFlags;
    extensionQuantum = params.extensionQuantum;
    initIndexSize = params.initIndexSize; 
    initSize = params.initSize;
    freeSpaceReuseThreshold = params.freeSpaceReuseThreshold;
    setConcurrency(params.nThreads);
    pollInterval = params.pollInterval;
    waitLockTimeout = params.waitLockTimeoutMsec;
    waitReadyTimeout = params.waitReadyTimeout;
    waitStatusTimeout = params.waitStatusTimeout;
    recoveryConnectionAttempts = params.recoveryConnectionAttempts;
    startupConnectionAttempts = params.startupConnectionAttempts;
    replicationWriteTimeout = params.replicationWriteTimeout;
    maxAsyncRecoveryIterations = params.maxAsyncRecoveryIterations;
    return open(params.databaseName, params.databaseFilePath, params.nodeId, params.nodeAddresses, params.nNodes);
}

bool dbReplicatedDatabase::open(char_t const* dbName, char_t const* fiName,
                                int id, char* servers[], int nServers)
{
    int i;
    char buf [64];
    ReplicationRequest rr;

    FASTDB_ASSERT(accessType != dbReadOnly);

    this->id = id;
    this->nServers = nServers;
    con = new dbConnection[nServers];    
    serverURL = servers;
    delete[] databaseName;
    delete[] fileName;
    logger = NULL;
    commitDelay = 0;
    commitTimeout = 0;
    commitTimerStarted = 0;
    waitLockTimeout = INFINITE;
    delayedCommitEventsOpened = false;
    backupFileName = NULL;
    backupPeriod = 0;
    opened = false;
    header = NULL;
    stopDelayedCommitThread = false;
    onlineRecovery = false;
    memset(tableHash, 0, sizeof tableHash);
    databaseNameLen = (int)_tcslen(dbName);
    char_t* name = new char_t[databaseNameLen+16];
    _stprintf(name, _T("%s.in"), dbName);
    databaseName = name;
    if (fiName == NULL) { 
        fileName = new char_t[databaseNameLen + 5];
        _stprintf(fileName, _T("%s.fdb"), dbName);
    } else { 
        fileName = new char_t[_tcslen(fiName)+1];
        _stprintf(fileName, fiName);
    }

    dbInitializationMutex::initializationStatus status = initMutex.initialize(name);
    if (status == dbInitializationMutex::InitializationError) { 
        handleError(DatabaseOpenError, "Failed to start database initialization");
        return false;
    }
    if (status != dbInitializationMutex::NotYetInitialized) { 
        handleError(DatabaseOpenError, "Database is already started");
        return false;
    }
    _stprintf(name, _T("%s.dm"), dbName);
    if (!shm.open(name)) { 
        handleError(DatabaseOpenError, "Failed to open database monitor");
        cleanup(status, 0);
        return false;
    }
    monitor = shm.get();
    _stprintf(name, _T("%s.ws"), dbName);
    if (!writeSem.open(name)) { 
        handleError(DatabaseOpenError, 
                    "Failed to initialize database writers semaphore");
        cleanup(status, 1);
        return false;
    }
    _stprintf(name, _T("%s.rs"), dbName);
    if (!readSem.open(name)) { 
        handleError(DatabaseOpenError, 
                    "Failed to initialize database readers semaphore");
        cleanup(status, 2);
        return false;
    }
    _stprintf(name, _T("%s.us"), dbName);
    if (!upgradeSem.open(name)) { 
        handleError(DatabaseOpenError, 
                    "Failed to initialize database upgrade semaphore");
        cleanup(status, 3);
        return false;
    }
    _stprintf(name, _T("%s.bce"), dbName);
    if (!backupCompletedEvent.open(name)) { 
        handleError(DatabaseOpenError, 
                    "Failed to initialize database backup completed event");
        cleanup(status, 4);
        return false;
    }    
    backupInitEvent.open();
    backupFileName = NULL;

    fixedSizeAllocator.reset();
    allocatedSize = 0;
    size_t indexSize = initIndexSize < dbFirstUserId 
        ? size_t(dbFirstUserId) : initIndexSize;
    indexSize = DOALIGN(indexSize, dbHandlesPerPage);
            
    size_t fileSize = initSize ? initSize : dbDefaultInitDatabaseSize;
    fileSize = DOALIGN(fileSize, dbBitmapSegmentSize);

    if (fileSize < indexSize*sizeof(offs_t)*4) {
        fileSize = indexSize*sizeof(offs_t)*4;
    }

    for (i = dbBitmapId + dbBitmapPages; --i >= 0;) { 
        bitmapPageAvailableSpace[i] = INT_MAX;
    }
    currRBitmapPage = currPBitmapPage = dbBitmapId;
    currRBitmapOffs = currPBitmapOffs = 0;
    bitmapEnd = dbBitmapId;
    reservedChain = NULL;
    reservedChainLength = 0;
    tables = NULL;
    modified = false;
    selfId = 0;
    maxClientId = 0;
    attach();
    
    _stprintf(name, _T("%s.cs"), dbName);
    if (!cs.create(name, &monitor->sem)) { 
        handleError(DatabaseOpenError, "Failed to initialize database monitor");
        cleanup(status, 6);
        return false;
    }
    if (accessType == dbConcurrentUpdate || accessType == dbConcurrentRead) { 
        _stprintf(name, _T("%s.mcs"), dbName);
        if (!mutatorCS.create(name, &monitor->mutatorSem)) { 
            cleanup(status, 7);
            handleError(DatabaseOpenError,
                        "Failed to initialize database monitor");
            return false;
        }
    }
    readSem.reset();
    writeSem.reset();
    upgradeSem.reset();
    monitor->nReaders = 0;
    monitor->nWriters = 0;
    monitor->nWaitReaders = 0;
    monitor->nWaitWriters = 0;
    monitor->waitForUpgrade = false;
    monitor->version = version = 1;
    monitor->users = 0;
    monitor->backupInProgress = 0;
    monitor->forceCommitCount = 0;
    monitor->lastDeadlockRecoveryTime = 0;
    monitor->delayedCommitContext = NULL;
    monitor->concurrentTransId = 1;
    monitor->commitInProgress = false;
    monitor->uncommittedChanges = false;
    monitor->clientId = 0;
    monitor->upgradeId = 0;
    monitor->modified = false;
    monitor->exclusiveLockOwner = 0;
    memset(monitor->dirtyPagesMap, 0, dbDirtyPageBitmapSize);
    memset(monitor->sharedLockOwner, 0, sizeof(monitor->sharedLockOwner));
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
    monitor->sessionFreeList[0].head = monitor->sessionFreeList[0].tail = 0;
    monitor->sessionFreeList[1].head = monitor->sessionFreeList[1].tail = 0;
#endif                

    _stprintf(databaseName, _T("%s.%d"), dbName, version);
    if (file.open(fileName, databaseName, fileOpenFlags, fileSize, true) != dbFile::ok)
    {
        handleError(DatabaseOpenError, "Failed to create database file");
        cleanup(status, 8);
        return false;
    }
    baseAddr = (byte*)file.getAddr();
    monitor->size = fileSize = file.getSize();
    header = (dbHeader*)baseAddr;

    if ((unsigned)header->curr > 1) { 
        handleError(DatabaseOpenError, "Database file was corrupted: "
                    "invalid root index");
        cleanup(status, 9);
        return false;
    } 
    acceptSock = socket_t::create_global(servers[id]);
    if (!acceptSock->is_ok()) { 
        acceptSock->get_error_text(buf, sizeof buf);
        dbTrace("<<<FATAL>>> Failed to create accept socket: %s\n", buf);
        cleanup(status, 9);
        delete acceptSock;
        return false;
    }
    FD_ZERO(&inputSD);
    socket_handle_t acceptSockHnd = acceptSock->get_handle();
    FD_SET(acceptSockHnd, &inputSD);
    nInputSD = acceptSockHnd+1;
    startEvent.open(false);
    recoveredEvent.open(false);
    int connectionAttempts = startupConnectionAttempts;
    bool recoveryNeeded = false;

    if (!header->initialized) {
        monitor->curr = header->curr = 0;
        header->size = fileSize;
        size_t used = dbPageSize;
        header->root[0].index = used;
        header->root[0].indexSize = indexSize;
        header->root[0].indexUsed = dbFirstUserId;
        header->root[0].freeList = 0;
        used += indexSize*sizeof(offs_t);
        header->root[1].index = used;
        header->root[1].indexSize = indexSize;
        header->root[1].indexUsed = dbFirstUserId;
        header->root[1].freeList = 0;
        used += indexSize*sizeof(offs_t);
 
        header->root[0].shadowIndex = header->root[1].index;
        header->root[1].shadowIndex = header->root[0].index;
        header->root[0].shadowIndexSize = indexSize;
        header->root[1].shadowIndexSize = indexSize;
            
        header->majorVersion= FASTDB_MAJOR_VERSION;
        header->minorVersion = FASTDB_MINOR_VERSION;
        header->mode = header->getCurrentMode();
      
        index[0] = (offs_t*)(baseAddr + header->root[0].index);
        index[1] = (offs_t*)(baseAddr + header->root[1].index);
        index[0][dbInvalidId] = dbFreeHandleMarker;

        size_t bitmapPages = 
            (used + dbPageSize*(dbAllocationQuantum*8-1) - 1)
            / (dbPageSize*(dbAllocationQuantum*8-1));
        memset(baseAddr+used, 0xFF, (used + bitmapPages*dbPageSize)
               / (dbAllocationQuantum*8));
        size_t i;
        for (i = 0; i < bitmapPages; i++) { 
            index[0][dbBitmapId + i] = used + dbPageObjectMarker;
            used += dbPageSize;
        }
        while (i < dbBitmapPages) { 
            index[0][dbBitmapId+i] = dbFreeHandleMarker;
            i += 1;
        }
        currIndex = index[0];
        currIndexSize = dbFirstUserId;
        committedIndexSize = 0;
        initializeMetaTable();
        header->dirty = true;
        if (accessType == dbConcurrentRead) { 
            modified = false;
        }
        memcpy(index[1], index[0], currIndexSize*sizeof(offs_t));
        file.markAsDirty(0, used);
        file.flush(true);
        header->initialized = true;
        file.markAsDirty(0, sizeof(dbHeader));
        con[id].status = (accessType == dbConcurrentRead) ? ST_RECOVERED : ST_ONLINE;
    } else {
        if (!header->isCompatible()) { 
            handleError(DatabaseOpenError, "Incompatible database mode");
            cleanup(status, 9);
            delete acceptSock;
            return false;
        }        
        monitor->curr = header->curr;
        if (header->dirty) { 
            recoveryNeeded = true;
            dbTrace("Replicated node %d was not normally closed\n", id);
            con[id].status = ST_RECOVERED;
            if (accessType != dbConcurrentRead) {
                connectionAttempts = recoveryConnectionAttempts;
            }
        } else { 
            con[id].status = ST_ONLINE;
        }
    }
    cs.enter();            
    monitor->users += 1;
    selfId = ++monitor->clientId;
#ifdef AUTO_DETECT_PROCESS_CRASH
    _stprintf(databaseName + databaseNameLen, _T(".pid.%ld"), selfId);
    selfWatchDog.create(databaseName);
    watchDogMutex = new dbMutex();
#endif
    opened = true;
    cs.leave();

    if (accessType == dbConcurrentUpdate) {
        initMutex.done();
    }
    handshake = true;
    readerThread.create(startReader, this);    
  pollNodes:
    bool startup = true;
    bool standalone;
    int nOnlineNodes = 0;
    int minOnlineNodes = nServers;
    if (accessType == dbConcurrentRead && nServers > 1) { 
        minOnlineNodes = 2;
    }
    masterNodeId = activeNodeId = -1;

    do { 
        standalone = true;
        if (nOnlineNodes == minOnlineNodes) { 
            dbThread::sleep(1);
        }
        nOnlineNodes = 1;
        for (i = 0; i < nServers && (nOnlineNodes < minOnlineNodes || activeNodeId < 0); i++) { 
            if (i != id) { 
                socket_t* s = con[i].reqSock;
                if (s == NULL) { 
                    TRACE_IMSG(("Try to connect to node %d address '%s'\n", i, servers[i]));
                    s = socket_t::connect(servers[i], 
                                          socket_t::sock_global_domain, 
                                          connectionAttempts);      
                    if (!s->is_ok()) { 
                        s->get_error_text(buf, sizeof buf);
                        dbTrace("Failed to establish connection with node %d: %s\n", i, buf);
                        delete s;
                        continue;
                    } 
                    TRACE_IMSG(("Establish connection with node %d address '%s'\n", i, servers[i]));
                }
                standalone = false;
                rr.op = ReplicationRequest::RR_GET_STATUS;
                rr.nodeId = id;
                bool success = false;
                if (con[i].reqSock == NULL) { 
                    TRACE_IMSG(("Send GET_STATUS request to node %d and wait for response\n", i));
                    if (!s->write(&rr, sizeof rr) || !s->read(&rr, sizeof rr)) { 
                        s->get_error_text(buf, sizeof buf);
                        dbTrace("Failed to get status from node %d: %s\n", i, buf);
                        delete s;
                    } else { 
                        TRACE_IMSG(("Node %d returns status %s\n", i, statusText[rr.status]));
                        addConnection(i, s);
                        con[i].status = rr.status;
                        con[i].updateCounter = rr.size;
                        success = true;
                    }
                } else { 
                    con[i].statusEvent.reset();
                    con[i].waitStatusEventFlag += 1; 
                    TRACE_IMSG(("Send GET_STATUS request to node %d\n", i));
                    if (writeReq(i, rr)) {              
                        dbCriticalSection cs(startCS);
                        lockConnection(i);
                        if (!con[i].statusEvent.wait(startCS, waitStatusTimeout)) { 
                            dbTrace("Failed to get status from node %d\n", i);
                            deleteConnection(i);
                        } else { 
                            TRACE_IMSG(("Received response from node %d with status %s\n", i, statusText[con[i].status]));
                            success = true;
                        }
                        unlockConnection(i);
                    }
                    con[i].waitStatusEventFlag -= 1; 
                }
                if (success) { 
                    nOnlineNodes += 1;
                    TRACE_IMSG(("Status of node %d is %s\n", i, statusText[con[i].status]));
                    if (con[i].status == ST_ACTIVE) { 
                        startup = false;
                        masterNodeId = activeNodeId = i;
                    } else if (con[i].status == ST_STANDBY) { 
                        startup = false;
                    } else if (con[i].status == ST_ONLINE 
                        && con[i].updateCounter > file.updateCounter) 
                    { 
                        TRACE_IMSG(("Change status of current node to RECOVERED because its updateCounter=%d and update counter of active node is %d\n", file.updateCounter, con[i].updateCounter));
                        con[id].status = ST_RECOVERED;
                    }
                }
            }
        }
    } while (!standalone && con[id].status == ST_RECOVERED && activeNodeId < 0 && (id != 0 || nOnlineNodes < minOnlineNodes));
    
    if (!startup) { 
        //
        // The node was activated after the active node start the user application
        // So the node's data is out of date. Mark it as recovered.
        // 
        TRACE_IMSG(("Change status of node connected after application startup to RECOVERED\n"));
        con[id].status = ST_RECOVERED;
    }
    file.configure(this);
    TRACE_IMSG(("My status is %s\n", statusText[con[id].status]));

    if (con[id].status == ST_RECOVERED && activeNodeId < 0) { 
        if (recoveryNeeded) {
            dbTrace("Database was not normally closed: start recovery\n");
            recovery();
            recoveryNeeded = false;
        }
        con[id].status = ST_ONLINE;
    }

    if (con[id].status == ST_ONLINE) { 
        for (activeNodeId = 0; 
             activeNodeId < id && con[activeNodeId].status != ST_ONLINE; 
             activeNodeId++);
        if (activeNodeId == id && accessType != dbConcurrentRead) { 
            dbTrace("Node %d becomes active at startup\n", id);
            //
            // Nobody else pretends to be active, so I will be...
            //
            for (i = 0; i < nServers; i++) {        
                lockConnection(i);
                if (i != id && con[i].status == ST_ONLINE) { 
                    dbCriticalSection cs(startCS);
                    TRACE_IMSG(("Waiting ready event from node %d\n", i));
                    if (!con[i].readyEvent.wait(startCS, waitReadyTimeout)) { 
                        dbTrace("Didn't receive ready event from node %d\n", i);
                        deleteConnection(i);
                        unlockConnection(i);
                        goto pollNodes;
                    }
                    TRACE_IMSG(("Received ready event from node %d\n", i));
                }
                unlockConnection(i);
            }
            con[id].status = ST_ACTIVE;
            for (i = 0; i < nServers; i++) {        
                if (con[i].status == ST_ONLINE) { 
                    con[i].status = ST_STANDBY;
                    rr.op = ReplicationRequest::RR_STATUS;
                    rr.nodeId = i;
                    rr.status = ST_STANDBY;
                    TRACE_IMSG(("Send STANDBY status to node %d\n", i));
                    writeReq(i, rr); 
                } else if (con[i].status == ST_STANDBY) { 
                    rr.op = ReplicationRequest::RR_CHANGE_ACTIVE_NODE;
                    rr.nodeId = id;
                    con[i].status = rr.status = ST_RECOVERED;
                    TRACE_IMSG(("Send CHANGE_ACTIVE_NODE message to node %d\n", i));
                    writeReq(i, rr); 
                }
            }
        } else if (activeNodeId < id) { 
            rr.op = ReplicationRequest::RR_READY;
            rr.nodeId = id;
            masterNodeId = activeNodeId;
            TRACE_IMSG(("Send READY status to node %d\n", activeNodeId));
            if (!writeReq(activeNodeId, rr)) { 
                TRACE_IMSG(("Failed to send READY request to node %d\n", activeNodeId));
                goto pollNodes;
            }
        }
    } else if (activeNodeId >= 0) { 
        TRACE_IMSG(("Send RECOVERY request to node %d\n", activeNodeId));
        rr.op = ReplicationRequest::RR_RECOVERY;
        rr.size = file.getUpdateCountTableSize();
        rr.nodeId = id;
        if (!writeReq(activeNodeId, rr, file.diskUpdateCount, rr.size*sizeof(int))) { 
            TRACE_IMSG(("Failed to send RECOVERY request to node %d\n", activeNodeId));
            goto pollNodes;
        }
    } 
    handshake = false;;
    TRACE_IMSG(("My new status is %s\n", statusText[con[id].status]));
    if (con[id].status != ST_ACTIVE) { 
        dbCriticalSection cs(startCS);
        if (accessType == dbConcurrentRead) { 
            if (con[id].status == ST_RECOVERED) { 
                recoveredEvent.wait(startCS);
                dbTrace("Recovery of node %d is completed\n", id);
            }
            monitor->curr = header->curr;
        } else { 
            if (!standalone) { 
                startEvent.wait(startCS);
            } else {
                con[id].status = ST_ACTIVE;
            }
            baseAddr = (byte*)file.getAddr();
            header = (dbHeader*)baseAddr;
            if (opened) { 
                int curr = header->curr;
                monitor->curr = curr;
                offs_t* shadowIndex = (offs_t*)(baseAddr + header->root[curr].index);
                offs_t* currIndex = (offs_t*)(baseAddr + header->root[1-curr].index);
                for (size_t i = 0, size = header->root[curr].indexUsed; i < size; i++) { 
                    if (currIndex[i] != shadowIndex[i]) { 
                        currIndex[i] = shadowIndex[i];
                        file.markAsDirty(header->root[1-curr].index + i*sizeof(offs_t), sizeof(offs_t));
                    } 
                }
                this->currIndex = currIndex;
                header->size = file.getSize();
                header->root[1-curr].index = header->root[curr].shadowIndex;
                header->root[1-curr].indexSize = header->root[curr].shadowIndexSize;
                header->root[1-curr].shadowIndex = header->root[curr].index;
                header->root[1-curr].shadowIndexSize = header->root[curr].indexSize;
                header->root[1-curr].indexUsed = header->root[curr].indexUsed;
                header->root[1-curr].freeList  = header->root[curr].freeList; 
#ifdef DO_NOT_REUSE_OID_WITHIN_SESSION
                monitor->sessionFreeList[1-curr] = monitor->sessionFreeList[curr];
#endif
   
                file.markAsDirty(0, sizeof(dbHeader));
                file.updateCounter += MAX_LOST_TRANSACTIONS;
                restoreTablesConsistency();
                file.flush();
            } 
        }
    }
    if (opened) {         
        if (loadScheme(true)) { 
            if (accessType != dbConcurrentUpdate) {
                initMutex.done();
            }
            return true;
        } else { 
            if (accessType == dbConcurrentRead) { 
                do { 
                    dbTableDescriptor *desc, *next;
                    for (desc = tables; desc != NULL; desc = next) { 
                        next = desc->nextDbTable;
                        if (desc->cloneOf != NULL) {
                            delete desc;
                        } else if (!desc->fixedDatabase) { 
                            desc->db = NULL;
                        }
                    }
                    tables = NULL;
                    TRACE_IMSG(("Database schema was not yet initialized\n"));
                    endTransaction(); // release locks
                    dbThread::sleep(1);
                } while (!loadScheme(false));
                return true;
            }
            dbTrace("Failed to load database scheme\n");
            opened = false;
        }
    }
    if (accessType != dbConcurrentUpdate) {
        initMutex.done();
    }
#ifdef PROTECT_DATABASE
    if (accessType == dbConcurrentRead) { 
        file.protect(0, header->size);
    }
#endif
    readerThread.join();
    delete[] con; 
    delete acceptSock;
    dbDatabase::close0();
    return false;
}       


void thread_proc dbReplicatedDatabase::startReader(void* arg)
{
    ((dbReplicatedDatabase*)arg)->reader();
}

int dbReplicatedDatabase::getNumberOfOnlineNodes()
{
    int n = 0;
    for (int i = 0; i < nServers; i++) { 
        if (con[i].status != ST_OFFLINE) { 
            n += 1;
        }
    }
    return n;
}

void dbReplicatedDatabase::deleteConnection(int nodeId)
{
    dbTrace("Delete connection with node %d, used counter %d\n", nodeId, con[nodeId].useCount);
    { 
        dbCriticalSection cs(sockCS);
        socket_t* reqSock = con[nodeId].reqSock;
        socket_t* respSock = con[nodeId].respSock;
        while (con[nodeId].useCount > 1) { 
            con[nodeId].waitUseEventFlag += 1;
            con[nodeId].useCount -= 1;
            con[nodeId].useEvent.reset();
            con[nodeId].useEvent.wait(sockCS);
            con[nodeId].waitUseEventFlag -= 1;
            con[nodeId].useCount += 1;
        }
        FASTDB_ASSERT(con[nodeId].useCount == 1);
        con[nodeId].status = ST_OFFLINE;
        if (con[nodeId].reqSock != NULL) { 
            FASTDB_ASSERT(con[nodeId].reqSock == reqSock);
            FD_CLR(reqSock->get_handle(), &inputSD);    
            delete reqSock;
            con[nodeId].reqSock = NULL;
        }
        if (con[nodeId].respSock != NULL) { 
            FASTDB_ASSERT(con[nodeId].respSock == respSock);
            FD_CLR(respSock->get_handle(), &inputSD);    
            delete respSock;
            con[nodeId].respSock = NULL;
        }
    }
    if (nodeId == activeNodeId) { 
        changeActiveNode();
    }
}


void dbReplicatedDatabase::changeActiveNode() 
{
    activeNodeId = -1;
    TRACE_IMSG(("Change active node\n"));
    if (accessType != dbConcurrentRead && con[id].status == ST_STANDBY) {
        ReplicationRequest rr;
        int i;
        for (i = 0; i < id; i++) { 
            if (con[i].status == ST_ONLINE || con[i].status == ST_STANDBY) { 
                rr.op = ReplicationRequest::RR_GET_STATUS;
                TRACE_IMSG(("Send GET_STATUS request to node %d to choose new active node\n", i));
                rr.nodeId = id;
                if (writeReq(i, rr)) { 
                    return;
                }
            }
        }
        dbTrace("Activate stand-by server %d\n", id);
        for (i = 0; i < nServers; i++) { 
            if (con[i].status != ST_OFFLINE && i != id) { 
                TRACE_IMSG(("Send NEW_ACTIVE_NODE request to node %d\n", i));
                rr.op = ReplicationRequest::RR_NEW_ACTIVE_NODE;
                rr.nodeId = id;
                if (writeReq(i, rr)) { 
                    con[i].status = ST_STANDBY;
                }
            }
        }
        con[id].status = ST_ACTIVE;
        { 
            dbCriticalSection cs(startCS);
            startEvent.signal();
        }
    }
}
    
void dbReplicatedDatabase::stopReplica()
{
    dbCriticalSection cs(startCS);
    opened = false;
    startEvent.signal();
}

void dbReplicatedDatabase::reader()
{
    char buf[256];
    ReplicationRequest rr;
    bool statusRequested = false;
    bool closed = false;
    dbDatabaseThreadContext* ctx = NULL;
    
    if (accessType != dbAllAccess) { // concurrent update
        ctx = new dbDatabaseThreadContext();
        threadContext.set(ctx);
    }
        
    while (opened) { 
        timeval tv;                             
        bool handshake = this->handshake;
        if (handshake) { 
            tv.tv_sec = dbOpenPollInterval / 1000;
            tv.tv_usec = dbOpenPollInterval % 1000 * 1000; 
        } else { 
            tv.tv_sec = pollInterval / 1000;
            tv.tv_usec = pollInterval % 1000 * 1000; 
        }
        fd_set ready = inputSD;
        int rc = ::select(nInputSD, &ready, NULL, NULL, &tv);
#if 0 // for testing only
        static int nIterations = 0;
        if (id == 1 && ++nIterations == 50000) { 
            printf("Simulate timeout %d\n", nIterations);
            lockConnection(masterNodeId);
            deleteConnection(masterNodeId);
            unlockConnection(masterNodeId);
            rc = 0; // simulate timeout
        }
#endif
        if (rc == 0) { // timeout
            if (!closed && !handshake && con[id].status == ST_STANDBY) {
                if (statusRequested || activeNodeId < 0) { 
                    if (!onlineRecovery && accessType == dbConcurrentRead && activeNodeId < 0 && masterNodeId >= 0) {  
                        con[id].status = ST_RECOVERED;
                        TRACE_IMSG(("Try to reestablish connection with master node\n"));
                        socket_t* s = socket_t::connect(serverURL[masterNodeId], 
                                                      socket_t::sock_global_domain, 
                                                      1);      
                        if (!s->is_ok()) { 
                            s->get_error_text(buf, sizeof buf);
                            dbTrace("Failed to establish connection with master node: %s\n", buf);
                            delete s;
                        } else { 
                            activeNodeId = masterNodeId;
                            addConnection(masterNodeId, s);
                            TRACE_IMSG(("Connection with master is restored\n"));
                            rr.op = ReplicationRequest::RR_RECOVERY;
                            rr.size = file.getUpdateCountTableSize();
                            rr.nodeId = id;
                            if (writeReq(masterNodeId, rr, file.diskUpdateCount, rr.size*sizeof(int))) { 
                                TRACE_IMSG(("Perform recovery from master\n"));
                                activeNodeId = masterNodeId;
                                con[id].nRecoveredPages = 0;
                                onlineRecovery = true;
                                beginTransaction(dbDatabase::dbCommitLock);
                            } else { 
                                dbTrace("Connection with master is lost\n");
                            }
                        } 
                    } else { 
                        TRACE_MSG(("Initiate change of active node %d\n", activeNodeId));
                        changeActiveNode();
                    }
                } else { 
                    rr.op = ReplicationRequest::RR_GET_STATUS;
                    rr.nodeId = id;
                    if (!writeResp(activeNodeId, rr)) { 
                        dbTrace("Connection with active server is lost\n");
                        changeActiveNode();
                    } else { 
                        TRACE_MSG(("Send GET_STATUS request to node %d\n", activeNodeId));
                        statusRequested = true;
                    }
                }
            }
        } else if (rc < 0) {  
#ifndef _WIN32
            if (errno != EINTR) 
#endif
            { 
                dbTrace("Select failed: %d\n", errno);
                tv.tv_sec = 0;
                tv.tv_usec = 0;
                FD_ZERO(&ready);
                for (int i = nInputSD; --i >= 0;) {                 
                    FD_SET(i, &ready);
                    if (::select(i+1, &ready, NULL, NULL, &tv) < 0) { 
                        FD_CLR(i, &inputSD);
                        for (int j = nServers; --j >= 0;) { 
                            if (j != id) { 
                                lockConnection(j);
                                if ((con[j].respSock != NULL && con[j].respSock->get_handle() == i) ||
                                    (con[j].reqSock != NULL && con[j].reqSock->get_handle() == i))
                                { 
                                    deleteConnection(j);
                                }
                                unlockConnection(j);
                            }
                        }                           
                    }
                    FD_CLR(i, &ready);
                }
            }                   
        } else { 
            statusRequested = false;
            if (FD_ISSET(acceptSock->get_handle(), &ready)) { 
                socket_t* s = acceptSock->accept();
                if (s != NULL && s->read(&rr, sizeof rr)) {
                    int op = rr.op;
                    int nodeId = rr.nodeId;
                    if (op == ReplicationRequest::RR_RECOVERY) { 
                        int* updateCountTable = new int[file.getMaxPages()];
                        
                        if (rr.size != 0 && !s->read(updateCountTable, rr.size*sizeof(int))) {
                            dbTrace("Failed to read update counter table\n"); 
                            delete[] updateCountTable;
                            delete s;  
                        } else { 
                            cs.enter();
                            con[nodeId].status = ST_OFFLINE;
                            cs.leave();
                            addConnection(nodeId, s);
                            cs.enter();
                            con[nodeId].status = ST_RECOVERED;
                            con[nodeId].nRecoveredPages = 0;
                            cs.leave();
                            file.recovery(nodeId, updateCountTable, rr.size);
                        }
                    } else if (op != ReplicationRequest::RR_GET_STATUS) { 
                        dbTrace("Receive unexpected request %d\n", rr.op);
                        delete s;
                    } else if (nodeId >= nServers) { 
                        dbTrace("Receive request from node %d while master was configured only for %d nodes\n", rr.op, nServers);
                        delete s;                        
                    } else {
                        TRACE_IMSG(("Send STATUS response %s for GET_STATUS request from node %d\n", 
                                   statusText[con[id].status], nodeId));
                        rr.op = ReplicationRequest::RR_STATUS;
                        rr.nodeId = id;
                        rr.status = con[id].status;
                        rr.size = file.updateCounter;
                        if (!s->write(&rr, sizeof rr)) { 
                            s->get_error_text(buf, sizeof buf);
                            dbTrace("Failed to write response: %s\n", buf);
                            delete s;
                        } else { 
                            lockConnection(nodeId);
                            if (con[nodeId].respSock != NULL) { 
                                deleteConnection(nodeId);
                            }
                            { 
                                dbCriticalSection cs(sockCS);
                                con[nodeId].respSock = s;
                                socket_handle_t hnd = s->get_handle();
                                if ((int)hnd >= nInputSD) { 
                                    nInputSD = (int)hnd+1;
                                }
                                FD_SET(hnd, &inputSD);
                            }
                            unlockConnection(nodeId);
                        }
                    }
                } else if (s == NULL) { 
                    acceptSock->get_error_text(buf, sizeof buf);
                    dbTrace("Accept failed: %s\n", buf);
                } else { 
                    s->get_error_text(buf, sizeof buf);
                    dbTrace("Failed to read login request: %s\n", buf);
                    delete s;
                }
            }
            for (int i = nServers; --i >= 0;) { 
                if (i == id) {
                    continue;
                }
                lockConnection(i);
                socket_t* s;
                if (((s = con[i].respSock) != NULL && FD_ISSET(s->get_handle(), &ready)) 
                    || ((s = con[i].reqSock) != NULL && FD_ISSET(s->get_handle(), &ready)))

                { 
                    if (!s->read(&rr, sizeof rr)) { 
                        dbTrace("Failed to read response from node %d\n", i); 
                        deleteConnection(i);
                        if (closed && i == activeNodeId) { 
                            dbCriticalSection cs(startCS);
                            opened = false;
                            startEvent.signal();
                            delete ctx;
                            unlockConnection(i);
                            return;
                        } 
                    } else {
                        TRACE_MSG(("Receive request %s, status %s, size %d from node %d\n", 
                                   requestText[rr.op], 
                                   ((rr.status <= ST_RECOVERED) ? statusText[rr.status] : "?"), 
                                   rr.size, rr.nodeId));
                        switch (rr.op) { 
                          case ReplicationRequest::RR_COMMITTED:
                          {
                              dbCriticalSection cs(commitCS);
                              con[i].committedEvent.signal();
                              break;
                          }
                          case ReplicationRequest::RR_GET_STATUS:
                            rr.op = ReplicationRequest::RR_STATUS;
                            rr.nodeId = id;
                            rr.status = con[id].status;
                            rr.size = file.updateCounter;
                            TRACE_MSG(("Send RR_STATUS response %s to node %d\n", 
                                       statusText[con[id].status], i));
                            writeResp(i, rr);
                            break;
                          case ReplicationRequest::RR_STATUS:
                            if (con[rr.nodeId].status == ST_RECOVERED && rr.status == ST_STANDBY) { 
                                if (onlineRecovery) { 
                                    monitor->curr = header->curr;
                                    dbTrace("Online recovery of node %d is completed: recover %d pages\n", id, con[id].nRecoveredPages);
                                    endTransaction();
                                    onlineRecovery = false;
                                } else { 
                                    dbCriticalSection cs(startCS);
                                    dbTrace("Recovery of node %d is completed: recover %d pages\n", id, con[id].nRecoveredPages);
                                    monitor->curr = header->curr;
                                    recoveredEvent.signal();
                                }
                            }
                            con[rr.nodeId].status = rr.status;
                            con[rr.nodeId].updateCounter = rr.size;
                            if (con[rr.nodeId].waitStatusEventFlag) { 
                                con[rr.nodeId].statusEvent.signal();
                            } else if (activeNodeId < 0 && rr.nodeId < id && rr.status == ST_RECOVERED) { 
                                TRACE_MSG(("activeNodeId=%d rr.nodeId=%d, rr.status=%d\n", activeNodeId, rr.nodeId, rr.status)); 
                                changeActiveNode();
                            }
                            statusRequested = false;
                            break;
                          case ReplicationRequest::RR_NEW_ACTIVE_NODE:
                            TRACE_MSG(("New active node is %d\n", rr.nodeId));
                            activeNodeId = rr.nodeId;
                            statusRequested = false;
                            break;
                          case ReplicationRequest::RR_CHANGE_ACTIVE_NODE:
                            TRACE_IMSG(("Change active node to %d\n", rr.nodeId));
                            masterNodeId = activeNodeId = rr.nodeId;
                            statusRequested = false;
                            rr.op = ReplicationRequest::RR_RECOVERY;
                            rr.size = file.getUpdateCountTableSize();
                            rr.nodeId = id;
                            writeReq(activeNodeId, rr, file.diskUpdateCount, rr.size*sizeof(int));
                            con[id].nRecoveredPages = 0;
                            con[id].status = ST_RECOVERED;
                            if (accessType == dbConcurrentRead) { 
                                if (!onlineRecovery) { 
                                    onlineRecovery = true;
                                    beginTransaction(dbDatabase::dbCommitLock);
                                    TRACE_IMSG(("Initiate online recovery from master %d\n", activeNodeId));
                                } else {
                                    TRACE_IMSG(("Recovery from master %d in progress\n", activeNodeId));
                                }
                            }
                            break;
                          case ReplicationRequest::RR_CLOSE:
                            dbTrace("Receive close request from master\n"); 
                            if (accessType != dbConcurrentRead) { 
                                dbCriticalSection cs(startCS);
                                opened = false;
                                startEvent.signal();
                                closed = true;
                            }
                            break;
                          case ReplicationRequest::RR_RECOVERY: 
                            {
                                TRACE_IMSG(("Receive recovery request from node %d\n", rr.nodeId));
                                int* updateCountTable = new int[file.getMaxPages()];

                                if (rr.size != 0 && !s->read(updateCountTable, rr.size*sizeof(int))) {
                                    dbTrace("Failed to read update counter table\n"); 
                                    deleteConnection(i);
                                    delete[] updateCountTable;
                                } else { 
                                    con[i].status = ST_RECOVERED;
                                    file.recovery(i, updateCountTable, rr.size);
                                }
                                break;
                            }
                          case ReplicationRequest::RR_RECOVER_PAGE: 
                            TRACE_MSG(("Recovere page at address %x size %d\n", rr.page.offs, rr.size));
                            con[id].nRecoveredPages += rr.size >> dbModMapBlockBits;
                            if (!file.updatePages(s, rr.page.offs, rr.page.updateCount, rr.size))
                            { 
                                dbTrace("Failed to recover page %lx\n", (long)rr.page.offs);
                                activeNodeId = -1;
                            }
                            break;
                          case ReplicationRequest::RR_UPDATE_PAGE: 
                            FASTDB_ASSERT(!onlineRecovery);
                            TRACE_MSG(("Update page at address %x size %d\n", rr.page.offs, rr.size));
                            if ((accessType != dbAllAccess
                                 && !file.concurrentUpdatePages(s, rr.page.offs, rr.page.updateCount, rr.size)) 
                                || (accessType == dbAllAccess
                                    && !file.updatePages(s, rr.page.offs, rr.page.updateCount,
                                                         rr.size)))
                            { 
                                dbTrace("Failed to update page %lx\n", (long)rr.page.offs);
                                activeNodeId = -1;
                            }
                            break;
                          case ReplicationRequest::RR_READY:                        
                            con[rr.nodeId].readyEvent.signal();
                            break;
                          default:
                            dbTrace("Unexpected request %d from node %d\n", rr.op, i);
                        }
                    }
                }
                unlockConnection(i);
            }
        }
    }
    delete ctx;
}

void dbReplicatedDatabase::close0()
{
    detach();
    opened = false;
    readerThread.join();
    file.flush();
    if (header != NULL && header->dirty && accessType != dbConcurrentRead) 
    {
        header->dirty = false;
        file.markAsDirty(0, sizeof(dbHeader));
        file.flush(false);
    }
    ReplicationRequest rr;
    rr.op = ReplicationRequest::RR_CLOSE;
    rr.nodeId = id;
    for (int i = nServers; --i >= 0;) { 
        if (i != id
            && con[i].reqSock != NULL 
            && (con[i].status == ST_STANDBY || con[i].status == ST_RECOVERED)) 
        { 
            con[i].reqSock->write(&rr, sizeof rr);          
        }
    }
    dbDatabase::close0();
    delete[] con;
    delete acceptSock;
    startEvent.close();
}

bool dbReplicatedDatabase::writeReq(int nodeId, ReplicationRequest const& hdr, 
                                    void* body, size_t bodySize)
{
    bool result;
    lockConnection(nodeId);
    dbCriticalSection cs(con[nodeId].writeCS);
    socket_t* s = con[nodeId].reqSock;
    if (s == NULL) { 
        s = con[nodeId].respSock;
    }
    if (s != NULL) { 
        if (!s->write(&hdr, sizeof hdr, replicationWriteTimeout/1000) ||
            (bodySize != 0 && !s->write(body, bodySize, replicationWriteTimeout/1000)))
        {
            char buf[64];
            s->get_error_text(buf, sizeof buf);
            dbTrace("Failed to write request to node %d: %s\n", nodeId, buf);
            deleteConnection(nodeId);
            result = false;
        } else { 
            result = true;
        }
    } else { 
        TRACE_MSG(("Connection %d request socket is not opened\n", nodeId));
        result = false;     
    }
    unlockConnection(nodeId);
    return result;
}

bool dbReplicatedDatabase::writeResp(int nodeId, ReplicationRequest const& hdr)
{
    lockConnection(nodeId);
    bool result;
    socket_t* s = con[nodeId].respSock;
    if (s == NULL) { 
        s = con[nodeId].reqSock;
    }
    if (s != NULL) { 
        if (!s->write(&hdr, sizeof hdr)) { 
            char buf[64];
            s->get_error_text(buf, sizeof buf);
            dbTrace("Failed to write request to node %d: %s\n", nodeId, buf);
            deleteConnection(nodeId);
            result = false;
        } else { 
            result = true;
        }
    } else { 
        result = false;     
    }
    unlockConnection(nodeId);
    return result;
}

void dbReplicatedDatabase::lockConnection(int nodeId) 
{
    dbCriticalSection cs(sockCS);
    con[nodeId].useCount += 1;
}

void dbReplicatedDatabase::unlockConnection(int nodeId) 
{
    dbCriticalSection cs(sockCS);
    if (--con[nodeId].useCount == 0 && con[nodeId].waitUseEventFlag) {
        con[nodeId].useEvent.signal();
    }
}

void dbReplicatedDatabase::addConnection(int nodeId, socket_t* s)
{
    TRACE_MSG(("Add connection with node %d\n", nodeId));
    lockConnection(nodeId);
    if (con[nodeId].reqSock != NULL) { 
        deleteConnection(nodeId);
    }
    { 
        dbCriticalSection cs(sockCS);
        con[nodeId].reqSock = s;
        socket_handle_t hnd = s->get_handle();
        if ((int)hnd >= nInputSD) { 
            nInputSD = (int)hnd+1;
        }
        FD_SET(hnd, &inputSD);
    }
    unlockConnection(nodeId);
}

void dbReplicatedDatabase::waitTransactionAcknowledgement()
{
    int i;
    int n = nServers;
    dbCriticalSection cs(commitCS);
    for (i = 0; i < n; i++) { 
        con[i].committedEvent.reset();
    }
    file.flush();    
    for (i = 0; i < n; i++) { 
        if (con[i].status == dbReplicatedDatabase::ST_STANDBY) { 
            con[i].committedEvent.wait(commitCS);
        }
    }
}

#endif

bool dbHeader::isCompatible()
{
    return getVersion() < 329 || getCurrentMode() == mode;
}

int dbHeader::getCurrentMode()
{
    int mode = (getVersion() < 374 ? MODE_RECTANGLE_DIM_OLD : MODE_RECTANGLE_DIM) * RECTANGLE_DIMENSION * sizeof(RECTANGLE_COORDINATE_TYPE);
#if dbDatabaseOffsetBits > 32
    mode |= MODE_OFFS_64;
#endif        
#if dbDatabaseOidBits > 32
    mode |= MODE_OID_64;
#endif        
#ifdef AUTOINCREMENT_SUPPORT    
    mode |= MODE_AUTOINCREMENT;
#endif
    return mode;
}

dbTransactionLogger::~dbTransactionLogger() {}

inline char* dbFileTransactionLogger::extend(size_t size)
{
    size = DOALIGN(size, 8);
    if (used + size > allocated) { 
        size_t newSize = used + size > allocated*2 ? used + size : allocated*2;
        char* newBuf = new char[newSize];
        allocated = newSize;
        memcpy(newBuf, buf, used);
        delete[] buf;
        buf = newBuf;
    }
    char* dst = buf + used;
    used += size;
    return dst;
}

struct TransLogObjectHeader 
{ 
    int   kind;
    nat4  size;
    oid_t oid;
    oid_t table;
};

struct TransLogTransHeader 
{
    nat4 size;
    nat4 crc;
};

void dbFileTransactionLogger::append(OperationKind kind, dbTableDescriptor* table, oid_t oid, dbRecord const* body)
{
    TransLogObjectHeader* hdr = (TransLogObjectHeader*)extend(sizeof(TransLogObjectHeader));
    hdr->kind = kind;
    hdr->oid = oid;
    hdr->table = table->getId();
    if (body != NULL) { 
        hdr->size = body->size;
        memcpy(extend(body->size), body, body->size);
    }
}

bool dbFileTransactionLogger::insert(dbTableDescriptor* table, oid_t oid, dbRecord const* dbsObj, void const* appObj)
{
    append(opInsert, table, oid, dbsObj);
    return true;
}

bool dbFileTransactionLogger::update(dbTableDescriptor* table, oid_t oid, dbRecord const* dbsObj, void const* appObj)
{
    append(opUpdate, table, oid, dbsObj);
    return true;
}

bool dbFileTransactionLogger::remove(dbTableDescriptor* table, oid_t oid)
{
    append(opRemove, table, oid, NULL);
    return true;
}

inline nat4 calculate_crc(void const* content, size_t content_length, nat4 crc)
{
    static const nat4 table [] = {
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
        0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
        0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
        0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
        
        0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
        0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
        0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
        0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
        
        0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
        0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
        0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
        0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
        
        0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
        0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
        0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
        0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
        
        0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
        0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
        0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
        0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
        
        0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
        0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
        0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
        0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
        
        0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
        0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
        0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
        0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
        
        0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
        0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
        0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
        0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
        
        0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
        0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
        0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
        0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
        
        0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
        0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
        0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
        0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
        
        0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
        0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
        0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
        0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
        
        0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
        0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
        0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
        0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
        
        0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
        0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
        0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
        0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
        
        0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
        0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
        0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
        0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
        
        0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
        0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
        0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
        0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
        
        0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
        0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
        0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
        0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
    };
    
    unsigned char* buffer = (unsigned char*)content;

    while (content_length-- != 0) {
          crc = (crc >> 8) ^ table[(crc & 0xFF) ^ *buffer++];
    }
    return crc;
}

bool dbFileTransactionLogger::commitPhase1()
{
    TransLogTransHeader* hdr = (TransLogTransHeader*)buf;
    hdr->size = used - sizeof(TransLogTransHeader);
    hdr->crc = crc ? calculate_crc(buf + sizeof(TransLogTransHeader), used - sizeof(TransLogTransHeader), ~0) : 0;
    bool rc = log.write(buf, used);
    used = sizeof(TransLogTransHeader);
    return rc;
}

void dbFileTransactionLogger::commitPhase2()
{
    log.flush();
}

void dbFileTransactionLogger::rollback()
{
    used = sizeof(TransLogTransHeader);
}
 
dbFileTransactionLogger::dbFileTransactionLogger()
{
    allocated = dbPageSize;
    buf = new char[allocated];
    used = sizeof(TransLogTransHeader);
    crc = true;
}

dbFileTransactionLogger::~dbFileTransactionLogger()
{
    delete[] buf;
}
 
bool dbFileTransactionLogger::open(char_t const* path, int flags, bool crc)
{
    this->crc = crc;
    used = sizeof(TransLogTransHeader);
    if (log.create(path, flags) == dbFile::ok) { 
        if (!(flags & (dbFile::truncate|dbFile::read_only))) { 
            log.seek(0, SEEK_END);
        }
        return true;
    }
    return false;
}

void dbFileTransactionLogger::close()
{
    log.close();
}

dbFileTransactionLogger::RestoreStatus dbFileTransactionLogger::restore(dbDatabase& db, size_t& nTrans)
{
    TransLogTransHeader transHdr;
    size_t n;
    dbSmallBuffer transBuf;
    dbSmallBuffer recBuf;
    nTrans = 0;
    log.seek(0, SEEK_SET);
    while (log.read(&transHdr, n, sizeof(TransLogTransHeader)) == dbFile::ok && n == sizeof(TransLogTransHeader)) { 
        transBuf.put(transHdr.size);
        if (log.read(transBuf.base(), n, transHdr.size) != dbFile::ok || n != transHdr.size) { 
            return rsReadFailed;
        }
        if (crc) { 
            if (transHdr.crc != calculate_crc(transBuf.base(), transHdr.size, ~0)) { 
                return rsCRCMismatch;
            }
        }
        char* cur = transBuf.base();
        char* end = cur + transHdr.size;
        while (cur < end) { 
            TransLogObjectHeader& objHdr = *(TransLogObjectHeader*)cur;          
            dbTableDescriptor* table = db.findTableByID(objHdr.table);
            if (table == NULL) { 
                return rsTableNotFound;
            }
            cur += sizeof(TransLogObjectHeader);

            if (objHdr.kind == opRemove) { 
                db.remove(table, objHdr.oid);
            } else { 
                recBuf.put(table->size());
                memset(recBuf.base(), 0, table->size());
                table->columns->fetchRecordFields((byte*)recBuf.base(), (byte*)cur);
                cur += DOALIGN(objHdr.size, 8);

                if (objHdr.kind == opInsert) { 
                    dbAnyReference ref;
                    db.insertRecord(table, &ref, recBuf.base());
                    if (ref.getOid() != objHdr.oid) { 
                        return rsOIDMismatch;
                    }
                } else { 
                    db.update(objHdr.oid, table, recBuf.base());
                }
            }
        }
        nTrans += 1;
    }
    return rsOK;
}
            

END_FASTDB_NAMESPACE
