//-< XML.CPP >-------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     14-Feb-2008  K.A. Knizhnik  * / [] \ *
//                          Last update: 14-Feb-2008  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// XML export/import and related stuff
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "fastdb.h"
#include "compiler.h"
#include "symtab.h"
#include "hashtab.h"
#include "ttree.h"
#include "rtree.h"

BEGIN_FASTDB_NAMESPACE

class dbTmpAllocator { 
    enum { 
        CHUNK_SIZE = 4096
    };
    struct Chunk { 
        Chunk* next;
        Chunk* prev; // is not used, added for alignment
    };
    Chunk* curr;
    size_t used;

  public:
    dbTmpAllocator() { 
        curr = NULL;
        used = CHUNK_SIZE;
    }

    ~dbTmpAllocator() { 
        reset();
    }

    void reset() { 
        Chunk *c, *next; 
        for (c = curr; c != NULL; c = next) { 
            next = c->next;
            dbFree(c);
        }
        curr = NULL;
        used = CHUNK_SIZE;
    }


    void* alloc(size_t size) { 
        size = DOALIGN(size, 8);
        if (size > CHUNK_SIZE/2) { 
            Chunk* newChunk = (Chunk*)dbMalloc(size + sizeof(Chunk));
            if (curr != NULL) { 
                newChunk->next = curr->next;
                curr->next = newChunk;
            } else { 
                curr = newChunk;
                newChunk->next = NULL;
                used = CHUNK_SIZE;
            }
            return newChunk+1;
        } else if (size <= CHUNK_SIZE - used) { 
            used += size;
            return (char*)curr + used - size;
        } else { 
            Chunk* newChunk = (Chunk*)dbMalloc(CHUNK_SIZE);
            used = sizeof(Chunk) + size;
            newChunk->next = curr;
            curr = newChunk;
            return newChunk+1;
        }
    }
};

#if !STLPORT_WITH_VC6
#ifdef USE_STD_STRING
template <class T>
class std_tmp_allocator {
  public:
    dbTmpAllocator* allocator;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    template<class _Other>
    std_tmp_allocator(std_tmp_allocator<_Other> const& other) : allocator(other.allocator) {}
    std_tmp_allocator(dbTmpAllocator& al) : allocator(&al) {}
    std_tmp_allocator() : allocator(NULL) {}
    
	template <class _Other>
    struct rebind  { // convert an std_tmp_allocator<T> to an std_tmp_allocator<_Other>
		typedef std_tmp_allocator<_Other> other;
	};

    pointer allocate(size_type _Count)
    {	// allocate array of _Count elements
        return (allocate(_Count, (pointer)0));
    }

    pointer allocate(size_type _Count, const void*) 
    {
        return (pointer)allocator->alloc(_Count*sizeof(T));
    }

    void deallocate(pointer, size_t) {
    }
 
	void construct(pointer _Ptr, const T& _Val)
    {	// construct object at _Ptr with value _Val
		new((void*)_Ptr) T(_Val);
    }

	void destroy(pointer _Ptr)
    {	// destroy *_Ptr
		_Ptr->~T();
    }

    size_t max_size() const {
        size_t n = (size_t)(-1) / sizeof(T);
        return (0 < n ? n : 1);
    }
};

#ifndef _THROW0
#define _THROW0 throw
#endif

template<class _Ty, class _Other> inline
bool operator==(const std_tmp_allocator<_Ty>&, const std_tmp_allocator<_Other>) _THROW0()
{	// test for allocator equality (always true)
    return (true);
}

typedef std::basic_string<char,std::char_traits<char>,std_tmp_allocator<char> > tmp_basic_string;
typedef std::basic_string<wchar_t,std::char_traits<wchar_t>,std_tmp_allocator<wchar_t> > tmp_basic_wstring;
#endif
#endif // STLPORT_WITH_VC6

class dbXmlContext {
  public:
    oid_t*  oidMap;
    oid_t   oidMapSize;
    dbTmpAllocator tmpAlloc;    
#if !STLPORT_WITH_VC6
#ifdef USE_STD_STRING
    std_tmp_allocator<char> stdStringAlloc;
    std_tmp_allocator<wchar_t> stdWStringAlloc;
    dbXmlContext() : oidMap(NULL), stdStringAlloc(tmpAlloc), stdWStringAlloc(tmpAlloc) {}
#else
    dbXmlContext() : oidMap(NULL) {}
#endif
#else // STLPORT_WITH_VC6
    dbXmlContext() : oidMap(NULL) {}
#endif // STLPORT_WITH_VC6
    ~dbXmlContext() { 
        delete[] oidMap;
    }
};

class dbXmlScanner { 
  public:
    enum { 
        MaxIdentSize = 256
    };
    enum token { 
        xml_ident, 
        xml_sconst, 
        xml_iconst, 
        xml_fconst, 
        xml_lt, 
        xml_gt, 
        xml_lts, 
        xml_gts,
        xml_eq, 
        xml_eof,
        xml_error
    };    
    dbXmlScanner(FILE* f) { 
        in = f;
        sconst = new char[size = 1024];
        line = 1;
        pos = 0;
    }

    ~dbXmlScanner() {
        delete[] sconst;
    }

    token scan();

    void warning(char const* msg)           
    {
        fprintf(stderr, "%s at line %d position %d\n", msg, line, pos > 0 ? pos - 1 : 0);
    }

    char* getString() { 
        return sconst;
    }

    char* getIdentifier() { 
        return ident;
    }

    size_t  getStringLength() { 
        return slen;
    }

    db_int8 getInt() { 
        return iconst;
    }

    double getReal() { 
        return fconst;
    }

    bool expect(int sourcePos, token expected) { 
        token tkn = scan();
        if (tkn != expected) { 
            fprintf(stderr, "xml.cpp:%d: line %d, column %d: Get token %d instead of expected token %d\n", 
                    sourcePos, line, pos, tkn, expected);
            return false;
        }
        return true;
    }

    bool expect(int sourcePos, char* expected) { 
        token tkn = scan();
        if (tkn != xml_ident) { 
            fprintf(stderr, "xml.cpp:%d: line %d, column %d: Get token %d instead of expected identifier\n", 
                    sourcePos, line, pos, tkn);
            return false;
        }
        if (strcmp(ident, expected) != 0) { 
            fprintf(stderr, "xml.cpp:%d: line %d, column %d: Get tag '%s' instead of expected '%s'\n", 
                    sourcePos, line, pos, ident, expected);
            return false;
        }
        return true;
    }

  private:
    int   get();
    void  unget(int ch);

    int       line;
    int       pos;
    FILE*     in;
    char*     sconst;
    size_t    size;
    size_t    slen;
    db_int8   iconst;
    double    fconst;
    char      ident[MaxIdentSize];
};


static void exportString(FILE* out, char* src, int len)
{
    fprintf(out, "\"");
    while (--len > 0) { 
        byte b = (byte)*src++;
        switch (b) { 
          case '&':
            fprintf(out, "&amp;");
            break;
          case '<':
            fprintf(out, "&lt;");
            break;
          case '>':
            fprintf(out, "&gt;");
            break;
          case '"':
            fprintf(out, "&quot;");
            break;
          case '\'':
            fprintf(out, "&apos;");
            break;
          default:
            fprintf(out, "%c", b);
        }
    }
    fprintf(out, "\"");
}

static void exportWString(FILE* out, wchar_t* src, int len)
{
    fprintf(out, "\"");
    while (--len > 0) { 
        wchar_t ch = *src++;
        switch (ch) { 
          case '&':
            fprintf(out, "&amp;");
            break;
          case '<':
            fprintf(out, "&lt;");
            break;
          case '>':
            fprintf(out, "&gt;");
            break;
          case '"':
            fprintf(out, "&quot;");
            break;
          case '\'':
            fprintf(out, "&apos;");
            break;
          default:
            fprintf(out, "%lc", ch);
        }
    }
    fprintf(out, "\"");
}

    
static void exportBinary(FILE* out, byte* src, int len)
{
    fprintf(out, "\"");
    while (--len >= 0) { 
        fprintf(out, "%02X", *src++);
    }
    fprintf(out, "\"");
}

static void exportRecord(dbFieldDescriptor* fieldList, FILE* out, byte* src, int indent)
{
    int i;
    dbFieldDescriptor* fd = fieldList;
    do {
        byte* ptr = src + fd->dbsOffs;
        for (i = indent; --i >= 0;) { 
            fprintf(out, " ");
        }
        char* fieldName = fd->name;
        if (strcmp(fieldName, "[]") == 0) { 
            fieldName = "array-element";
        }
        fprintf(out, "<%s>", fieldName);
        switch (fd->type) {
          case dbField::tpBool:
            fprintf(out, "%d", *(bool*)ptr);
            break;
          case dbField::tpInt1:
            fprintf(out, "%d", *(int1*)ptr);
            break;
          case dbField::tpInt2:
            fprintf(out, "%d", *(int2*)ptr);
            break;
          case dbField::tpInt4:
            fprintf(out, "%d", *(int4*)ptr);
            break;
          case dbField::tpInt8:
            fprintf(out, INT8_FORMAT, *(db_int8*)ptr);
            break;
          case dbField::tpReal4:
            fprintf(out, "%.8G", *(real4*)ptr);
            break;
          case dbField::tpReal8:
            fprintf(out, "%.16G", *(real8*)ptr);
            break;
          case dbField::tpRawBinary:
            exportBinary(out, src+fd->dbsOffs, (int)fd->dbsSize);
            break;
          case dbField::tpString:
            exportString(out, (char*)(src + ((dbVarying*)ptr)->offs), ((dbVarying*)ptr)->size);
            break;
          case dbField::tpWString:
            exportWString(out, (wchar_t*)(src + ((dbVarying*)ptr)->offs), ((dbVarying*)ptr)->size);
            break;
          case dbField::tpArray:
            {
                int nElems = ((dbVarying*)ptr)->size;
                byte* srcElem = src + ((dbVarying*)ptr)->offs;
                dbFieldDescriptor* element = fd->components;
                fprintf(out, "\n");
                while (--nElems >= 0) {
                    exportRecord(element, out, srcElem, indent+1);
                    srcElem += element->dbsSize;
                }
                for (i = indent; --i >= 0;) { 
                    fprintf(out, " ");
                }
                break;
            }
          case dbField::tpReference:
            fprintf(out, "<ref id=\"%ld\"/>", (long)*(oid_t*)ptr);
            break;
          case dbField::tpRectangle:
            { 
                rectangle& r = *(rectangle*)ptr;
                fprintf(out, "<rectangle><vertex");
                for (i = 0; i < rectangle::dim; i++) { 
                    fprintf(out, " c%d=\"%d\"", i, r.boundary[i]);
                }
                fprintf(out, "/><vertex");
                for (i = 0; i < rectangle::dim; i++) { 
                    fprintf(out, " c%d=\"%d\"", i, r.boundary[rectangle::dim+i]);
                }
                fprintf(out, "/></rectangle>");
            }
            break;
          case dbField::tpStructure:
            fprintf(out, "\n");
            exportRecord(fd->components, out, src, indent+1);
            for (i = indent; --i >= 0;) { 
                fprintf(out, " ");
            }
            break;
        }
        fprintf(out, "</%s>\n", fieldName);
    } while ((fd = fd->next) != fieldList);
}

void dbDatabase::exportScheme(FILE* out) 
{
    fprintf(out, "<!DOCTYPE database [\n");
    if (tables == NULL) { 
        fprintf(out, "<!ELEMENT database EMPTY>\n");
    } else { 
        fprintf(out, "<!ELEMENT database (%s*", tables->name);
        dbTableDescriptor* desc = tables; 
        while ((desc = desc->nextDbTable) != NULL) { 
            if (desc->tableId != dbMetaTableId) {
                fprintf(out, ", %s*", desc->name);
            }
        }
        fprintf(out, ")>\n");
    }
    for (dbTableDescriptor* desc = tables; desc != NULL; desc = desc->nextDbTable) { 
        if (desc->tableId != dbMetaTableId) {
            exportClass(out, desc->name, desc->columns);
            fprintf(out, "<!ATTLIST %s id CDATA #REQUIRED>\n", desc->name);
        }
    }
    fprintf(out, "<!ELEMENT array-element ANY>\n<!ELEMENT ref EMPTY>\n<!ATTLIST ref id CDATA #REQUIRED>\n]>\n");
}

void dbDatabase::exportClass(FILE* out, char* name, dbFieldDescriptor* fieldList) 
{
    fprintf(out, "<!ELEMENT %s (%s", name, fieldList->name);
    dbFieldDescriptor* fd = fieldList;
    while ((fd = fd->next) != fieldList) {
        fprintf(out, ", %s", fd->name);
    }
    fprintf(out, ")>\n");
    fd = fieldList;
    do { 
        switch (fd->type) {
          case dbField::tpStructure:
            exportClass(out, fd->name, fd->components);
            break;
          case dbField::tpArray:
            fprintf(out, "<!ELEMENT %s (array-element*)>\n", fd->name);
            if (fd->components->type == dbField::tpStructure) { 
                exportClass(out, fd->name, fd->components->components);
            }
            break;
          default:
            fprintf(out, "<!ELEMENT %s (#PCDATA)>\n", fd->name);
            
        } 
    } while ((fd = fd->next) != fieldList);
}


bool dbDatabase::exportDatabaseToXml(char_t const* file, char const* encoding, bool dtd) 
{
    FILE* f = _tcscmp(file, _T("-")) == 0 ? stdout : _tfopen(file, _T("w"));
    if (f != NULL) { 
        exportDatabaseToXml(f, encoding, dtd);
        if (f != stdout) { 
            fclose(f);
        }
        return true;
    }
    return false;
}

void dbDatabase::exportDatabaseToXml(FILE* out, char const* encoding, bool dtd) 
{
    fprintf(out, "<?xml version=\"1.0\" encoding=\"%s\"?>\n", encoding);
    beginTransaction(dbSharedLock);
    if (tables == NULL) { 
        loadMetaTable();
    }
    if (dtd) { 
        exportScheme(out);
    }
    fprintf(out, "<database>\n");
    for (dbTableDescriptor* desc=tables; desc != NULL; desc=desc->nextDbTable) { 
        if (desc->tableId != dbMetaTableId) {
            dbTable* table = (dbTable*)getRow(desc->tableId);    
            oid_t oid = table->firstRow; 
            int n = table->nRows;
            int percent = 0;
            for (int i = 0; oid != 0; i++) { 
                dbRecord* rec = getRow(oid);
                fprintf(out, " <%s id=\"%lu\">\n", desc->name, (unsigned long)oid); 
                exportRecord(desc->columns, out, (byte*)rec, 2);
                fprintf(out, " </%s>\n", desc->name); 
                oid = rec->next;
                int p = (i+1)*100/n;
                if (p != percent) { 
                    fprintf(stderr, "Exporting table %s: %d%%\r", desc->name, p);
                    fflush(stderr);
                    percent = p;
                }
            }
            fprintf(stderr, "Exporting table %s: 100%%   \n", desc->name);
        }
    }
    fprintf(out, "</database>\n");
}

inline int dbXmlScanner::get()
{
    int ch = getc(in);
    if (ch == '\n') {
        pos = 0;
        line += 1;
    } else if (ch == '\t') {
        pos = DOALIGN(pos + 1, 8);
    } else {
        pos += 1;
    }
    return ch;
}

inline void dbXmlScanner::unget(int ch) {
    if (ch != EOF) {
        if (ch != '\n') {
            pos -= 1;
        } else {
            line -= 1;
        }
        ungetc(ch, in);
    }
}


dbXmlScanner::token dbXmlScanner::scan()
{
    int ch, i, pos;
    bool floatingPoint;

  retry:
    do {
        if ((ch = get()) == EOF) {
            return xml_eof;
        }
    } while (ch <= ' ');
    
    switch (ch) { 
      case '<':
        ch = get();
        if (ch == '?') { 
            while ((ch = get()) != '?') { 
                if (ch == EOF) { 
                    return xml_error;
                }
            }
            if ((ch = get()) != '>') { 
                return xml_error;
            }
            goto retry;
        } else if (ch == '!') {
            int nParen = 1;
            do { 
                switch (get()) {
                  case '<':
                    nParen += 1;
                    break;
                  case '>':
                    nParen -= 1;
                    break;
                  case EOF:
                    return xml_error;
                }
            } while (nParen != 0);
            goto retry;
        } 
        if (ch != '/') { 
            unget(ch);
            return xml_lt;
        }
        return xml_lts;
      case '>':
        return xml_gt;
      case '/':
        ch = get();
        if (ch != '>') { 
            unget(ch);
            return xml_error;
        }
        return xml_gts;
      case '=':
        return xml_eq;
      case '"':
        i = 0;
        while (true) { 
            ch = get();
            switch (ch) { 
              case EOF:
                return xml_error;
              case '&':
                switch (get()) { 
                  case 'a':
                    ch = get();
                    if (ch == 'm' && get() == 'p' && get() == ';') { 
                        ch = '&';
                    } else if (ch == 'p' && get() == 'o' && get() == 's' && get() == ';') { 
                        ch = '\'';
                    } else { 
                        return xml_error;
                    }
                    break;
                  case 'l':
                    if (get() != 't' || get() != ';') { 
                        return xml_error;
                    }
                    ch = '<';
                    break;
                  case 'g':
                    if (get() != 't' || get() != ';') { 
                        return xml_error;
                    }
                    ch = '>';
                    break;
                  case 'q':
                    if (get() != 'u' || get() != 'o' || get() != 't' || get() != ';') { 
                        return xml_error;
                    }
                    ch = '"';
                    break;
                  default:
                    return xml_error;
                }
                break;
              case '"':
                slen = i;
                sconst[i] = 0;
                return xml_sconst;
            }
            if ((size_t)i+1 >= size) { 
                char* newBuf = new char[size *= 2];
                memcpy(newBuf, sconst, i);
                delete[] sconst;
                sconst = newBuf;
            }
            sconst[i++] = (char)ch;
        } 
      case '-': case '+':
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        i = 0;
        floatingPoint = false;
        while (true) { 
            if ((size_t)i == size) { 
                return xml_error;
            }
            if (!isdigit(ch) && ch != '-' && ch != '+' && ch != '.' && ch != 'E') { 
                unget(ch);
                sconst[i] = '\0';
                slen = i;
                if (floatingPoint) { 
                   return sscanf(sconst, "%lf%n", &fconst, &pos) == 1 && pos == i
                       ? xml_fconst : xml_error;
                } else { 
                    return sscanf(sconst, INT8_FORMAT "%n", &iconst, &pos) == 1 && pos == i
                       ? xml_iconst : xml_error;
                }
            }
            sconst[i++] = (char)ch;
            if (ch == '.' || ch == 'E') { 
                floatingPoint = true;
            }
            ch = get();
        }
      default:
        i = 0;
        while (isalnum(ch) || ch == '-' || ch == ':' || ch == '_') { 
            if (i == MaxIdentSize) { 
                return xml_error;
            }
            ident[i++] = (char)ch;
            ch = get();
        }
        unget(ch);
        if (i == MaxIdentSize || i == 0) { 
            return xml_error;            
        }
        ident[i] = '\0';
        return xml_ident;
    }
}

#define EXPECT(x) scanner.expect(__LINE__, x)

static bool skipElement(dbXmlScanner& scanner) 
{
    int depth = 1;
    do {  
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_lt:
            depth += 1;
            continue;
          case dbXmlScanner::xml_lts:
            depth -= 1;
            if (depth < 0 || !EXPECT(dbXmlScanner::xml_ident) || !EXPECT(dbXmlScanner::xml_gt))
            { 
                return false;
            }
            break;
          case dbXmlScanner::xml_gts:
            depth -= 1;
            break;
          default:
            continue;            
        }
    } while (depth != 0);

    return true;
}

bool dbDatabase::importRecord(char* terminator, dbFieldDescriptor* fieldList, byte* rec, dbXmlScanner& scanner) 
{
    dbXmlScanner::token tkn;

    while ((tkn = scanner.scan()) != dbXmlScanner::xml_lts) { 
        if (tkn != dbXmlScanner::xml_lt || !EXPECT(dbXmlScanner::xml_ident)
            || !EXPECT(dbXmlScanner::xml_gt)) 
        { 
            return false;
        }
        char* fieldName = scanner.getIdentifier();
        dbSymbolTable::add(fieldName, tkn_ident, FASTDB_CLONE_ANY_IDENTIFIER);
        dbFieldDescriptor* fd = fieldList;
        while (true) {
            if (fd->name == fieldName) {
                if (!importField(fd->name, fd, rec, scanner)) { 
                    return false;
                }
                break;
            }
            if ((fd = fd->next) == fieldList) { 
                if (!skipElement(scanner)) { 
                    return false;
                }   
                break;
            }
        } 
    }
    return EXPECT(terminator) && EXPECT(dbXmlScanner::xml_gt);
} 

#define HEX_DIGIT(ch) ((ch) >= 'a' ? ((ch) - 'a' + 10) : (ch) >= 'A' ? (((ch) - 'A' + 10)) : ((ch) - '0'))

bool dbDatabase::importField(char* terminator, dbFieldDescriptor* fd, byte* rec, dbXmlScanner& scanner) 
{
    dbXmlScanner::token tkn;
    int i;
    long id;
    byte* dst = rec + fd->appOffs;
    
    switch (fd->appType) { 
      case dbField::tpStructure:
        return importRecord(terminator, fd->components, dst, scanner);
      case dbField::tpArray:
      { 
          int arrSize = 8;
          int elemSize = (int)fd->components->appSize;
          byte* arr = (byte*)xmlContext->tmpAlloc.alloc(elemSize*arrSize);
          memset(arr, 0, elemSize*arrSize);
          for (i = 0; (tkn = scanner.scan()) == dbXmlScanner::xml_lt; i++) { 
              if (!EXPECT("array-element")
                  || !EXPECT(dbXmlScanner::xml_gt))
              {
                  return false;
              }
              if (i == arrSize) { 
                  arrSize *= 2;
                  byte* newArr = (byte*)xmlContext->tmpAlloc.alloc(elemSize*arrSize);
                  memcpy(newArr, arr, i*elemSize);
                  memset(newArr + i*elemSize, 0, i*elemSize);
                  arr = newArr;
              }
              importField("array-element", fd->components, arr + i*elemSize, scanner);
          }
          dbAnyArray::arrayAllocator((dbAnyArray*)dst, arr, i); 
          return tkn == dbXmlScanner::xml_lts
              && EXPECT(terminator)
              && EXPECT(dbXmlScanner::xml_gt);
      }
      case dbField::tpReference:
        if (!EXPECT(dbXmlScanner::xml_lt)
            || !EXPECT("ref")
            || !EXPECT("id")
            || !EXPECT(dbXmlScanner::xml_eq)
            || !EXPECT(dbXmlScanner::xml_sconst)
            || sscanf(scanner.getString(), "%ld", &id) != 1
            || !EXPECT(dbXmlScanner::xml_gts))
        { 
            return false;
        }
        *(oid_t*)dst = mapId(id);
        break;
      case dbField::tpRectangle:
        if (!EXPECT(dbXmlScanner::xml_lt)
            || !EXPECT("rectangle")
            || !EXPECT(dbXmlScanner::xml_gt)
            || !EXPECT(dbXmlScanner::xml_lt)
            || !EXPECT("vertex"))
        { 
            return false;
        } else {
            rectangle& r = *(rectangle*)dst;
            for (i = 0; i < rectangle::dim; i++) { 
                if (!EXPECT(dbXmlScanner::xml_ident)
                    || !EXPECT(dbXmlScanner::xml_eq)
                    || !EXPECT(dbXmlScanner::xml_sconst)
                    || sscanf(scanner.getString(), "%d", &r.boundary[i]) != 1)
                {
                    return false;
                }
            }
            if (!EXPECT(dbXmlScanner::xml_gts)
                || !EXPECT(dbXmlScanner::xml_lt)
                || !EXPECT("vertex"))
            {
                return false;
            }
            for (i = 0; i < rectangle::dim; i++) { 
                if (!EXPECT(dbXmlScanner::xml_ident)
                    || !EXPECT(dbXmlScanner::xml_eq)
                    || !EXPECT(dbXmlScanner::xml_sconst)
                    || sscanf(scanner.getString(), "%d", &r.boundary[rectangle::dim+i]) != 1)
                {
                    return false;
                }
            }
            if (!EXPECT(dbXmlScanner::xml_gts)
                || !EXPECT(dbXmlScanner::xml_lts)
                || !EXPECT("rectangle")
                || !EXPECT(dbXmlScanner::xml_gt))
            {
                return false;
            }
            break;
        }
      case dbField::tpBool:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
            *(bool*)dst = scanner.getInt() != 0;
            break;
          case dbXmlScanner::xml_fconst:
            *(bool*)dst = scanner.getReal() != 0.0;
            break;
          default:
            scanner.warning("Failed to convert field");
        }
        break;            
      case dbField::tpInt1:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
            *(int1*)dst = (int1)scanner.getInt();
            break;
          case dbXmlScanner::xml_fconst:
            *(int1*)dst = (int1)scanner.getReal();
            break;
          default:
            scanner.warning("Failed to convert field");
        }
        break;            
      case dbField::tpInt2:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
            *(int2*)dst = (int2)scanner.getInt();
            break;
          case dbXmlScanner::xml_fconst:
            *(int2*)dst = (int2)scanner.getReal();
            break;
          default:
            scanner.warning("Failed to convert field");
        }
        break;            
      case dbField::tpInt4:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
            *(int4*)dst = (int4)scanner.getInt();
            break;
          case dbXmlScanner::xml_fconst:
            *(int4*)dst = (int4)scanner.getReal();
            break;
          default:
            scanner.warning("Failed to convert field");
        }
        break;            
      case dbField::tpInt8:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
            *(db_int8*)dst = scanner.getInt();
            break;
          case dbXmlScanner::xml_fconst:
            *(db_int8*)dst = (db_int8)scanner.getReal();
            break;
          default:
            scanner.warning("Failed to convert field");
        }
        break;            
      case dbField::tpReal4:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
            *(real4*)dst = (real4)scanner.getInt();
            break;
          case dbXmlScanner::xml_fconst:
            *(real4*)dst = (real4)scanner.getReal();
            break;
          default:
            scanner.warning("Failed to convert field");
        }
        break;            
      case dbField::tpReal8:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
            *(real8*)dst = (real8)scanner.getInt();
            break;
          case dbXmlScanner::xml_fconst:
            *(real8*)dst = scanner.getReal();
            break;
          default:
            scanner.warning("Failed to convert field");
        }
        break;            
      case dbField::tpString:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
          case dbXmlScanner::xml_sconst:
          case dbXmlScanner::xml_fconst:
          { 
              char* str = (char*)xmlContext->tmpAlloc.alloc((scanner.getStringLength()+1));
              memcpy(str, scanner.getString(), scanner.getStringLength());
              str[scanner.getStringLength()] = '\0';
              *(char**)dst = str;
              break;
          }
          default:
            scanner.warning("Failed to convert field");
        }
        break;            
      case dbField::tpWString:
        switch (scanner.scan()) { 
          case dbXmlScanner::xml_iconst:
          case dbXmlScanner::xml_sconst:
          case dbXmlScanner::xml_fconst:
          { 
              wchar_t* str = (wchar_t*)xmlContext->tmpAlloc.alloc((scanner.getStringLength()+1)*sizeof(wchar_t));
              size_t len = mbstowcs(str, scanner.getString(), scanner.getStringLength());
              str[len] = '\0';
              *(wchar_t**)dst = str;
              break;
          }
          default:
            scanner.warning("Failed to convert field");
        }
        break;            
      case dbField::tpRawBinary:
      {
          if (scanner.scan() != dbXmlScanner::xml_sconst) { 
              scanner.warning("Failed to convert field");
              break;            
          }
          char* src = scanner.getString();
          int len = (int)scanner.getStringLength() >> 1;
          if (fd->appSize != (size_t)len) { 
              scanner.warning("Length of raw binary field was changed");
          } else { 
              while (--len >= 0) { 
                  *dst++ = (HEX_DIGIT(src[0]) << 4) | HEX_DIGIT(src[1]);
                  src += 2;
              }
          }
          break;
      }
#ifdef USE_STD_STRING
      case dbField::tpStdString:
          if (scanner.scan() != dbXmlScanner::xml_sconst) { 
              scanner.warning("Failed to convert field");
              break;            
          }
#if STLPORT_WITH_VC6
          ((std::string*)dst)->assign(scanner.getString(), scanner.getStringLength());
#else
          new (dst) tmp_basic_string(scanner.getString(), scanner.getStringLength(), xmlContext->stdStringAlloc);
#endif
          break;
      case dbField::tpStdWString:
          if (scanner.scan() != dbXmlScanner::xml_sconst) { 
              scanner.warning("Failed to convert field");
          } else { 
              wchar_t* buf = new wchar_t[scanner.getStringLength()+1];
              size_t len = mbstowcs(buf, scanner.getString(), scanner.getStringLength());
              buf[len] = '\0';
#if STLPORT_WITH_VC6
              ((std::basic_string<wchar_t>*)dst)->assign(buf, len);
#else
              new (dst) tmp_basic_wstring(buf, len, xmlContext->stdWStringAlloc);
#endif
              delete[] buf;
          }
          break;
#endif
    }    
    return EXPECT(dbXmlScanner::xml_lts)
        && EXPECT(terminator)
        && EXPECT(dbXmlScanner::xml_gt); 
}

oid_t dbDatabase::mapId(long id)
{
    oid_t oid;
    if (id == 0) { 
        return 0;
    }
    if ((oid_t)id >= xmlContext->oidMapSize) { 
        oid_t* newOidMap = new oid_t[id*2];
        memcpy(newOidMap, xmlContext->oidMap, xmlContext->oidMapSize*sizeof(oid_t));
        memset(newOidMap + xmlContext->oidMapSize, 0, (id*2-xmlContext->oidMapSize)*sizeof(oid_t));
        xmlContext->oidMapSize = id*2;
        xmlContext->oidMap = newOidMap;
    }
    oid = xmlContext->oidMap[id];
    if (oid == 0) { 
        oid = allocateId();
        xmlContext->oidMap[id] = oid;
    }
    return oid;
}

void dbDatabase::insertRecord(dbTableDescriptor* desc, oid_t oid, void const* record) 
{
    dbFieldDescriptor* fd;
    byte* src = (byte*)record;
    size_t size = desc->columns->calculateRecordSize(src, desc->fixedSize);
    allocateRow(desc->tableId, oid, size);
    dbTable* table = (dbTable*)getRow(desc->tableId);
#ifdef AUTOINCREMENT_SUPPORT
    desc->autoincrementCount = table->count;
#endif
    dbRecord* dst = getRow(oid);
    desc->columns->storeRecordFields((byte*)dst, src, desc->fixedSize, dbFieldDescriptor::Import);
    
#ifdef AUTOINCREMENT_SUPPORT
    if ((nat4)desc->autoincrementCount > table->count) {
        table->count = desc->autoincrementCount;
    }
#endif
    size_t nRows = table->nRows;
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

bool dbDatabase::importDatabaseFromXml(char_t const* file)
{
    FILE* f = _tcscmp(file, _T("-")) == 0 ? stdin : _tfopen(file, _T("r"));
    if (f != NULL) { 
        bool rc = importDatabaseFromXml(f);
        if (f != stdin) { 
            fclose(f);
        }
        return rc;
    }
    return false;
}

bool dbDatabase::importDatabaseFromXml(FILE* in)
{
    dbXmlContext ctx;
    xmlContext = &ctx;
    dbXmlScanner scanner(in);
    dbXmlScanner::token tkn;
    
    if (!EXPECT(dbXmlScanner::xml_lt) ||
        !EXPECT("database") || 
        !EXPECT(dbXmlScanner::xml_gt))
    {
        return false;
    }
    beginTransaction(dbExclusiveLock);
    
    if (tables == NULL) { 
        loadMetaTable();
    }
    ctx.oidMapSize = dbDefaultInitIndexSize;
    ctx.oidMap = new oid_t[ctx.oidMapSize];
    memset(ctx.oidMap, 0, ctx.oidMapSize*sizeof(oid_t));

    while ((tkn = scanner.scan()) != dbXmlScanner::xml_lts) { 
        if (tkn != dbXmlScanner::xml_lt || !EXPECT(dbXmlScanner::xml_ident)) { 
            return false;
        }
        dbTableDescriptor* desc = findTableByName(scanner.getIdentifier());
        if (desc == NULL) { 
            fprintf(stderr, "Table '%s' not found\n", scanner.getIdentifier());
        }
        if (!EXPECT("id")
            || !EXPECT(dbXmlScanner::xml_eq)
            || !EXPECT(dbXmlScanner::xml_sconst)
            || !EXPECT(dbXmlScanner::xml_gt)) 
        {
            return false;
        }
        if (desc != NULL) { 
            long id;
            if (sscanf(scanner.getString(), "%ld", &id) != 1 || id == 0) { 
                return false;
            }
            oid_t oid = mapId(id);
            byte *record = (byte*)xmlContext->tmpAlloc.alloc(desc->appSize);  
            memset(record, 0, desc->appSize);
            if (!importRecord(desc->name, desc->columns, record, scanner)) {                 
                xmlContext->tmpAlloc.reset();
                return false;
            }
            insertRecord(desc, oid, record);
            xmlContext->tmpAlloc.reset();
        } else { // skip record
            if (!skipElement(scanner)) { 
                return false;
            }
        }    
    }
    return EXPECT("database") && EXPECT(dbXmlScanner::xml_gt);
}

END_FASTDB_NAMESPACE
