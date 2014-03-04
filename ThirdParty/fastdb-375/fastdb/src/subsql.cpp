//-< SUBSQL.CPP >----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Interactive data manipulation language (subset of SQL)
//-------------------------------------------------------------------*--------*

#include "fastdb.h"
#include "compiler.h"
#include "wwwapi.h"
#include "symtab.h"
#include "hashtab.h"
#include "ttree.h"
#include "rtree.h"
#include "subsql.h"
#include <locale.h>
#include <stdio.h>
#include <ctype.h>

#if THREADS_SUPPORTED && !defined(NO_SOCKETS)
#define SERVER_SUPPORTED 1
#include "server.h"
#else
#define SERVER_SUPPORTED 0
#endif

BEGIN_FASTDB_NAMESPACE

static char* typeMnem[] = {
        "Boolean",
        "Int1",
        "Int2",
        "Int4",
        "Int8",
        "Real4",
        "Real8",
        "String",
        "Reference",
        "Array",
        "MethodBool",
        "MethodInt1",
        "MethodInt2",
        "MethodInt4",
        "MethodInt8",
        "MethodReal4",
        "MethodReal8",
        "MethodString",
        "MethodReference",
        "Structure",
        "RawBinary",
        "StdString",
        "Rectangle",
        "WString",
        "StdWString",
        "MethodWString",
        "Unknown" 
};

char* dbSubSql::prompt = ">> ";


dbSubSql::dbSubSql(dbAccessType accessType)
: dbDatabase(accessType)
{
    static struct { 
        char* name;
        int   tag;
    } keywords[] = { 
        {"alter",   tkn_alter},
        {"array",   tkn_array},
        {"autocommit",   tkn_autocommit},
        {"autoincrement",tkn_autoincrement},
        {"backup",  tkn_backup},        
        {"bool",    tkn_bool},
        {"commit",  tkn_commit},
        {"compactify",tkn_compactify},
        {"count",   tkn_count},
        {"create",  tkn_create},
        {"delete",  tkn_delete},
        {"describe",tkn_describe},
        {"drop",    tkn_drop},
        {"exit",    tkn_exit},
        {"export",  tkn_export},
        {"hash",    tkn_hash},
        {"help",    tkn_help},
        {"http",    tkn_http},
        {"import",  tkn_import},
        {"index",   tkn_index},
        {"int1",    tkn_int1},
        {"int2",    tkn_int2},
        {"int4",    tkn_int4},
        {"int8",    tkn_int8},
        {"inverse", tkn_inverse},
        {"memory",  tkn_memory},
        {"of",      tkn_of},
        {"off",     tkn_off},
        {"on",      tkn_on},
        {"open",    tkn_open},
        {"profile", tkn_profile},        
        {"reference",tkn_reference},
        {"real4",   tkn_real4},
        {"real8",   tkn_real8},
        {"rectangle", tkn_rectangle},
        {"rename",  tkn_rename},
        {"rollback",tkn_rollback},
        {"server",  tkn_server},
        {"set",     tkn_set},
        {"stop",    tkn_stop},
        {"show",    tkn_show},
        {"to",      tkn_to},
        {"update",  tkn_update},
        {"values",  tkn_values},
        {"version", tkn_version}
    };
    for (unsigned i = 0; i < itemsof(keywords); i++) { 
        dbSymbolTable::add(keywords[i].name, keywords[i].tag, FASTDB_CLONE_ANY_IDENTIFIER);    
    }
    droppedTables = NULL;
    existedTables = NULL;
    opened = false;
    buflen = 1024;
    buf = new char[buflen];
    httpServerRunning = false;
    databaseName = NULL;
    historyUsed = historyCurr = 0;
    ungetToken = -1;
    dotIsPartOfIdentifier = false;
    autocommit = false;
    dateFormat = getenv("SUBSQL_DATE_FORMAT");
}

dbSubSql::~dbSubSql() 
{
    
    delete[] buf; 
}


//
// Find one string within another, ignoring case
//

inline char* stristr(const char* haystack, const char* needle)
{
    size_t i, hayLen, ndlLen;

    ndlLen = strlen(needle);
    hayLen = strlen(haystack);

    if (ndlLen > hayLen) {
        return NULL;
    }

    for (i = 0; i <= (hayLen - ndlLen); i++) {
        if (strnicmp(&haystack[i], needle, ndlLen) == 0) {
            return (char*)&haystack[i];
        }
    }
    return NULL;
}


bool contains(dbUserFunctionArgument& arg1, dbUserFunctionArgument& arg2) { 
    assert(arg1.type == dbUserFunctionArgument::atString && arg2.type == dbUserFunctionArgument::atString);
    return stristr(arg1.u.strValue, arg2.u.strValue) != NULL;
}

USER_FUNC(contains);

char* cmpOidAndStr(dbUserFunctionArgument& arg1, dbUserFunctionArgument& arg2) { 
    assert(arg1.type == dbUserFunctionArgument::atReference && arg2.type == dbUserFunctionArgument::atString);    
    printf("arg1.u.oidValue=%x, arg1.u.strValue=%s\n", arg1.u.oidValue, arg2.u.strValue);
    char* ptr = new char[16];
    sprintf(ptr, "%x", arg1.u.oidValue);
    return ptr;
}

USER_FUNC(cmpOidAndStr);

int dbSubSql::get() 
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

void dbSubSql::unget(int ch) { 
    if (ch != EOF) { 
        if (ch != '\n') { 
            pos -= 1;
        } else { 
            line -= 1;
        }
        ungetc(ch, in);
    }
}

void dbSubSql::error(char const* msg)
{
#ifdef THROW_EXCEPTION_ON_ERROR
   dbDatabaseThreadContext* ctx = threadContext.get();
    if (ctx != NULL) {
        ctx->interactive = true;
    }
    try { 
        handleError(QueryError, msg, tknPos > 0 ? tknPos - 1 : 0);
    } catch(dbException) {}
#else
    dbDatabaseThreadContext* ctx = threadContext.get();
    if (ctx != NULL) { 
        ctx->interactive = true;
        ctx->catched = true;
        if (setjmp(ctx->unwind) == 0) { 
            handleError(QueryError, msg, tknPos > 0 ? tknPos - 1 : 0);
        }
        ctx->catched = false;
    } else { 
        handleError(QueryError, msg, tknPos > 0 ? tknPos - 1 : 0);
    }
#endif
}


int dbSubSql::scan() 
{
    int i, ch, digits;
    
    if (ungetToken >= 0) { 
        int tkn = ungetToken;
        ungetToken = -1;
        return tkn;
    }
    bool dotIsIdentifierChar = dotIsPartOfIdentifier;
    dotIsPartOfIdentifier = false;
  nextToken:
    do { 
        if ((ch = get()) == EOF) { 
            return tkn_eof;
        }
    } while (isspace(ch));
    
    tknPos = pos;
    switch (ch) { 
      case '*':
        return tkn_all;
      case '(':
        return tkn_lpar;
      case ')':
        return tkn_rpar;
      case ',':
        return tkn_comma;
      case '.':
        return tkn_dot;
      case ';':
        return tkn_semi;
      case '=':
        return tkn_eq;
      case '\'':
        i = 0; 
        while (true) { 
            ch = get();
            if (ch == '\'') { 
                if ((ch = get()) != '\'') { 
                    unget(ch);
                    break;
                }
            } else if (ch == '\n' || ch == EOF) { 
                unget(ch);
                error("New line within character constant");
                return tkn_error;
            }
            if (i+1 == buflen) { 
                char* newbuf = new char[buflen*2];
                memcpy(newbuf, buf, buflen);
                delete[] buf;
                buf = newbuf;
                buflen *= 2;
            }
            buf[i++] = ch;
        }
        buf[i] = '\0';
        return tkn_sconst;
      case '-':
        if ((ch = get()) == '-') { 
            // ANSI comments
            while ((ch = get()) != EOF && ch != '\n');
            goto nextToken;
        }
        unget(ch);
        ch = '-';
        // no break
      case '0': case '1': case '2': case '3': case '4': 
      case '5': case '6': case '7': case '8': case '9':
      case '+':
        i = 0;
        do { 
            buf[i++] = ch;
            if (i == buflen) { 
                error("Numeric constant too long");
                return tkn_error;
            }
            ch = get();
        } while (ch != EOF 
                 && (isdigit(ch) || ch == '+' || ch == '-' || ch == 'e' || 
                     ch == 'E' || ch == '.'));
        unget(ch);
        buf[i] = '\0';
        if (sscanf(buf, INT8_FORMAT "%n", &ival, &digits) != 1) { 
            error("Bad integer constant");
            return tkn_error;
        }
        if (digits != i) { 
            if (sscanf(buf, "%lf%n", &fval, &digits) != 1 || digits != i) {
                error("Bad float constant");
                return tkn_error;
            }
            return tkn_fconst;
        } 
        return tkn_iconst;

      case '#':
        ival = 0;
        while (true) { 
            ch = get();
            if (ch >= '0' && ch <= '9') { 
                ival = (ival << 4) + ch-'0';
            } else if (ch >= 'a' && ch <= 'f') {
                ival = (ival << 4) + ch-'a'+10;
            } else if (ch >= 'A' && ch <= 'F') {
                ival = (ival << 4) + ch-'A'+10;
            } else { 
                unget(ch);
                return tkn_iconst;
            }
        }

      default:
        if (isalpha(ch) || ch == '$' || ch == '_') { 
            i = 0;
            do { 
                buf[i++] = ch;
                if (i == buflen) { 
                    error("Identifier too long");
                    return tkn_error;
                }
                ch = get();
            } while (ch != EOF && (isalnum(ch) || ch == '$' || ch == '_' || (ch == '.' && dotIsIdentifierChar)));
            unget(ch);
            buf[i] = '\0';
            name = buf;
            return dbSymbolTable::add(name, tkn_ident);
        } else { 
            error("Invalid symbol");
            return tkn_error;
        }
    }
}


bool dbSubSql::expect(char* expected, int token)
{
    int tkn = scan();
    if (tkn != token) { 
        if (tkn != tkn_error) { 
            char buf[256];
            sprintf(buf, "Token '%s' expected", expected);
            error(buf);
        }
        return false;
    }
    return true;
}

    
bool dbSubSql::updateTable(bool create)
{
    int tkn;
    dotIsPartOfIdentifier = true;
    if (!expect("table name", tkn_ident) || !expect("(", tkn_lpar)) { 
        return false;
    }
    char* name = this->name;
    int varyingLength = (int)strlen(name)+1;

    static const struct { 
        int size;
        int alignment;
    } typeDesc[] = { 
        { sizeof(bool), sizeof(bool) }, 
        { sizeof(int1), sizeof(int1) }, 
        { sizeof(int2), sizeof(int2) }, 
        { sizeof(int4), sizeof(int4) }, 
        { sizeof(db_int8), sizeof(db_int8) }, 
        { sizeof(real4), sizeof(real4) }, 
        { sizeof(real8), sizeof(real8) }, 
        { sizeof(dbVarying), 4 }, 
        { sizeof(oid_t), sizeof(oid_t) }, 
        { sizeof(dbVarying), 4 },
        {0}, // tpMethodBool,
        {0}, // tpMethodInt1,
        {0}, // tpMethodInt2,
        {0}, // tpMethodInt4,
        {0}, // tpMethodInt8,
        {0}, // tpMethodReal4,
        {0}, // tpMethodReal8,
        {0}, // tpMethodString,
        {0}, // tpMethodReference,
        {0}, // tpStructure,
        {0}, // tpRawBinary,
        {0}, // tpStdString,
        { sizeof(rectangle), sizeof(coord_t) }, // tpRectangle,
        { sizeof(dbVarying), 4 },  // tpWString,
        {0}, // tpStdWString,
        {0}, // tpMethodWString,
        {0} // tpUnknown
    };

    const int maxFields = 256;
    tableField fields[maxFields];
    int nFields = 0;
    int nColumns = 0;
    tkn = tkn_comma;
    while (tkn == tkn_comma) { 
        if (nFields+1 == maxFields) { 
            error("Too many fields");
            break;
        }
        if (!expect("field name", tkn_ident)) { 
            break;
        }
        int nameLen = (int)strlen(buf)+1;
        fields[nFields].name = new char[nameLen];
        strcpy(fields[nFields].name, buf);
        varyingLength += nameLen + 2;
        char* refTableName;
        char* inverseRefName;
        int type = parseType(refTableName, inverseRefName);
        fields[nFields++].type = type;
        if (type == dbField::tpUnknown) { 
            break;
        }
        nColumns += 1;
        if (type == dbField::tpArray) {
            if (nFields+1 == maxFields) { 
                error("Too many fields");
                break;
            }
            fields[nFields].name = new char[nameLen+2];
            sprintf(fields[nFields].name, "%s[]", fields[nFields-1].name);
            varyingLength += nameLen+2+2;
            type = parseType(refTableName, inverseRefName);
            if (type == dbField::tpUnknown) { 
                break;
            }
            if (type == dbField::tpArray) { 
                error("Arrays of arrays are not supported by CLI");
                break;
            }
            if (type == dbField::tpReference) {
                fields[nFields].refTableName = refTableName;
                varyingLength += (int)strlen(refTableName);
                if (inverseRefName != NULL) { 
                    fields[nFields-1].inverseRefName = inverseRefName;
                    varyingLength += (int)strlen(inverseRefName);
                }                   
            }
            fields[nFields++].type = type;
        } else if (type == dbField::tpReference) { 
            fields[nFields-1].refTableName = refTableName;
            varyingLength += (int)strlen(refTableName);
            if (inverseRefName != NULL) { 
                fields[nFields-1].inverseRefName = inverseRefName;
                varyingLength += (int)strlen(inverseRefName);
            }                   
        }
        tkn = scan(); 
    }
    if (tkn == tkn_rpar) { 
        beginTransaction(dbExclusiveLock);
        dbTableDescriptor* oldDesc = findTable(name);
        if (oldDesc != NULL) {
            if (create) { 
                error("Table already exists");
                return false;
            }
        } else { 
            if (!create) { 
                error("Table not found");
                return false;
            }
        }
        dbTable* table; 
        oid_t oid;
        if (create) { 
            modified = true;
            oid = allocateRow(dbMetaTableId, 
                              sizeof(dbTable) + sizeof(dbField)*nFields + varyingLength);
            table = (dbTable*)getRow(oid);    
        } else { 
            oid = oldDesc->tableId;
            table = (dbTable*)new char[sizeof(dbTable) + sizeof(dbField)*nFields + varyingLength];
        }
        int offs = sizeof(dbTable) + sizeof(dbField)*nFields;
        table->name.offs = offs;
        table->name.size = (nat4)strlen(name)+1;
        strcpy((char*)table + offs, name);
        offs += table->name.size;
        size_t size = sizeof(dbRecord);
        table->fields.offs = sizeof(dbTable);
        dbField* field = (dbField*)((char*)table + table->fields.offs);
        offs -= sizeof(dbTable);
        bool arrayComponent = false;

        for (int i = 0; i < nFields; i++) { 
            field->name.offs = offs;
            field->name.size = (nat4)strlen(fields[i].name) + 1;
            strcpy((char*)field + offs, fields[i].name);
            offs += field->name.size;
            
            field->tableName.offs = offs;
            if (fields[i].refTableName) { 
                field->tableName.size = (nat4)strlen(fields[i].refTableName) + 1;
                strcpy((char*)field + offs, fields[i].refTableName);
                offs += field->tableName.size;
            } else { 
                field->tableName.size = 1;
                *((char*)field + offs++) = '\0';
            }

            field->inverse.offs = offs;
            if (fields[i].inverseRefName) { 
                field->inverse.size = (nat4)strlen(fields[i].inverseRefName) + 1;
                strcpy((char*)field + offs, fields[i].inverseRefName);
                offs += field->inverse.size;
            } else { 
                field->inverse.size = 1;
                *((char*)field + offs++) = '\0';
            }
    
            field->flags = 0;
            field->type = fields[i].type;
            field->size = typeDesc[fields[i].type].size;
            if (!arrayComponent) { 
                size = DOALIGN(size, typeDesc[fields[i].type].alignment);
                field->offset = (int)size;
                size += field->size;
            } else { 
                field->offset = 0;
            }
            field->hashTable = 0;
            field->tTree = 0;
            arrayComponent = field->type == dbField::tpArray; 
            field += 1;
            offs -= sizeof(dbField);
        }
        table->fields.size = nFields;
        table->fixedSize = (nat4)size;
        table->nRows = 0;
        table->nColumns = nColumns;
        table->firstRow = 0;
        table->lastRow = 0;

        if (create) { 
            linkTable(new dbTableDescriptor(this, table), oid);
        } else { 
            dbTableDescriptor* newDesc = new dbTableDescriptor(this, table);      
            delete[] (char*)table;
            dbTable* oldTable = (dbTable*)getRow(oid);
            if (!newDesc->equal(oldTable)) {
                bool saveConfirmDeleteColumns = confirmDeleteColumns; 
                confirmDeleteColumns = true;
                modified = true;
                schemeVersion += 1;
                unlinkTable(oldDesc);
                if (oldTable->nRows == 0) {
                    updateTableDescriptor(newDesc, oid);
                } else {
                    reformatTable(oid, newDesc);
                }
                delete oldDesc;
                confirmDeleteColumns = saveConfirmDeleteColumns;
                addIndices(true, newDesc);
            }
        }
        completeDescriptorsInitialization();
    }
    return tkn == tkn_rpar;
}

int dbSubSql::parseType(char*& refTableName, char*& inverseFieldName)
{
    switch (scan()) { 
      case tkn_bool:
        return dbField::tpBool;
      case tkn_int1:
        return dbField::tpInt1;
      case tkn_int2:
        return dbField::tpInt2;
      case tkn_int4:
        return dbField::tpInt4;
      case tkn_int8:
        return dbField::tpInt8;
      case tkn_real4:
        return dbField::tpReal4;
      case tkn_real8:
        return dbField::tpReal8;
      case tkn_array:
        return expect("of", tkn_of) ? dbField::tpArray : dbField::tpUnknown;
      case tkn_string:
        return dbField::tpString;
      case tkn_wstring:
        return dbField::tpWString;
      case tkn_reference:
        if (expect("to", tkn_to) && expect("referenced table name", tkn_ident)) {
            refTableName = new char[strlen(buf)+1];
            strcpy(refTableName, buf);
            int tkn = scan();
            if (tkn == tkn_inverse) {
                if (!expect("inverse reference field name", tkn_ident)) { 
                    return dbField::tpUnknown;
                }
                inverseFieldName = new char[strlen(buf)+1];
                strcpy(inverseFieldName, buf);
            } else { 
                inverseFieldName = NULL;
                ungetToken = tkn;
            }
            return dbField::tpReference;
        } else { 
            return dbField::tpUnknown;
        }
      case tkn_rectangle:
        return dbField::tpRectangle;
      default:
        error("Field type expected");
    }
    return dbField::tpUnknown;
}

int dbSubSql::readExpression()
{
    int i, ch;
    int nesting = 0;
    for (i = 0; (ch = get()) != ';' && (ch != ',' || nesting != 0)  && ch != EOF; i++) { 
        if (ch == '(') { 
            nesting += 1;
        } else if (ch == ')') { 
            nesting -= 1;
        }
        if (i+1 >= buflen) { 
            char* newbuf = new char[buflen*2];
            memcpy(newbuf, buf, buflen);
            delete[] buf;
            buf = newbuf;
            buflen *= 2;
        }
        buf[i] = ch;
    }
    buf[i] = '\0';
    return ch;
}

bool dbSubSql::readCondition()
{
    int i, ch;
    for (i = 0; (ch = get()) != ';' && ch !=  EOF; i++) { 
        if (i+1 >= buflen) { 
            char* newbuf = new char[buflen*2];
            memcpy(newbuf, buf, buflen);
            delete[] buf;
            buf = newbuf;
            buflen *= 2;
        }
        buf[i] = ch;
    }
    buf[i] = '\0';
    if (ch != ';') { 
        error("unexpected end of input");
        return false;
    } 
    return true;
}


void dbSubSql::dumpRecord(byte* base, dbFieldDescriptor* first)
{
    int i, n;
    byte* elem;
    dbFieldDescriptor* fd = first;
    do { 
        if (fd != first) { 
            printf(", ");
        }
        switch (fd->type) { 
          case dbField::tpBool:
            printf("%s", *(bool*)(base + fd->dbsOffs) 
                   ? "true" : "false");
            continue;
          case dbField::tpInt1:
            printf("%d", *(int1*)(base + fd->dbsOffs)); 
            continue;       
          case dbField::tpInt2:
            printf("%d", *(int2*)(base + fd->dbsOffs)); 
            continue;
          case dbField::tpInt4:
            printf("%d", *(int4*)(base + fd->dbsOffs)); 
            continue;
          case dbField::tpInt8:
            printf(INT8_FORMAT, *(db_int8*)(base + fd->dbsOffs)); 
            continue;
          case dbField::tpReal4:
            printf("%f", *(real4*)(base + fd->dbsOffs)); 
            continue;
          case dbField::tpReal8:
            printf("%f", *(real8*)(base + fd->dbsOffs)); 
            continue;
          case dbField::tpString:
            printf("'%s'", (char*)base+((dbVarying*)(base+fd->dbsOffs))->offs);
            continue;
          case dbField::tpWString:
            printf("'%ls'", (wchar_t*)((char*)base+((dbVarying*)(base+fd->dbsOffs))->offs));
            continue;
          case dbField::tpReference:
            printf("#%lx", (unsigned long)*(oid_t*)(base + fd->dbsOffs)); 
            continue;
          case dbField::tpRectangle:
            {
                int i, sep = '(';
                rectangle& r = *(rectangle*)(base + fd->dbsOffs);
                for (i = 0; i < rectangle::dim*2; i++) { 
                    printf("%c%f", sep, (double)r.boundary[i]);
                    sep = ',';
                }
                printf(")");
            }
            continue;
          case dbField::tpRawBinary:
            n = (int)fd->dbsSize;
            elem = base + fd->dbsOffs;
            printf("(");
            for (i = 0; i < n; i++) { 
                if (i != 0) { 
                    printf(", ");
                }
                printf("%02x", *elem++);
            }
            printf(")");
            continue;
          case dbField::tpArray:
            n = ((dbVarying*)(base + fd->dbsOffs))->size;
            elem = base + ((dbVarying*)(base + fd->dbsOffs))->offs;
            printf("(");
            for (i = 0; i < n; i++) { 
                if (i != 0) { 
                    printf(", ");
                }
                dumpRecord(elem, fd->components);
                elem += fd->components->dbsSize;
            }
            printf(")");
            continue;
          case dbField::tpStructure:
            if (dateFormat != NULL 
                && fd->components->next == fd->components 
                && strcmp(fd->components->name, "stamp") == 0) 
            { 
                char buf[64];
                printf(((dbDateTime*)(base + fd->components->dbsOffs))->asString(buf, sizeof buf, dateFormat));
                continue;
            }
            printf("(");
            dumpRecord(base, fd->components);
            printf(")");
        }
    } while ((fd = fd->next) != first);
}

int dbSubSql::calculateRecordSize(dbList* node, int offs, 
                                  dbFieldDescriptor* first)
{
    dbFieldDescriptor* fd = first;
    do { 
        if (node == NULL) { 
            return -1;
        }
        if (fd->type == dbField::tpArray) { 
            if (node->type != dbList::nTuple) { 
                return -1;
            }
            int nElems = node->aggregate.nComponents;
            offs = (int)(DOALIGN(offs, fd->components->alignment) 
                 + nElems*fd->components->dbsSize);
            if (fd->attr & dbFieldDescriptor::HasArrayComponents) {
                dbList* component = node->aggregate.components;
                while (--nElems >= 0) { 
                    int d = calculateRecordSize(component,offs,fd->components);
                    if (d < 0) return d;
                    offs = d;
                    component = component->next;
                }
            } 
        } else if (fd->type == dbField::tpString) { 
            if (node->type != dbList::nString) { 
                return -1;
            }
            offs += (int)strlen(node->sval) + 1;
        } else if (fd->type == dbField::tpRectangle) {
            if (node->type != dbList::nTuple) { 
                return -1;
            }
            int nCoords = node->aggregate.nComponents;
            if (nCoords != rectangle::dim*2) {
                return -1;
            }
            dbList* component = node->aggregate.components;
            while (--nCoords >= 0) {
                if (component->type != dbList::nInteger && component->type != dbList::nReal) {
                    return -1;
                }
                component = component->next;
            }
        } else if (fd->type == dbField::tpRawBinary) { 
            if (node->type != dbList::nTuple) { 
                return -1;
            }
            int nElems = node->aggregate.nComponents;
            dbList* component = node->aggregate.components;
            if (size_t(nElems) > fd->dbsSize) { 
                return -1;
            }
            while (--nElems >= 0) { 
                if (component->type != dbList::nInteger
                    || (component->ival & ~0xFF) != 0) 
                { 
                    return -1;
                }
                component = component->next;
            }
#ifdef AUTOINCREMENT_SUPPORT
        } else if (node->type == dbList::nAutoinc) {        
            if (fd->type != dbField::tpInt4) {
                return -1;
            }
#endif
        } else { 
            if (!((node->type == dbList::nBool && fd->type == dbField::tpBool)
                  || (node->type == dbList::nInteger 
                      && (fd->type == dbField::tpInt1
                          || fd->type == dbField::tpInt2
                          || fd->type == dbField::tpInt4
                          || fd->type == dbField::tpInt8
                          || fd->type == dbField::tpReference))
                  || (node->type == dbList::nReal 
                      && (fd->type == dbField::tpReal4
                          || fd->type == dbField::tpReal8))
                  || (node->type == dbList::nTuple 
                      && fd->type == dbField::tpStructure)))
            {
                return -1;
            }
            if (fd->attr & dbFieldDescriptor::HasArrayComponents) {
                int d = calculateRecordSize(node->aggregate.components,
                                            offs, fd->components);
                if (d < 0) return d;
                offs = d;
            }
        }
        node = node->next;
    } while ((fd = fd->next) != first);
    return offs;    
}

bool dbSubSql::isValidOid(oid_t oid) 
{
    return oid == 0 || 
        (oid < currIndexSize && (currIndex[oid] & (dbFreeHandleMarker|dbInternalObjectMarker)) == 0);
}

int dbSubSql::initializeRecordFields(dbList* node, byte* dst, int offs, 
                                     dbFieldDescriptor* first)
{
    dbFieldDescriptor* fd = first;
    dbList* component;
    byte* elem;
    coord_t* coord;
    int len, elemOffs, elemSize;

    do { 
        if (node->type == dbList::nString && fd->type != dbField::tpString) { 
            char* s = node->sval;
            long  ival;
            switch (fd->type) {
              case dbField::tpBool:
                *(bool*)(dst+fd->dbsOffs) = *s == '1' || *s == 't' || *s == 'T';
                break;
              case dbField::tpInt1:
                if (sscanf(s, "%ld", &ival) != 1) { 
                    return -1;
                }
                *(int1*)(dst+fd->dbsOffs) = (int1)ival;
              case dbField::tpInt2:
                if (sscanf(s, "%ld", &ival) != 1) { 
                    return -1;
                }
                *(int2*)(dst+fd->dbsOffs) = (int2)ival;
              case dbField::tpInt4:
                if (sscanf(s, "%ld", &ival) != 1) { 
                    return -1;
                }
                *(int4*)(dst+fd->dbsOffs) = (int4)ival;
              case dbField::tpInt8:
                if (sscanf(s, "%ld", &ival) != 1) { 
                    return -1;
                }
                *(db_int8*)(dst+fd->dbsOffs) = ival;
                break;
              case dbField::tpReal4:
                if (sscanf(s, "%f", (real4*)(dst+fd->dbsOffs)) != 1) { 
                    return -1;
                }
                break;
              case dbField::tpReal8:
                if (sscanf(s, "%lf", (real8*)(dst+fd->dbsOffs)) != 1) { 
                    return -1;
                }
                break;
              case dbField::tpWString:
                ((dbVarying*)(dst+fd->dbsOffs))->offs = offs;
                len = mbstowcs(NULL, node->sval, 0);
                ((dbVarying*)(dst+fd->dbsOffs))->size = len+1;
                mbstowcs((wchar_t*)(dst + offs), node->sval, len);
                *((wchar_t*)(dst + offs) + len) = '\0';
                offs += len*sizeof(wchar_t);
                break;
            }
#ifdef AUTOINCREMENT_SUPPORT
        } else if (node->type == dbList::nAutoinc) {        
            if (fd->type == dbField::tpInt4) {
                *(int4*)(dst+fd->dbsOffs) = fd->defTable->autoincrementCount;
            } else { 
                return -1;
            }
#endif
        } else { 
            switch (fd->type) { 
              case dbField::tpBool:
                *(bool*)(dst+fd->dbsOffs) = node->bval;
                break;
              case dbField::tpInt1:
                *(int1*)(dst+fd->dbsOffs) = (int1)node->ival;
                break;
              case dbField::tpInt2:
                *(int2*)(dst+fd->dbsOffs) = (int2)node->ival;
                break;
              case dbField::tpInt4:
                *(int4*)(dst+fd->dbsOffs) = (int4)node->ival;
                break;
              case dbField::tpInt8:
                *(db_int8*)(dst+fd->dbsOffs) = node->ival;
                break;
              case dbField::tpReal4:
                *(real4*)(dst+fd->dbsOffs) = (real4)node->fval;
                break;
              case dbField::tpReal8:
                *(real8*)(dst+fd->dbsOffs) = node->fval;
                break;
              case dbField::tpReference:
                if (isValidOid((oid_t)node->ival)) {                
                    *(oid_t*)(dst+fd->dbsOffs) = (oid_t)node->ival;
                } else { 
                    return -1;
                }
                break;
              case dbField::tpString:
                ((dbVarying*)(dst+fd->dbsOffs))->offs = offs;
                len = (int)strlen(node->sval) + 1;
                ((dbVarying*)(dst+fd->dbsOffs))->size = len;
                memcpy(dst + offs, node->sval, len);
                offs += len;
                break;
              case dbField::tpRawBinary:
                len = node->aggregate.nComponents;
                component = node->aggregate.components;
                elem = dst + fd->dbsOffs;
                while (--len >= 0) { 
                    *elem++ = (byte)component->ival;
                    component = component->next;
                }
                break;
              case dbField::tpRectangle:
                len = node->aggregate.nComponents;
                component = node->aggregate.components;
                coord = (coord_t*)(dst + fd->dbsOffs);
                assert(len == rectangle::dim*2);                    
                while (--len >= 0) {
                    *coord++ = (component->type == dbList::nInteger) 
                        ? (coord_t)component->ival : (coord_t)component->fval;
                    component = component->next;
                }
                break;
              case dbField::tpArray:
                len = node->aggregate.nComponents;
                elem = (byte*)DOALIGN(size_t(dst) + offs, fd->components->alignment);
                offs = (int)(elem - dst);
                ((dbVarying*)(dst+fd->dbsOffs))->offs = offs;
                ((dbVarying*)(dst+fd->dbsOffs))->size = len;
                elemSize = (int)fd->components->dbsSize;  
                elemOffs = len*elemSize;
                offs += elemOffs;
                component = node->aggregate.components;
                while (--len >= 0) { 
                    elemOffs = initializeRecordFields(component, elem, elemOffs, 
                                                      fd->components);
                    elemOffs -= elemSize;
                    elem += elemSize;
                    component = component->next;
                }
                offs += elemOffs;
                break;
              case dbField::tpStructure:
                offs = initializeRecordFields(node->aggregate.components, 
                                              dst, offs, fd->components);
            }
        }
        node = node->next;
    } while ((fd = fd->next) != first);

    return offs;
}


bool dbSubSql::insertRecord(dbList* list, dbTableDescriptor* desc)
{
    int size = calculateRecordSize(list, (int)desc->fixedSize, desc->columns);
    if (size < 0) { 
        error("Incompatible types in insert statement");
        return false;
    }
    oid_t oid = allocateRow(desc->tableId, size);
    byte* dst = (byte*)getRow(oid);    
    dbTable* table = (dbTable*)getRow(desc->tableId);
#ifdef AUTOINCREMENT_SUPPORT
    desc->autoincrementCount = table->count;
#endif
    initializeRecordFields(list, dst, (int)desc->fixedSize, desc->columns);

    int nRows = table->nRows;
    dbFieldDescriptor* fd;
    for (fd = desc->hashedFields; fd != NULL; fd = fd->nextHashedField){
        dbHashTable::insert(this, fd, oid, nRows);
    }
    for (fd = desc->indexedFields; fd != NULL; fd = fd->nextIndexedField) { 
        if (fd->type == dbField::tpRectangle) { 
            dbRtree::insert(this, fd->tTree, oid, fd->dbsOffs);
        } else { 
            dbTtree::insert(this, fd->tTree, oid, fd->type, (int)fd->dbsSize, fd->_comparator, fd->dbsOffs);
        }
    }    
    return true;
}

int dbSubSql::readValues(dbList** chain)
{
    int i, n = 0;
    int tkn;
    dbList* node;

    while (true) { 
        switch (scan()) { 
          case tkn_lpar:
            node = new dbList(dbList::nTuple);
            node->aggregate.components = NULL;
            i = readValues(&node->aggregate.components);
            if (i < 0) { 
                return -1;
            }
            node->aggregate.nComponents = i;
            break;
          case tkn_rpar:
            return -n; // valid only in case of empty list
          case tkn_iconst:
            node = new dbList(dbList::nInteger);
            node->ival = ival;
            break;
          case tkn_true:
            node = new dbList(dbList::nBool);
            node->bval = true;
            break;
          case tkn_false:
            node = new dbList(dbList::nBool);
            node->bval = false;
            break;          
          case tkn_fconst:
            node = new dbList(dbList::nReal);
            node->fval = fval;
            break;
          case tkn_sconst:
            node = new dbList(dbList::nString);
            node->sval = new char[strlen(buf)+1];
            strcpy(node->sval, buf);
            break;
          case tkn_autoincrement:
            node = new dbList(dbList::nAutoinc);
            break;
          case tkn_error:
            return -1;
          default:
            error("Syntax error in insert list");
            return -1;
        }
        *chain = node;
        chain = &node->next;
        n += 1;
        if ((tkn = scan()) == tkn_rpar) { 
            return n;
        }
        if (tkn != tkn_comma) {   
            error("',' expected");
            return -1;
        }
    }
}


dbFieldDescriptor* dbSubSql::readFieldName(int terminator)
{
    int tkn;

    if (expect("table name", tkn_ident)) { 
        dbTableDescriptor* desc;
        dbFieldDescriptor* fd;
        if ((desc = findTable(name)) == NULL) { 
            error("No such table in database");
            return NULL;
        }       
        if (expect(".", tkn_dot) && expect("field name", tkn_ident)) { 
            if ((fd = desc->findSymbol(name)) == NULL) { 
                error("No such field in the table");
                return NULL;
            } else if (fd->type == dbField::tpArray) {  
                error("Array components can not be indexed");
                return NULL;
            }
        } else { 
            return NULL;
        }
        while ((tkn = scan()) != terminator) { 
            if (tkn != tkn_dot) { 
                error("'.' expected");
                return NULL;
            }   
            if (expect("field name", tkn_ident)) { 
                if ((fd = fd->findSymbol(name)) == NULL) { 
                    error("No such field in the table");
                    return NULL;
                } else if (fd->type == dbField::tpArray) {      
                    error("Array components can not be indexed");
                    return NULL;
                }       
            } else { 
                return NULL;
            }
        }
        if (fd->type == dbField::tpStructure) { 
            error("Structures can not be indexed");
            return NULL;
        } 
        return fd;
    }
    return NULL;
}

inline int getListSize(dbExprNode* expr) 
{ 
    int nElems;
    if (expr->type == tpList) { 
        nElems = 0;
        if (expr->operand[0] != NULL) {
            do { 
                nElems += 1;
            } while ((expr = expr->operand[1]) != NULL);
        } 
    } else { 
        nElems = 1;
    }
    return nElems;
}

bool dbSubSql::updateFields(dbAnyCursor* cursor, dbUpdateElement* elems)
{
     char buf[64], *src;
     dbInheritedAttribute iattr;
     dbSynthesizedAttribute sattr;
     iattr.db = this;
     iattr.oid = cursor->currId;
     iattr.table = (dbTable*)getRow(cursor->table->tableId);
     iattr.record = (byte*)getRow(cursor->currId);    
     iattr.paramBase = (size_t)cursor->paramBase;

     do { 
         dbExprNode* expr = elems->value;
         dbFieldDescriptor* fd = elems->field;
         execute(expr, iattr, sattr);
         byte* dst = cursor->record + fd->appOffs;

         switch (fd->type) {
           case dbField::tpArray:
           {
               int nElems = getListSize(expr);
               if (nElems != 0) 
               {
                   switch (fd->components->type) {    
                     case dbField::tpBool:
                     {
                         bool* arr = new bool[nElems];
                         switch (expr->type) { 
                           case tpList:
                           {
                               int i = 0;
                               do { 
                                   dbExprNode* elem = expr->operand[0];
                                   execute(elem, iattr, sattr);
                                   switch (elem->type) { 
                                     case tpInteger:
                                       arr[i++] = sattr.ivalue != 0;
                                       continue;
                                     case tpBoolean:
                                       arr[i++] = sattr.bvalue;
                                       continue;
                                     case tpReal:
                                       arr[i++] = sattr.fvalue != 0;
                                       continue;
                                     case tpString:
                                       arr[i++] = *sattr.base == 'T' || *sattr.base == 't' || *sattr.base == '1';
                                       continue;
                                     default:
                                       return false;
                                   }                                 
                               } while ((expr = expr->operand[1]) != NULL);
                               break;
                           } 
                           case tpInteger:
                             arr[0] = sattr.ivalue != 0;
                             break;
                           case tpBoolean:
                             arr[0] = sattr.bvalue;
                             break;
                           case tpReal:
                             arr[0] = sattr.fvalue != 0;
                             break;
                           case tpString:
                             arr[0] = *sattr.base == 'T' || *sattr.base == 't' || *sattr.base == '1';
                             break;
                           default:
                             error("Invalid array element type");
                             return false;
                         }
                         ((dbArray<bool>*)dst)->assign(arr, nElems, false);
                         elems->strValue = (char*)arr;
                         continue;
                     }
                     case dbField::tpInt1:
                     {
                         int1* arr = new int1[nElems];
                         switch (expr->type) { 
                           case tpList:
                           {
                               int i = 0;
                               do { 
                                   dbExprNode* elem = expr->operand[0];
                                   execute(elem, iattr, sattr);
                                   switch (elem->type) { 
                                     case tpInteger:
                                       arr[i++] = (int1)sattr.ivalue;
                                       continue;
                                     case tpBoolean:
                                       arr[i++] = sattr.bvalue ? 1 : 0;
                                       continue;
                                     case tpReal:
                                       arr[i++] = (int1)sattr.fvalue;
                                       continue;
                                     case tpString:
                                       arr[i++] = (int1)atoi((char*)sattr.base);
                                       continue;
                                     default:
                                       return false;
                                   }                                 
                               } while ((expr = expr->operand[1]) != NULL);
                               break;
                           } 
                           case tpInteger:
                             arr[0] = (int1)sattr.ivalue;
                             break;
                           case tpBoolean:
                             arr[0] = sattr.bvalue ? 1 : 0;
                             break;
                           case tpReal:
                             arr[0] = (int1)sattr.fvalue;
                             break;
                           case tpString:
                             arr[0] = (int1)atoi((char*)sattr.base);
                             break;
                           default:
                             error("Invalid array element type");
                             return false;
                         }
                         ((dbArray<int1>*)dst)->assign(arr, nElems, false);
                         elems->strValue = (char*)arr;
                         continue;
                     }
                     case dbField::tpInt2:
                     {
                         int2* arr = new int2[nElems];
                         switch (expr->type) { 
                           case tpList:
                           {
                               int i = 0;
                               do { 
                                   dbExprNode* elem = expr->operand[0];
                                   execute(elem, iattr, sattr);
                                   switch (elem->type) { 
                                     case tpInteger:
                                       arr[i++] = (int2)sattr.ivalue;
                                       continue;
                                     case tpBoolean:
                                       arr[i++] = sattr.bvalue ? 1 : 0;
                                       continue;
                                     case tpReal:
                                       arr[i++] = (int2)sattr.fvalue;
                                       continue;
                                     case tpString:
                                       arr[i++] = (int2)atoi((char*)sattr.base);
                                       continue;
                                     default:
                                       return false;
                                   }                                 
                               } while ((expr = expr->operand[1]) != NULL);
                               break;
                           } 
                           case tpInteger:
                             arr[0] = (int2)sattr.ivalue;
                             break;
                           case tpBoolean:
                             arr[0] = sattr.bvalue ? 1 : 0;
                             break;
                           case tpReal:
                             arr[0] = (int2)sattr.fvalue;
                             break;
                           case tpString:
                             arr[0] = (int2)atoi((char*)sattr.base);
                             break;
                           default:
                             error("Invalid array element type");
                             return false;
                         }
                         ((dbArray<int2>*)dst)->assign(arr, nElems, false);
                         elems->strValue = (char*)arr;
                         continue;
                     }
                     case dbField::tpInt4:
                     {
                         int4* arr = new int4[nElems];
                         switch (expr->type) { 
                           case tpList:
                           {
                               int i = 0;
                               do { 
                                   dbExprNode* elem = expr->operand[0];
                                   execute(elem, iattr, sattr);
                                   switch (elem->type) { 
                                     case tpInteger:
                                       arr[i++] = (int4)sattr.ivalue;
                                       continue;
                                     case tpBoolean:
                                       arr[i++] = sattr.bvalue ? 1 : 0;
                                       continue;
                                     case tpReal:
                                       arr[i++] = (int4)sattr.fvalue;
                                       continue;
                                     case tpString:
                                       arr[i++] = (int4)atoi((char*)sattr.base);
                                       continue;
                                     default:
                                       return false;
                                   }                                 
                               } while ((expr = expr->operand[1]) != NULL);
                               break;
                           } 
                           case tpInteger:
                             arr[0] = (int4)sattr.ivalue;
                             break;
                           case tpBoolean:
                             arr[0] = sattr.bvalue ? 1 : 0;
                             break;
                           case tpReal:
                             arr[0] = (int4)sattr.fvalue;
                             break;
                           case tpString:
                             arr[0] = (int4)atoi((char*)sattr.base);
                             break;
                           default:
                             error("Invalid array element type");
                             return false;
                         }
                         ((dbArray<int4>*)dst)->assign(arr, nElems, false);
                         elems->strValue = (char*)arr;
                         continue;
                     }
                     case dbField::tpInt8:
                     {
                         db_int8* arr = new db_int8[nElems];
                         switch (expr->type) { 
                           case tpList:
                           {
                               int i = 0;
                               do { 
                                   dbExprNode* elem = expr->operand[0];
                                   execute(elem, iattr, sattr);
                                   switch (elem->type) { 
                                     case tpInteger:
                                       arr[i++] = sattr.ivalue;
                                       continue;
                                     case tpBoolean:
                                       arr[i++] = sattr.bvalue ? 1 : 0;
                                       continue;
                                     case tpReal:
                                       arr[i++] = (db_int8)sattr.fvalue;
                                       continue;
                                     case tpString:
                                       arr[i++] = atol((char*)sattr.base);
                                       continue;
                                     default:
                                       return false;
                                   }                                 
                               } while ((expr = expr->operand[1]) != NULL);
                               break;
                           } 
                           case tpInteger:
                             arr[0] = sattr.ivalue;
                             break;
                           case tpBoolean:
                             arr[0] = sattr.bvalue ? 1 : 0;
                             break;
                           case tpReal:
                             arr[0] = (db_int8)sattr.fvalue;
                             break;
                           case tpString:
                             arr[0] = atol((char*)sattr.base);
                             break;
                           default:
                             error("Invalid array element type");
                             return false;
                         }
                         ((dbArray<db_int8>*)dst)->assign(arr, nElems, false);
                         elems->strValue = (char*)arr;
                         continue;
                     }
                     case dbField::tpReal4:
                     {
                         real4* arr = new real4[nElems];
                         switch (expr->type) { 
                           case tpList:
                           {
                               int i = 0;
                               do { 
                                   dbExprNode* elem = expr->operand[0];
                                   execute(elem, iattr, sattr);
                                   switch (elem->type) { 
                                     case tpInteger:
                                       arr[i++] = (real4)sattr.ivalue;
                                       continue;
                                     case tpBoolean:
                                       arr[i++] = (real4)(sattr.bvalue ? 1.0 : 0.0);
                                       continue;
                                     case tpReal:
                                       arr[i++] = (real4)sattr.fvalue;
                                       continue;
                                     case tpString:
                                       arr[i++] = (real4)atof((char*)sattr.base);
                                       continue;
                                     default:
                                       return false;
                                   }                                 
                               } while ((expr = expr->operand[1]) != NULL);
                               break;
                           } 
                           case tpInteger:
                             arr[0] = (real4)sattr.ivalue;
                             break;
                           case tpBoolean:
                             arr[0] = (real4)(sattr.bvalue ? 1.0 : 0.0);
                             break;
                           case tpReal:
                             arr[0] = (real4)sattr.fvalue;
                             break;
                           case tpString:
                             arr[0] = (real4)atof((char*)sattr.base);
                             break;
                           default:
                             error("Invalid array element type");
                             return false;
                         }
                         ((dbArray<real4>*)dst)->assign(arr, nElems, false);
                         elems->strValue = (char*)arr;
                         continue;
                     }
                     case dbField::tpReal8:
                     {
                         real8* arr = new real8[nElems];
                         switch (expr->type) { 
                           case tpList:
                           {
                               int i = 0;
                               do { 
                                   dbExprNode* elem = expr->operand[0];
                                   execute(elem, iattr, sattr);
                                   switch (elem->type) { 
                                     case tpInteger:
                                       arr[i++] = (real8)sattr.ivalue;
                                       continue;
                                     case tpBoolean:
                                       arr[i++] = sattr.bvalue ? 1.0 : 0.0;
                                       continue;
                                     case tpReal:
                                       arr[i++] = sattr.fvalue;
                                       continue;
                                     case tpString:
                                       arr[i++] = atoi((char*)sattr.base);
                                       continue;
                                     default:
                                       return false;
                                   }                                 
                               } while ((expr = expr->operand[1]) != NULL);
                               break;
                           } 
                           case tpInteger:
                             arr[0] = (real8)sattr.ivalue;
                             break;
                           case tpBoolean:
                             arr[0] = sattr.bvalue ? 1.0 : 0.0;
                             break;
                           case tpReal:
                             arr[0] = sattr.fvalue;
                             break;
                           case tpString:
                             arr[0] = atof((char*)sattr.base);
                             break;
                           default:
                             error("Invalid array element type");
                             return false;
                         }
                         ((dbArray<real8>*)dst)->assign(arr, nElems, false);
                         elems->strValue = (char*)arr;
                         continue;
                     }
                     case dbField::tpReference:
                     {
                         oid_t* arr = new oid_t[nElems];
                         switch (expr->type) { 
                           case tpList:
                           {
                               int i = 0;
                               do { 
                                   dbExprNode* elem = expr->operand[0];
                                   execute(elem, iattr, sattr);
                                   switch (elem->type) { 
                                     case tpInteger:
                                       arr[i++] = (oid_t)sattr.ivalue;
                                       continue;
                                     case tpReference:
                                       arr[i++] = sattr.oid;
                                       continue;
                                     case tpString:
                                       arr[i++] = (oid_t)atol((char*)sattr.base);
                                       continue;
                                     default:
                                       return false;
                                   }                                 
                               } while ((expr = expr->operand[1]) != NULL);
                               break;
                           } 
                           case tpInteger:
                             arr[0] = (oid_t)sattr.ivalue;
                             break;
                           case tpReference:
                             arr[0] = sattr.oid;
                             break;
                           case tpString:
                             arr[0] = (oid_t)atol((char*)sattr.base);
                             break;
                           default:
                             error("Invalid array element type");
                             return false;
                         }
                         ((dbArray<oid_t>*)dst)->assign(arr, nElems, false);
                         elems->strValue = (char*)arr;
                         continue;
                     }
                     default:
                       error("Usupported array type");
                       return false;
                   }
               } else {
                   dbAnyArray::arrayAllocator((dbAnyArray*)dst, NULL, 0);                   
               }
               continue;
           }
           case dbField::tpRectangle:
             if (expr->type != tpList) { 
                 error("Array coordinates expected");
                 return false;
             } else { 
                 rectangle& r = *(rectangle*)dst;
                 for (int i = 0; i < RECTANGLE_DIMENSION*2; i++) { 
                     if (expr == NULL || expr->operand[0] == NULL) { 
                         error("Bad rectangle constant");
                         return false;
                     }
                     dbExprNode* elem = expr->operand[0];
                     dbExprNode* tail = expr->operand[1];
                     if (elem->type == tpReal) {                         
                         r.boundary[i] = (coord_t)elem->fvalue;
                     } else if (elem->type == tpInteger) {  
                         r.boundary[i] = (coord_t)elem->ivalue;
                     } else { 
                         error("Bad rectangle constant");
                         return false;
                     }
                     expr = tail;
                 }
                 continue;
             }
           case dbField::tpBool:
             switch (expr->type) { 
               case tpInteger:
                 *(bool*)dst = sattr.ivalue != 0;
                 continue;
               case tpBoolean:
                 *(bool*)dst = sattr.bvalue;
                 continue;
               case tpReal:
                 *(bool*)dst = sattr.fvalue != 0;
                 continue;
               case tpString:
                 *(bool*)dst = *sattr.base == 'T' || *sattr.base == 't' || *sattr.base == '1';
                 continue;
             }
             break;
           case dbField::tpInt1:
             switch (expr->type) { 
               case tpInteger:
                 *(int1*)dst = (int1)sattr.ivalue;
                 continue;
               case tpBoolean:
                 *(int1*)dst = sattr.bvalue ? 1 : 0;
                 continue;
               case tpReal:
                 *(int1*)dst = (int1)sattr.fvalue;
                 continue;
               case tpString:
                 *(int1*)dst = (int1)atoi((char*)sattr.base);
                 continue;
             }
             break;
           case dbField::tpInt2:
             switch (expr->type) { 
               case tpInteger:
                 *(int2*)dst = (int2)sattr.ivalue;
                 continue;
               case tpBoolean:
                 *(int2*)dst = sattr.bvalue ? 1 : 0;
                 continue;
               case tpReal:
                 *(int2*)dst = (int2)sattr.fvalue;
                 continue;
               case tpString:
                 *(int2*)dst = (int2)atoi((char*)sattr.base);
                 continue;
             }
             break;
           case dbField::tpInt4:
             switch (expr->type) { 
               case tpInteger:
                 *(int4*)dst = (int4)sattr.ivalue;
                 continue;
               case tpBoolean:
                 *(int4*)dst = sattr.bvalue ? 1 : 0;
                 continue;
               case tpReal:
                 *(int4*)dst = (int4)sattr.fvalue;
                 continue;
               case tpString:
                 *(int4*)dst = (int1)atoi((char*)sattr.base);
                 continue;
             }
             break;
           case dbField::tpInt8:
             switch (expr->type) { 
               case tpInteger:
                 *(db_int8*)dst = sattr.ivalue;
                 continue;
               case tpBoolean:
                 *(db_int8*)dst = sattr.bvalue ? 1 : 0;
                 continue;
               case tpReal:
                 *(db_int8*)dst = (db_int8)sattr.fvalue;
                 continue;
               case tpString:
                 *(db_int8*)dst = (db_int8)atoi((char*)sattr.base);
                 continue;
             }
             break;
           case dbField::tpReal4:
             switch (expr->type) { 
               case tpInteger:
                 *(real4*)dst = (real4)sattr.ivalue;
                 continue;
               case tpBoolean:
                 *(real4*)dst = (real4)(sattr.bvalue ? 1 : 0);
                 continue;
               case tpReal:
                 *(real4*)dst = (real4)sattr.fvalue;
                 continue;
               case tpString:
                 *(real4*)dst = (real4)atof((char*)sattr.base);
                 continue;
             }
             break;
           case dbField::tpReal8:
             switch (expr->type) { 
               case tpInteger:
                 *(real8*)dst = (real8)sattr.ivalue;
                 continue;
               case tpBoolean:
                 *(real8*)dst = sattr.bvalue ? 1.0 : 0.0;
                 continue;
               case tpReal:
                 *(real8*)dst = sattr.fvalue;
                 continue;
               case tpString:
                 *(real8*)dst = atof((char*)sattr.base);
                 continue;
             }
             break;
           case dbField::tpString:
             src = buf;
             switch (expr->type) { 
               case tpInteger:
                 sprintf(buf, INT8_FORMAT, sattr.ivalue);
                 break;
               case tpBoolean:
                 strcpy(buf, sattr.bvalue ? "t" : "f");
                 break;
               case tpReal:
                 sprintf(buf, "%f", sattr.fvalue);
                 break;
               case tpString:
                 src = (char*)sattr.base;
                 break;
               case tpWString:
                 error("Can not update multibyte string with wide character string");
                 continue;
             }
             *(char**)dst = new char[strlen(src)+1];
             strcpy(*(char**)dst, src);
             elems->strValue = *(char**)dst;
             continue;
           case dbField::tpWString:
           {
               if (expr->type == tpString) { 
                   size_t len = mbstowcs(NULL, (char*)sattr.base, 0);
                   wchar_t* buf = new wchar_t[len+1];
                   mbstowcs(buf, (char*)sattr.base, len);
                   buf[len] = '\0';
                   *(wchar_t**)dst = buf; 
                   elems->strValue = (char*)buf;
               } else if (expr->type == tpWString) { 
                   size_t len = wcslen((wchar_t*)sattr.base);
                   wchar_t* buf = new wchar_t[len+1];
                   memcpy(buf, sattr.base, (len+1)*sizeof(wchar_t));
                   elems->strValue = (char*)buf;
               }
               continue;
           }
           case dbField::tpReference:
             if (expr->type == tpReference) { 
                 *(oid_t*)dst = sattr.oid;
                 continue;
             } else if (expr->type == tpInteger) { 
                 *(oid_t*)dst = (oid_t)sattr.ivalue;
                 continue;
             }
         }
         error("Mismatched type of update expression");
         return false;
     } while ((elems = elems->next) != NULL);

     return true;
}

void dbSubSql::deleteColumns(dbFieldDescriptor* columns)
{
    if (columns != NULL) { 
        dbFieldDescriptor *next, *fd = columns;
        do {
            next = fd->next;
            fd->type = dbField::tpUnknown;
            fd->longName = NULL;
            delete fd;
            fd = next;
        } while (next != columns);
    }                    
}

void dbSubSql::profile()
{
    printf("TABLES:\n");
    printf("   Fixed   Fields  Columns     Rows    Total  Aligned  TableName\n");
    printf("----------------------------------------------------------------\n");
    beginTransaction(dbSharedLock);
    for (dbTableDescriptor* desc=tables; desc != NULL; desc=desc->nextDbTable)
    { 
        dbRecord* rec;
        dbTable* table = (dbTable*)getRow(desc->tableId);
        size_t totalSize = 0;
        size_t totalAlignedSize = 0;
        for (oid_t oid = table->firstRow; oid != 0; oid = rec->next) {
            rec = getRow(oid);
            totalSize += rec->size;
            totalAlignedSize += DOALIGN(rec->size, dbAllocationQuantum);
        }            
        printf("%8ld %8ld %8ld %8ld %8ld %8ld  %s\n",
               (long)desc->fixedSize,(long)desc->nFields, (long)desc->nColumns, 
               (long)table->nRows,  (long)totalSize, (long)totalAlignedSize,
               desc->name);
    }
}        


bool dbSubSql::parse() 
{
    dbTableDescriptor* desc = NULL;
    dbFieldDescriptor* fd;
    int tkn;
    bool count, outputOid, compactify;
    dbFieldDescriptor* columns;

    line = 1;
    pos = 0;

    while (true) { 
        if (in == stdin) { 
            printf(prompt);
            tkn = scan();
            pos += (int)strlen(prompt);
        } else { 
            tkn = scan();
        }           

        switch (tkn) { 
          case tkn_update:
            if (!opened) { 
                error("Database not opened");
                continue;
            }
            if (accessType == dbReadOnly || accessType == dbConcurrentRead) { 
                error("Operation is not possible in read-only mode");
                continue;
            }
            dotIsPartOfIdentifier = true;
            if (expect("table name", tkn_ident)) {
                if ((desc = findTable(name)) == NULL) { 
                    error("No such table in database");
                    continue;
                }
                if (!expect("set", tkn_set)) { 
                    continue;
                }

                dbDatabaseThreadContext* ctx = threadContext.get();
                byte *record = dbMalloc(desc->appSize);
                memset(record, 0, desc->appSize);
                ctx->interactive = true;
                ctx->catched = true;
                dbUpdateElement* elems = NULL;
                if (!expect("field name", tkn_ident)) { 
                    goto updateCleanup;
                }
        
#ifdef THROW_EXCEPTION_ON_ERROR
                try {
#else
                if (setjmp(ctx->unwind) == 0) {
#endif
                    
                    char* condition = NULL;
                    int startPos = pos;
                    while (true) { 
                        dbUpdateElement* elem = new dbUpdateElement;
                        dbFieldDescriptor* fd = desc->findSymbol(name);
                        if (fd == NULL) { 
                            error("No such field in the table");
                            goto updateCleanup;
                        }
                        if (fd->type > dbField::tpArray && fd->type != dbField::tpWString && fd->type != dbField::tpRectangle) { 
                            error("Field can not be updated");
                            goto updateCleanup;
                        }
                        elem->field = fd;
                        elem->next = elems;
                        elems = elem;
                        if (!expect("=", tkn_eq)) { 
                            goto updateCleanup;
                        }
                        startPos = pos;
                        int ch = readExpression();
                        if (ch == EOF) { 
                            error("unexpected end of input");
                            goto updateCleanup;
                        } 
                        condition = strstr(buf, "where");
                        if (condition != NULL) {
                            *condition = '\0';
                        }
                        dbExprNode* expr = ctx->compiler.compileExpression(desc, buf, startPos);
                        if (expr == NULL) { 
                            goto updateCleanup;
                        }
                        if (expr->type > tpReference && expr->type != tpList) {
                            error("Invalid expression type");
                            goto updateCleanup;
                        }
                        elem->value = expr;
                        if (condition == NULL && ch == ',') { 
                            if (!expect("field name", tkn_ident)) { 
                                goto updateCleanup;
                            }
                        } else { 
                            break;
                        }
                    }
                    dbAnyCursor cursor(*desc, dbCursorForUpdate, record);
                    cursor.reset();
                        
                    if (condition != NULL) { 
                        query.pos = startPos + (int)(condition - buf) + 5;
                        query = (char const*)(condition + 5);
                        select(&cursor, query);
                        if (query.compileError()) { 
                            goto updateCleanup;
                        }                                   
                    } else { 
                        select(&cursor);
                    }
                    if (cursor.gotoFirst()) { 
                        do { 
                            cursor.fetch();
                            if (!updateFields(&cursor, elems)) { 
                                goto updateCleanup;
                            }   
                            cursor.update();
                        } while (cursor.gotoNext());
                    }
                    printf("\n\t%d records updated\n", cursor.getNumberOfRecords());
#ifdef THROW_EXCEPTION_ON_ERROR
                } catch(dbException const&) {}
#else
                } else { 
                    if (query.mutexLocked) { 
                        query.mutexLocked = false;
                        query.mutex.unlock();
                    }
                }
#endif
              updateCleanup:
                query.reset();
                while (elems != NULL) { 
                    dbUpdateElement* elem = elems;
                    elems = elems->next;
                    delete elem;
                }
                dbExprNodeAllocator::instance.reset();
                ctx->catched = false;
                dbFree(record);
            }
            break;

          case tkn_select:
            if (!opened) { 
                error("Database not opened");
                continue;
            }
            outputOid = true;
            count = false;
            if ((tkn = scan()) == tkn_all) { 
                outputOid = false;
                tkn = scan();
            } else if (tkn == tkn_count) { 
                if (!expect("'('", tkn_lpar)
                    || !expect("'*'", tkn_all)
                    || !expect("')'", tkn_rpar))
                {
                    continue;
                }
                count = true;
                tkn = scan();
            }
            columns = NULL;
            if (tkn != tkn_from) {
                while (true) { 
                    if (tkn != tkn_ident) { 
                        error("Field name or 'from' expected");
                    }
                    dbFieldDescriptor* column = new dbFieldDescriptor(name);
                    if (columns != NULL) { 
                        column->next = columns;
                        column->prev = columns->prev;
                        column->prev->next = column;
                        columns->prev = column;
                    } else { 
                        columns = column;
                        column->prev = column->next = column;
                    }
                    tkn = scan();
                    if (tkn != tkn_comma) { 
                        break;
                    }
                    tkn = scan();
                }
            }
            if (tkn != tkn_from) {
                deleteColumns(columns);
                error("FROM expected");
                continue;
            }
            dotIsPartOfIdentifier = true;
            if (scan() != tkn_ident) {
                deleteColumns(columns);
                error("Table name expected");
                continue;
            }
            if ((desc = findTable(name)) != NULL) {     
                dbAnyCursor cursor(*desc, dbCursorViewOnly, NULL);
                query.pos = pos;
                dbDatabaseThreadContext* ctx = threadContext.get();
                ctx->interactive = true;
                ctx->catched = true;
#ifdef THROW_EXCEPTION_ON_ERROR
                try {
#else
                if (setjmp(ctx->unwind) == 0) {
#endif
                    if (readCondition()) { 
                        query = (char const*)buf;
                        cursor.reset();
                        select(&cursor, query);
                        if (query.compileError()) { 
                            deleteColumns(columns);
                            dbExprNodeAllocator::instance.reset();
                            ctx->catched = false;
                            break;
                        }                
                    } else { 
                        deleteColumns(columns);
                        ctx->catched = false;
                        break;
                    }
                    if (count) { 
                        printf("%d records selected\n",
                               cursor.getNumberOfRecords());
                    } else { 
                        if (cursor.gotoFirst()) { 
                            dbFieldDescriptor* columnList;
                            if (columns != NULL) { 
                                columnList = columns;
                                dbFieldDescriptor* cc = columns; 
                                do { 
                                    dbFieldDescriptor* next = cc->next;
                                    dbFieldDescriptor* fd = desc->columns;
                                    do { 
                                        if (cc->name == fd->name) { 
                                            *cc = *fd;
                                            cc->next = next;
                                            goto Found;
                                        }
                                    } while ((fd = fd->next) != desc->columns);                                
                                    char buf[256];
                                    sprintf(buf, "Column '%s' is not found\n", cc->name);
                                    error(buf);
                                  Found:
                                    printf("%s ", fd->name);
                                    cc = next;
                                } while (cc != columns);
                            } else {                  
                                columnList = desc->columns;
                                dbFieldDescriptor* fd = columnList;
                                do { 
                                    printf("%s ", fd->name);
                                } while ((fd = fd->next) != columnList);
                            }
                            if (outputOid) { 
                                printf("\n#%lx: (", (unsigned long)cursor.currId);
                            } else { 
                                printf("\n(");
                            }
                            dumpRecord((byte*)getRow(cursor.currId), columnList); 
                            printf(")");
                            while (cursor.gotoNext()) { 
                                if (outputOid) { 
                                    printf(",\n#%lx: (", (unsigned long)cursor.currId);
                                } else { 
                                    printf(",\n(");
                                }
                                dumpRecord((byte*)getRow(cursor.currId), columnList); 
                                printf(")");
                            }
                            printf("\n\t%d records selected\n", 
                                   cursor.getNumberOfRecords());
                        } else { 
                            fprintf(stderr, "No records selected\n");
                        }
                    }
#ifdef THROW_EXCEPTION_ON_ERROR
                } catch(dbException const&) {}
#else
                } else { 
                    if (query.mutexLocked) { 
                        query.mutexLocked = false;
                        query.mutex.unlock();
                    }
                }
#endif
                deleteColumns(columns); 
                ctx->catched = false;
            } else { 
                error("No such table in database");
            }
            break;
                
          case tkn_open:
            if (expect("database name", tkn_sconst)) { 
                char* databaseName = new char[strlen(buf)+1];
                strcpy(databaseName, buf);
                char* fileName = NULL;
                if ((tkn = scan()) == tkn_sconst) { 
                    fileName = buf;
                } else if (tkn != tkn_semi) { 
                    delete[] databaseName;
                    error("database file name expected");
                    continue;
                }
                if (opened) { 
                    close();
                    while (droppedTables != NULL) { 
                        dbTableDescriptor* next = droppedTables->nextDbTable;
                        delete droppedTables; 
                        droppedTables = next;
                    }
                    opened = false;
                    dbTableDescriptor::cleanup();
                }
                time_t transactionCommitDelay = 0;
                char* delay = getenv("FASTDB_COMMIT_DELAY");
                if (delay != NULL) { 
                    transactionCommitDelay = atoi(delay);
                }
                if (!open(WC_STRING(databaseName), WC_STRING(fileName), INFINITE, transactionCommitDelay)) { 
                    fprintf(stderr, "Database not opened\n");
                } else { 
                    opened = true;
                    loadMetaTable();
                    existedTables = tables;
                    char* backupName = getenv("FASTDB_BACKUP_NAME");
                    if (backupName != NULL) { 
                        char* backupPeriod = getenv("FASTDB_BACKUP_PERIOD");
                        time_t period = 60*60*24; // one day
                        if (backupPeriod != NULL) { 
                            period = atoi(backupPeriod);
                        }
                        printf("Schedule backup to file %s each %u seconds\n", 
                               backupName, (unsigned)period);   
                        scheduleBackup(WC_STRING(backupName), period);                     
                    }
                }
                delete[] databaseName;
            }
            break;
                    
          case tkn_drop:
            if (!opened) { 
                error("Database not opened");
                continue;
            }
            if (accessType == dbReadOnly || accessType == dbConcurrentRead) { 
                error("Operation is not possible in read-only mode");
                continue;
            }
            if (monitor->users != 1) { 
                error("Can not perform operation with active appliations");
                continue;
            }
            switch (scan()) { 
              case tkn_table:
                dotIsPartOfIdentifier = true;
                if (expect("table name", tkn_ident)) { 
                    desc = findTable(name);
                    if (desc == NULL) { 
                        error("No such table in database");
                    } else { 
                        dropTable(desc);
                        if (desc == existedTables) { 
                            existedTables = desc->nextDbTable;
                        }
                        unlinkTable(desc);
                        desc->nextDbTable = droppedTables;
                        droppedTables = desc;
                    }
                }
                break;
              case tkn_hash:
                fd = readFieldName();
                if (fd != NULL) { 
                    if (fd->hashTable == 0) { 
                        error("This field is not hashed");
                    } else { 
                        dropHashTable(fd);
                    }
                }
                break;
              case tkn_index:
                fd = readFieldName();
                if (fd != NULL) { 
                    if (fd->tTree == 0) { 
                        error("There is no index for this field");
                    } else { 
                        dropIndex(fd);
                    }
                }
                break;
              default:
                error("Expecting 'table', 'hash' or 'index' keyword");
                continue;
            }
            break;

          case tkn_backup:
            if (!opened) { 
                error("Database not opened");
                continue;
            }
            compactify = false;
            if ((tkn = scan()) == tkn_compactify) {
                compactify = true;
                tkn = scan();
            }
            if (tkn != tkn_sconst) { 
                 error("Backup file name expected");
            } else { 
                if (!backup(WC_STRING(buf), compactify)) { 
                    printf("Backup failed\n");
                } else { 
                    while (droppedTables != NULL) { 
                        dbTableDescriptor* next = droppedTables->nextDbTable;
                        delete droppedTables; 
                        droppedTables = next;
                    }
                    commit();
                    existedTables = tables;
                }
            }
            continue;
            
          case tkn_create:
            if (!opened) { 
                error("Database not opened");
                continue;
            }
            if (accessType == dbReadOnly || accessType == dbConcurrentRead) { 
                error("Operation is not possible in read-only mode");
                continue;
            }
            if (monitor->users != 1) { 
                error("Can not perform operation with active appliations\n");
                continue;
            }
            switch (scan()) { 
              case tkn_hash:
                if (!expect("on", tkn_on)) { 
                    continue;
                }
                fd = readFieldName();
                if (fd != NULL) { 
                    if (fd->hashTable != 0) { 
                        error("This field is already hashed");
                    } else { 
                        createHashTable(fd);
                    }
                }
                break;
              case tkn_index:
                if (!expect("on", tkn_on)) { 
                    continue;
                }
                fd = readFieldName();
                if (fd != NULL) { 
                    if (fd->tTree != 0) { 
                        error("Index already exists");
                    } else { 
                        createIndex(fd);
                    }
                }
                break;
                
              case tkn_table:
                updateTable(true);
                break;

              default:
                error("Expecting 'table', 'hash' or 'index' keyword");
                continue;
            }
            break;

          case tkn_rename:
            if (!opened) {
                error("Database not opened");
                continue;
            }
            if (accessType == dbReadOnly || accessType == dbConcurrentRead) { 
                error("Operation is not possible in read-only mode");
                continue;
            }
            fd = readFieldName(tkn_to);
            if (fd != NULL) { 
                if (expect("new field name", tkn_ident) && expect(";", tkn_semi)) { 
                    beginTransaction(dbExclusiveLock);
                    dbTable* table = (dbTable*)getRow(fd->defTable->tableId);
                    int   nRows = table->nRows;
                    oid_t first = table->firstRow;
                    oid_t last = table->lastRow;
                    fd->name = name;
                    delete[] fd->longName;
                    fd->longName = new char[strlen(name)+1];
                    strcpy(fd->longName, name);
                    size_t newSize = sizeof(dbTable) + desc->nFields*sizeof(dbField)
                        + fd->defTable->totalNamesLength();
                    table = (dbTable*)putRow(fd->defTable->tableId, newSize);
                    fd->defTable->storeInDatabase(table);
                    table->firstRow = first;
                    table->lastRow = last;
                    table->nRows = nRows;
                }
            }
            break;
           

          case tkn_alter:
            if (!opened) {
                error("Database not opened");
                continue;
            }
            if (accessType == dbReadOnly || accessType == dbConcurrentRead) { 
                error("Operation is not possible in read-only mode");
                continue;
            }
            if (monitor->users != 1) { 
                error("Can not perform operation with active appliations\n");
                continue;
            }
            switch (scan()) {
              case tkn_table:
                updateTable(false);
                break;
              default:
                error("Expecting 'table' keyword");
                continue;
            }
            break;

          case tkn_insert:
            if (!opened) { 
                error("Database not opened");
                continue;
            }
            if (accessType == dbReadOnly || accessType == dbConcurrentRead) { 
                error("Operation is not possible in read-only mode");
                continue;
            }
            if (expect("into", tkn_into)) {
                dotIsPartOfIdentifier = true;
                if (expect("table name", tkn_ident)) {
                    if ((desc = findTable(name)) == NULL) { 
                        error("No such table in database");
                        continue;
                    }
                    if (!expect("values", tkn_values)) { 
                        continue;
                    }
                    beginTransaction(dbExclusiveLock);
                    modified = true;
                    while (expect("(", tkn_lpar)) { 
                        dbList* list = NULL;
                        int n = readValues(&list);
                        if (n <= 0 || !insertRecord(list, desc)) { 
                            if (n == 0) { 
                                error("Empty fields list");
                            }
                            tkn = tkn_semi; // just avoid extra error messages
                        } else { 
                            tkn = scan();
                        }
                        while (list != NULL) { 
                            dbList* tail = list->next;
                            delete list;
                            list = tail;
                        }
                        if (tkn == tkn_semi) { 
                            break;
                        } else if (tkn != tkn_comma) { 
                            error("';' or ',' expected");
                        }
                    }
                }
            }
            break;
          case tkn_delete:
            if (!opened) { 
                error("Database not opened");
                continue;
            }
            if (accessType == dbReadOnly || accessType == dbConcurrentRead) { 
                error("Operation is not possible in read-only mode");
                continue;
            }
            if (expect("FROM", tkn_from)) { 
                dotIsPartOfIdentifier = true;
                if (expect("table name", tkn_ident)) {
                    if ((desc = findTable(name)) == NULL) {
                        error("No such table in database");
                        continue;
                    } 
                    dbAnyCursor cursor(*desc, dbCursorForUpdate, NULL);
                    dbDatabaseThreadContext* ctx = threadContext.get();
                    ctx->interactive = true;
                    ctx->catched = true;
        
#ifdef THROW_EXCEPTION_ON_ERROR
                    try {
#else
                        if (setjmp(ctx->unwind) == 0) {
#endif
                            if (readCondition()) {
                                query = (char const*)buf;
                                cursor.reset();
                                select(&cursor, query);
                                if (query.compileError()) {
                                    dbExprNodeAllocator::instance.reset();
                                    ctx->catched = false;
                                    break;
                                }
                            } else {
                                ctx->catched = false;
                                break;
                            }
                            int n_deleted = cursor.getNumberOfRecords();
                            cursor.removeAllSelected();
                            printf("\n\t%d records deleted\n", n_deleted);
#ifdef THROW_EXCEPTION_ON_ERROR
                        } catch(dbException const&) {}
#else
                    } else { 
                        if (query.mutexLocked) { 
                            query.mutexLocked = false;
                            query.mutex.unlock();
                        }
                    }
#endif
                    ctx->catched = false;
                }
            }
            break;

          case tkn_commit:
            if (!opened) { 
                error("Database not opened");
            } else { 
                while (droppedTables != NULL) { 
                    dbTableDescriptor* next = droppedTables->nextDbTable;
                    delete droppedTables; 
                    droppedTables = next;
                }
                commit();
                existedTables = tables;
            }
            continue;

          case tkn_rollback:
            if (!opened) { 
                error("Database not opened");
            } else { 
                while (droppedTables != NULL) { 
                    dbTableDescriptor* next = droppedTables->nextDbTable;
                    linkTable(droppedTables, droppedTables->tableId); 
                    droppedTables = next;
                }
                rollback();
                while (tables != existedTables) { 
                    dbTableDescriptor* table = tables;
                    unlinkTable(table);
                    delete table;
                }
            } 
            continue;

          case tkn_memory:
            if (!opened) { 
                error("Database not opened");
            } else { 
                dbMemoryStatistic stat;
                beginTransaction(dbSharedLock);
                getMemoryStatistic(stat);
                printf("Used memory: %ld\nFree memory: %ld\nNumber of holes: %ld\nMaximal hole size: %ld\nMinimal hole size: %ld\nAverage hole size: %ld\n\n", 
                       (long)stat.used,
                       (long)stat.free,
                       (long)stat.nHoles,
                       (long)stat.maxHoleSize,
                       (long)stat.minHoleSize,
                       (long)(stat.nHoles != 0 ? (stat.free / stat.nHoles) : 0));
                for (int i = 0; i < dbDatabaseOffsetBits; i++) { 
                    if (stat.nHolesOfSize[i] != 0) { 
                        printf("Number of holes of size [%ld...%ld): %ld\n", 1L << i, 1L << (i+1), (long)stat.nHolesOfSize[i]);
                    }
                }
            }
            break;

          case tkn_profile:
            if (!opened) { 
                error("Database not opened");
            } else { 
                profile();
            }
            break;

          case tkn_show:
            if (!opened) { 
                error("Database not opened");
            } else { 
                beginTransaction(dbSharedLock);
                printf("FastDB   version  :  %d.%02d\n"
                       "Database version  :  %d.%02d\n"
                       "Database file size: %lu Kb\n"
                       "Object index size : %lu handles\n"
                       "Used part of index: %lu handles\n"
                       "Number of users   : %d\n"
                       "Number of readers : %d\n" 
                       "Number of writers : %d\n" 
                       "Number of blocked readers : %d\n" 
                       "Number of blocked writers : %d\n", 
                       FASTDB_MAJOR_VERSION, FASTDB_MINOR_VERSION, 
                       header->majorVersion, header->minorVersion, 
                       (unsigned long)(header->size/1024),
                       (unsigned long)header->root[1-header->curr].indexSize, 
                       (unsigned long)header->root[1-header->curr].indexUsed,
                       monitor->users,
                       monitor->nReaders,
                       monitor->nWriters,
                       monitor->nWaitReaders,
                       monitor->nWaitWriters + monitor->waitForUpgrade);
                printf("\nTABLES:\n");
                printf("OID       FixedSize   Fields  Columns  TableName\n");
                printf("---------------------------------------------------------\n");
                for (dbTableDescriptor* desc=tables; desc != NULL; desc=desc->nextDbTable)
                { 
                    printf("0x%06x  %8ld %8ld %8ld   %s\n",
                           (int)desc->tableId, (long)desc->fixedSize,
                           (long)desc->nFields, (long)desc->nColumns, desc->name);
                }
            } 
            break;
                       
          case tkn_describe:
            if (!opened) {
                error("Database not opened");
                continue;
            }   
            dotIsPartOfIdentifier = true;
            if (expect("table name", tkn_ident)) {
                if ((desc = findTable(name)) == NULL) {
                    error("No such table in database");
                    continue;
                }
                printf("\nOID=0x%06x, TableName=%s\n", (int)desc->tableId, desc->name);
                printf("No Index FieldType        RefTableName     FieldName        InverseFieldName Flg\n");
                printf("--------------------------------------------------------------------------------\n");
                dbFieldDescriptor* fd = desc->columns; 
                for (int i = (int)desc->nColumns; --i >= 0;) { 
                    printf("%-2d %-5s %-16s %-16s %-16s %-16s %x\n", 
                           fd->fieldNo,
                           fd->hashTable != 0 && fd->tTree != 0 ? "H+T"
                           : fd->hashTable != 0 ? "H" : fd->tTree != 0 ? "T" : "-",
                           typeMnem[fd->type],
                           fd->refTableName != NULL
                               ? fd->refTableName 
                               : (fd->type == dbField::tpArray && fd->components->refTableName != NULL)
                                  ? fd->components->refTableName
                                  : "(null)", 
                           fd->name, 
                           (fd->inverseRefName != NULL ? fd->inverseRefName : "(null)"),
                           fd->indexType);
                    fd = fd->next;
                }
            }
            continue;

          case tkn_export:
            if (!opened) {
                error("Database not opened");
                continue;
            }   
            if (expect("xml file name", tkn_sconst)) { 
                if (!exportDatabaseToXml(WC_STRING(buf))) { 
                    error("Failed to open output file");
                }
            }
            break;

          case tkn_import:
            if (!opened) {
                error("Database not opened");
                continue;
            }   
            if (accessType == dbReadOnly || accessType == dbConcurrentRead) { 
                error("Operation is not possible in read-only mode");
                continue;
            }
            if (expect("xml file name", tkn_sconst)) { 
                if (!importDatabaseFromXml(WC_STRING(buf))) { 
                    error("Import from XML file failed");
                }
            }
            break;

          case tkn_autocommit:
            switch (scan()) {
              case tkn_on:
                autocommit = true;
                break;
               case tkn_off:
                autocommit = false;
                break;
              default:
                error("ON or OFF expected");
            }
            continue;

          case tkn_help:
            fprintf(stderr, "SubSQL commands:\n\n\
open 'database-name' ( 'database-file-name' ) ';'\n\
select ('*') from <table-name> where <condition> ';'\n\
update <table-name> set <field-name> '=' <expression> {',' <field-name> '=' <expression>} where <condition> ';'\n\
create table <table-name> '('<field-name> <field-type> {',' <field-name> <field-type>}')' ';' \n\
alter table <table-name> '('<field-name> <field-type> {',' <field-name> <field-type>}')' ';' \n\
rename <table-name> '.' <old-field-name> 'to' <new-field-name> ';' \n\
delete from <table-name>\n\
drop table <table-name>\n\
drop index <table-name> {'.' <field-name>} ';'\n\
create index on <table-name> {'.' <field-name>} ';'\n\
drop hash <table-name> {'.' <field-name>};\n\
create hash on <table-name> {'.' <field-name>}field> ';'\n\
insert into <table-name> values '(' <value>{',' <value>} ')' ';'\n\
backup [compactify] 'file-name'\n\
start server URL number-of-threads\n\
stop server URL\n\
start http server URL\n\
stop http server\n\
describe <table-name>\n\
import 'xml-file-name'\n\
export 'xml-file-name'\n\
show\n\
profile\n\
commit\n\
rollback\n\
autocommit (on|off)\n\
exit\n\
help\n\n");
            continue;
          case tkn_start:
            if (!opened) { 
                error("Database not opened");
            } else { 
                commit(); // allow server threads to process
                existedTables = tables;
                tkn = scan();
                if (tkn == tkn_http) { 
                    if (expect("server", tkn_server)
                        && expect("HTTP server URL", tkn_sconst))
                    {
#if !SERVER_SUPPORTED
                        error("Database was build without server support");
#else
                        startHttpServer(buf);
#endif
                    }
                } else if (tkn == tkn_server && expect("server URL", tkn_sconst)) { 
#if !SERVER_SUPPORTED
                    error("Database was build without server support");
#else
                    dbServer* server = dbServer::find(buf);
                    if (server == NULL) { 
                        char* serverURL = new char[strlen(buf)+1];
                        strcpy(serverURL, buf);
                        if (expect("number of threads", tkn_iconst)) { 
                            server = new dbServer(this, serverURL, (int)ival);
                            printf("Server started for URL %s\n", serverURL);
                        }
                        delete[] serverURL;
                    }
                    if (server != NULL) { 
                        server->start();
                    }
#endif
                } else { 
                    error("Token 'server' expected");
                }
            }
            continue;
          case tkn_stop:
            tkn = scan();
            if (tkn == tkn_http) { 
                if (expect("server", tkn_server) && expect("HTTP server URL", tkn_sconst))
                {
#if !SERVER_SUPPORTED
                    error("Database was build without server support");
#else
                    stopHttpServer(buf);
#endif
                }
            } else if (tkn == tkn_server) { 
                if (expect("server URL", tkn_sconst))
                {
#if !SERVER_SUPPORTED
                    error("Database was build without server support");
#else
                    dbServer* server = dbServer::find(buf);
                    if (server != NULL) { 
                        server->stop();
                        printf("Server stopped for URL %s\n", buf);
                    } else { 
                        fprintf(stderr, "No server was started for URL %s\n", buf);
                    }
#endif
                }
            } else { 
                error("Token 'server' expected");
            }
            continue;
          case tkn_semi:
            putchar('\n');
            // no break
          case tkn_error:
            continue;
          case tkn_exit:
            return false;
          case tkn_version:
            printf("FastDB version %d.%02d\n", FASTDB_MAJOR_VERSION, FASTDB_MINOR_VERSION);
            continue;
          case tkn_eof:
            return true;
          default:
            error("Unexpected token");
            continue;
        }
        if (autocommit || !modified) { 
            while (droppedTables != NULL) { 
                dbTableDescriptor* next = droppedTables->nextDbTable;
                delete droppedTables; 
                droppedTables = next;
            }
            commit();
            existedTables = tables;
        }
    }
}


void dbSubSql::handleError(dbErrorClass error, char const* msg, int arg)
{
    dbDatabaseThreadContext* ctx = threadContext.get();
    if (ctx == NULL || ctx->interactive) { 
        const int screenWidth = 80;
        int col;
        switch (error) { 
          case QueryError:
            col = arg % screenWidth;
            if (in == stdin) { 
                while (--col >= 0) putc('-', stderr);
                fprintf(stderr, "^\n%s\n", msg);
            } else { 
                fprintf(stderr, "%s at line %d position %d\n", msg, line, arg);
            }
            break;
          case ArithmeticError:
            fprintf(stderr, "%s\n", msg);
            break;
          case IndexOutOfRangeError:
            fprintf(stderr, "Index %d is out of range\n", arg);
            break;
          case NullReferenceError:
            fprintf(stderr, "Null object reference is accessed\n");
            break;
          case DatabaseOpenError:
            return;
          default:
            dbDatabase::handleError(error, msg, arg);
        }
        //
        // Recovery
        //
        if (in == stdin) { 
            int ch;
            while ((ch = get()) != '\n' && ch != EOF); 
        } else { 
            fseek(in, 0, SEEK_END);
        }
    } 
    switch (error) { 
      case DatabaseOpenError:
      case InconsistentInverseReference:
        fprintf(stderr, "%s\n", msg);
        break;
      default:
#ifdef THROW_EXCEPTION_ON_ERROR
        if (msg == NULL) { 
            msg = errorMessage[error];
        }
        throw dbException(error, msg, arg);
#else
        if (ctx != NULL) { 
            if (ctx->catched) {     
                longjmp(ctx->unwind, error);
            } else { 
                abort();
            }
        }
#endif
    }
}

void dbSubSql::run(int firstParam, int argc, char* argv[])
{
    for (int i = firstParam; i < argc; i++) { 
        if (strcmp(argv[i], "-") == 0) {
            break;
        } 
        in = fopen(argv[i], "r");
        if (in == NULL) { 
            fprintf(stderr, "Failed to open '%s' file\n", argv[i]);
        } else { 
            if (!parse()) { 
                if (opened) { 
                    close();
                }
#if SERVER_SUPPORTED
                dbServer::cleanup();
#endif
                return;
            }
        }
    }
    printf("SubSQL interactive utility for FastDB v. %d.%.02d\n"
           "Type 'help' for more information\n", 
           FASTDB_MAJOR_VERSION, FASTDB_MINOR_VERSION);
    in = stdin;
    parse();
    if (opened) { 
        close();
    }
#if SERVER_SUPPORTED
    dbServer::cleanup();
#endif
}

#if SERVER_SUPPORTED
#define HTML_HEAD "Content-type: text/html\r\n\r\n\
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\"><HTML><HEAD>"

#define BODY "<BODY BGCOLOR=\"#c0c0c0\">"

#define EMPTY_LIST "<OPTION>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</OPTION></SELECT><BR>"


void httpQueryError(WWWconnection& con, char const* msg, char const* table)
{
    con << TAG <<
        HTML_HEAD "<TITLE>BUGDB error</TITLE></HEAD>"
        BODY
        "<CENTER><FONT SIZE=+2 COLOR=\"#FF0000\">"
        << msg << "</FONT></CENTER><P><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=socket VALUE=\"" << con.getAddress() << "\">"
        "<INPUT TYPE=hidden NAME=table VALUE=\"" << table << "\">"
        "<INPUT TYPE=hidden NAME=page VALUE=queryPage>"
        "<CENTER><INPUT TYPE=submit VALUE=\"Ok\"></CENTER></FORM></BODY></HTML>";
}


void httpError(WWWconnection& con, char const* msg)
{
    con << TAG <<
        HTML_HEAD "<TITLE>BUGDB error</TITLE></HEAD>"
        BODY
        "<CENTER><FONT SIZE=+2 COLOR=\"#FF0000\">"
        << msg << "</FONT></CENTER><P><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=\"socket\" VALUE=\"" << con.getAddress() << "\">"
        "<INPUT TYPE=hidden NAME=\"page\" VALUE=defaultPage>"
        "<CENTER><INPUT TYPE=submit VALUE=\"Ok\"></CENTER></FORM></BODY></HTML>";
}


bool defaultPage(WWWconnection& con)
{
    ((dbSubSql*)con.userData)->defaultPage(con);
    return true;
}


void dbSubSql::defaultPage(WWWconnection& con)
{
    con << TAG <<
        HTML_HEAD "<TITLE>Database browser</TITLE></HEAD>"
        BODY
        "<TABLE><TR><TH align=left>Database name</TH><TD>" << databaseName << "</TD></TR>"
        "<TR><TH align=left>FastDB version</TH><TD>" << FASTDB_MAJOR_VERSION << "." << FASTDB_MINOR_VERSION << "</TD></TR>"
        "<TR><TH align=left>Database version</TH><TD>" << header->majorVersion << "." << header->minorVersion << "</TD></TR>"
        "<TR><TH align=left>Database size</TH><TD>" << (int)(getDatabaseSize() / 1024) << "Kb</TD></TR>"
        "</TABLE><P>"
        "<H2>Tables:</H2><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=socket VALUE=\"" << con.getAddress() << "\">"
        "<INPUT TYPE=hidden NAME=page VALUE=queryPage>"
        "<SELECT SIZE=10 NAME=table>";
    if (tables != NULL && tables->nextDbTable != NULL) { 
        for (dbTableDescriptor* desc=tables; desc != NULL; desc=desc->nextDbTable)
        { 
            if (strcmp(desc->name, "Metatable") != 0) { 
                con << TAG << "<OPTION VALUE=\"" << desc->name << "\">" << desc->name << "</OPTION>";
            }
        }
    } else { 
        con << TAG << EMPTY_LIST;
    }
    con << TAG << "</SELECT><P><INPUT TYPE=submit VALUE=Query></FORM></BODY></HTML>";
}
    
bool queryPage(WWWconnection& con)
{
    ((dbSubSql*)con.userData)->queryPage(con);
    return true;
}

char const* indexType(dbFieldDescriptor* fd) { 
    return (fd->tTree != 0) 
        ? ((fd->hashTable != 0) ? "HT" : "T") 
        : ((fd->hashTable != 0) ? "H" : " ");
}

void dbSubSql::queryPage(WWWconnection& con)
{
    char* tableName = con.get("table");
    if (tableName == NULL) { 
        httpError(con, "Table not selected");
        return;
    }
    dbTableDescriptor* desc = findTableByName(tableName);
    if (desc == NULL) {
        httpError(con, "No such table");
        return;
    }
    char* history = con.get("history");
    if (history == NULL) { 
        history = "";
    }
    con << TAG <<
        HTML_HEAD "<TITLE>Table query</TITLE></HEAD>"
        BODY
        "<H2>Table " << tableName << "</H2>"
        "<TABLE BORDER><TR><TH>Field name</TH><TH>Field type</TH><TH>Referenced table</TH><TH>Inverse reference</TH><TH>Index</TH></TR>";
    dbFieldDescriptor* fd = desc->columns; 
    for (int i = (int)desc->nColumns; --i >= 0;) { 
        con << TAG << "<TR><TD>" << fd->name << "</TD><TD>" << typeMnem[fd->type] << "</TD><TD>" 
            << (fd->refTableName != NULL 
                ? fd->refTableName 
                : (fd->type == dbField::tpArray && fd->components->refTableName != NULL)
                  ? fd->components->refTableName
                  : " ")  << "</TD><TD>" 
            << (fd->inverseRefName ? fd->inverseRefName : " ") 
            << "</TD><TD align=center>"
            << indexType(fd) << "</TD></TR>";
        fd = fd->next;
    }
    con << TAG << "</TABLE><P><TABLE>"
        "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
        "<INPUT TYPE=HIDDEN NAME=socket VALUE=\"" << con.getAddress() << "\">"
        "<INPUT TYPE=hidden NAME=page VALUE=selectionPage>"
        "<INPUT TYPE=hidden NAME=table VALUE=\"" << tableName << "\">"
        "<TR><TD>SELECT FROM <B>" << tableName << "</B> WHERE</TD>"
        "<TD><INPUT TYPE=text NAME=query VALUE=\""
        << history << "\" SIZE=40></TD>"
        "<TD><INPUT type=submit value=Select></TD></TR></FORM>";
    if (historyUsed != 0) { 
        con << TAG << "<FORM METHOD=POST ACTION=\"" << con.getStub() << "\">"
            "<INPUT TYPE=HIDDEN NAME=socket VALUE=\"" << con.getAddress() << "\">"
            "<INPUT TYPE=hidden NAME=page VALUE=queryPage>"
            "<INPUT TYPE=hidden NAME=table VALUE=\"" << tableName << "\">"
            "<TR><TD align=right>Query history</TD>"
            "<TD><SELECT SIZE=1 NAME=history>";
        for (unsigned i = historyCurr, j = historyUsed; j != 0; j -= 1) { 
            char* h = queryHistory[--i % MAX_HISTORY_SIZE];
            con << TAG << "<OPTION VALUE=\"" << h << "\">" << h << "</OPTION>";
        }
        con << TAG << "</TD><TD><INPUT type=submit value=Edit></TD></TR></FORM>";
    }
    con << TAG << "</TABLE></FORM>"
        "<P><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">" 
        "<INPUT TYPE=HIDDEN NAME=socket VALUE=\"" << con.getAddress() << "\">"
        "<INPUT TYPE=hidden NAME=page VALUE=defaultPage>"
        "<INPUT TYPE=submit VALUE=\"Another table\"></FORM></BODY></HTML>";
}

    
enum ComponentType { 
    RecordComponent,
    ArrayComponent, 
    StructureComponent
};

void httpDumpRecord(WWWconnection& con, byte* base, dbFieldDescriptor* first, ComponentType componentType,
                    int &buttonCounter, const char *condition, int buttonIndex)
{
    int i, n;
    byte* elem;
    // only maxDumpArrayLength entries will be shown for an array. 
    // To turn off this feature, set maxDumpArrayLength to zero.
    const int maxDumpArrayLength = 20;
    dbFieldDescriptor* fd = first;
    do {
        if (componentType == StructureComponent) { 
            con << TAG << "<TR><TH>" << fd->name << "</TH><TD>";
        } else if (componentType == RecordComponent) { 
            con << TAG << "<TD>";
        }
        switch (fd->type) {
          case dbField::tpBool:
            con << TAG << (*(bool*)(base + fd->dbsOffs) ? "true" : "false");
            break;
          case dbField::tpInt1:
            con << TAG << *(int1*)(base + fd->dbsOffs);
            break;
          case dbField::tpInt2:
            con << TAG << *(int2*)(base + fd->dbsOffs);
            break;
          case dbField::tpInt4:
            con << TAG << *(int4*)(base + fd->dbsOffs);
            break;
          case dbField::tpInt8:
            con << TAG << *(db_int8*)(base + fd->dbsOffs);
            break;
          case dbField::tpReal4:
            con << TAG << *(real4*)(base + fd->dbsOffs);
            break;
          case dbField::tpReal8:
            con << TAG << *(real8*)(base + fd->dbsOffs);
            break;
          case dbField::tpRectangle:
            {
                rectangle& r = *(rectangle*)(base + fd->dbsOffs);
                con << TAG << "<TABLE BORDER><TR>";                
                for (i = 0; i < rectangle::dim; i++) { 
                    con << TAG << "<TD>" << r.boundary[i] << "</TD>";
                }
                con << TAG << "</TR><TR>";                
                for (i = 0; i < rectangle::dim; i++) { 
                    con << TAG << "<TD>" << r.boundary[rectangle::dim+i] << "</TD>";
                }
                con << TAG << "</TR></TABLE>";
            }
            break;
          case dbField::tpString:
            con << TAG << "\"" << ((char*)base+((dbVarying*)(base+fd->dbsOffs))->offs) << "\"";
            break;
          case dbField::tpWString:
            con << TAG << "\"" << (wchar_t*)((char*)base+((dbVarying*)(base+fd->dbsOffs))->offs) << "\"";
            break;
          case dbField::tpReference:
            {
                oid_t oid = *(oid_t*)(base + fd->dbsOffs);
                if (oid == 0) { 
                    con << TAG << "null";
                } else { 
                    con << TAG << "<A HREF=\"" <<  con.getStub() << "?socket="
                        << con.getAddress() << "&page=selectionPage&table=" << URL << fd->refTableName <<  "&query=" 
                        << URL << "current=" << oid << TAG << "\">@" << oid << "</A>";
                }
            }
            break;
          case dbField::tpRawBinary:
            n = (int)fd->dbsSize;
            elem = base + fd->dbsOffs;
            con << TAG << "\"";
            for (i = 0; i < n; i++) {
                char buf[8];
                sprintf(buf, "\\x%02x", *elem++);
                con << TAG << buf;
            }
            con << TAG << "\"";
            break;
          case dbField::tpArray:
            {
                n = ((dbVarying*)(base + fd->dbsOffs))->size;
                elem = base + ((dbVarying*)(base + fd->dbsOffs))->offs;
                con << TAG << "<OL>";

                // limit array size to maxDumpArrayLength
                bool bigList = false;
                bool showButton = false;
                int oldn = n;
                if (maxDumpArrayLength > 0 && n > maxDumpArrayLength) { 
                    bigList = true; }

                if (bigList && buttonCounter != buttonIndex) {
                    n = maxDumpArrayLength;
                    showButton = true;
                }

                for (i = 0; i < n; i++) {
                    con << TAG << "<LI>";                
                    httpDumpRecord(con, elem, fd->components, ArrayComponent, buttonCounter, condition, buttonIndex);
                    elem += fd->components->dbsSize;
                }
                con << TAG << "</OL>";

                if (showButton)
                {
                    con << TAG << "<P align=\"center\"><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">" 
                                  "<INPUT TYPE=HIDDEN NAME=socket VALUE=\"" << con.getAddress() << "\">"
                                  "<INPUT TYPE=hidden NAME=page VALUE=selectionPage>"
                                  "<INPUT TYPE=hidden NAME=table VALUE=\"" << first->defTable->getName() << "\">"
                                  "<INPUT TYPE=hidden NAME=query VALUE=\"" << condition << "\">"
                                  "<INPUT TYPE=hidden NAME=button VALUE=\"" << buttonCounter << "\">"
                                  "<INPUT TYPE=submit VALUE=\"Full list (" << oldn << ")\"></FORM></P>";
                }

                if (bigList)
                   buttonCounter++;

            } // case
            break;
          case dbField::tpStructure:
            con << TAG << "<TABLE BORDER>"; 
            httpDumpRecord(con, base, fd->components,RecordComponent, buttonCounter, condition, buttonIndex);
            con << TAG << "</TABLE>"; 
        }
        if (componentType == StructureComponent) { 
            con << TAG << "</TD></TR>";
        } else if (componentType == RecordComponent) { 
            con << TAG << "</TD>";
        }
    } while ((fd = fd->next) != first);
}

bool selectionPage(WWWconnection& con)
{
    ((dbSubSql*)con.userData)->selectionPage(con);
    return true;
}

void dbSubSql::selectionPage(WWWconnection& con)
{
    char const* tableName = con.get("table");
    char const* condition = con.get("query");
    char const* buttonNr  = con.get("button");

    int buttonIndex = -1;
    if (buttonNr) {
        buttonIndex = atoi(buttonNr); }

    dbTableDescriptor* desc = findTableByName(tableName);
    if (desc == NULL) {
        httpError(con, "No such table");
        return;
    }
    if (condition == NULL) {
        httpError(con, "Condition was not specified");
        return;
    }
    if (strlen(condition) > 0 
        && (historyUsed == 0 
            || strcmp(condition, queryHistory[unsigned(historyCurr-1)%MAX_HISTORY_SIZE]) != 0))
    {
        char* h = new char[strlen(condition)+1];
        strcpy(h, condition);
        if (historyCurr == historyUsed) { 
            historyUsed += 1;
        } else { 
            delete[] queryHistory[historyCurr];
        }
        queryHistory[historyCurr] = h;
        if (++historyCurr == MAX_HISTORY_SIZE) { 
            historyCurr = 0;
        }
    }
    dbAnyCursor cursor(*desc, dbCursorViewOnly, NULL);
    query.pos = pos;
    dbDatabaseThreadContext* ctx = threadContext.get();
    ctx->interactive = false;
    ctx->catched = true;
#ifdef THROW_EXCEPTION_ON_ERROR
    try {
#else
    if (setjmp(ctx->unwind) == 0) {
#endif
        query = condition;
        cursor.reset();
        select(&cursor, query);
        if (query.compileError()) {
            dbExprNodeAllocator::instance.reset();
            ctx->catched = false;
            httpQueryError(con, "query syntax error", tableName);
            return;
        }
        con << TAG <<
            HTML_HEAD "<TITLE>Selection</TITLE></HEAD>" 
            BODY
            "<H2>Selection from table " << tableName << "</H2>"
            "<TABLE BORDER><TR><TH>OID</TH>";
        dbFieldDescriptor* fd = desc->columns;
        do {
            con << TAG << "<TH>" << fd->name << "</TH>";
        } while ((fd = fd->next) != desc->columns);
        con << TAG << "</TR>";

        int nSelected = 0;
        int buttonCounter = 0;        
        if (cursor.gotoFirst()) {
            do {
                nSelected += 1;
                con << TAG << "<TR><TD>@" << cursor.currId << "</TD>";
                httpDumpRecord(con, (byte*)getRow(cursor.currId),
                               cursor.table->columns, RecordComponent,
                               buttonCounter, condition, buttonIndex);
                con << TAG << "</TR>";
            } while (cursor.gotoNext());
            con << TAG << "</TABLE>";
            if (nSelected > 1) {
                con << TAG << "<P>" << nSelected << " records selected";
            }
        } else { 
            con << TAG << "</TABLE><P>No records selected";
        }
#ifdef THROW_EXCEPTION_ON_ERROR
    } catch(dbException const& x) {
        httpQueryError(con, x.getMsg(), tableName);
        ctx->catched = false;
        commit(); // release locks
        return;
    }
#else
    } else { 
        httpQueryError(con, "Query error", tableName);
        if (query.mutexLocked) { 
            query.mutexLocked = false;
            query.mutex.unlock();
        }
        ctx->catched = false;
        commit(); // release locks
        return;
    }
#endif
    ctx->catched = false;
    commit(); // release locks
    
    con << TAG << 
        "<P><FORM METHOD=POST ACTION=\"" << con.getStub() << "\">" 
        "<INPUT TYPE=HIDDEN NAME=socket VALUE=\"" << con.getAddress() << "\">"
        "<INPUT TYPE=hidden NAME=table VALUE=\"" << tableName << "\">"
        "<INPUT TYPE=hidden NAME=page VALUE=queryPage>"
        "<INPUT TYPE=submit VALUE=\"New query\"></FORM></BODY></HTML>";
}
    

WWWapi::dispatcher dispatchTable[] = {
    {"defaultPage", defaultPage},
    {"queryPage", queryPage},
    {"selectionPage", selectionPage}
};




void dbSubSql::startHttpServer(char const* address) 
{
    if (httpServerRunning) { 
        error("HTTP server already started");
    } else { 
        httpServer = new HTTPapi(*this, itemsof(dispatchTable), dispatchTable);
        if (!httpServer->open(address, socket_t::sock_global_domain)) {
            delete httpServer;
            error("Failed to open HTTP session");
        } else { 
            httpServerRunning = true;
            httpServerThread.create(httpServerThreadProc, this);
        }
    }
}

void dbSubSql::stopHttpServer(char const* address) 
{
    if (!httpServerRunning) { 
        error("HTTP server was not started");
    } else {
        httpServerRunning = false;
        httpServer->cancel();        
    }
}

void thread_proc dbSubSql::httpServerThreadProc(void* arg) 
{
    ((dbSubSql*)arg)->httpServerLoop();
}
    
void  dbSubSql::httpServerLoop() 
{
    WWWconnection con;
    con.userData = this;
    attach();
    while (httpServer->connect(con) && httpServerRunning && httpServer->serve(con));
    delete httpServer;
    detach();
    httpServerRunning = false;
}
#endif

END_FASTDB_NAMESPACE

#ifdef _WINCE

#define MAX_ARGS 32

#ifdef WINONLY
int WINAPI WinMain(
				   HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nCmdShow)
{
	char* line = m_cmdline;
#else
int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR lpCmdLine,
    int nCmdShow)
{
	int len = wcslen(lpCmdLine);
	char* line   = new char[len+1];
	wcstombs(line, lpCmdLine, len);	
#endif
	int argc = 1;
	char* argv[MAX_ARGS];
	argv[0] = NULL;
	if (*line != '\0') {
		while (true) { 
			if (*line == '\"') { 
				argv[argc++] = ++line;
				while (*++line != '\"' && *line != '\0');
			} else { 
				argv[argc++] = line;
				while (*++line != ' ' && *line != '\0');
			}
			if (*line == '\0') { 
				break;
			}
			*line++ = '\0';
		}
	}
	argv[argc] = NULL;
#else // !_WINCE
#ifdef VXWORKS
int subsql_main()
{
	int argc = 0;
	char* argv[] = {0};
#else // VXWORKS
int main(int argc, char* argv[]) 
{
#endif // VXWORKS
#endif // !_WINCE
    char* subsqlAccessType = getenv("SUBSQL_ACCESS_TYPE");    
    FASTDB_NS::dbDatabase::dbAccessType accessType = FASTDB_NS::dbDatabase::dbAllAccess;
    int firstParam = 1;
    if (argc > 1 && (strcmp(argv[1], "-?") == 0 || strcmp(argv[1], "-h") == 0)) { 
        printf("SubSQL interactive utility for FastDB v. %d.%.02d\n",
               FASTDB_MAJOR_VERSION, FASTDB_MINOR_VERSION);
        printf("Usage: subsql (-access [read-only|concurrent-read|all-access]) (SQL-FILE)*\n");
        return 1;
    }
    char* locale = getenv("SUBSQL_LOCALE");
    if (locale != NULL) { 
        setlocale(LC_COLLATE, locale);
        setlocale(LC_CTYPE, locale);
    }

    if  (argc > firstParam+1 && strcmp(argv[firstParam], "-access") == 0) { 
        subsqlAccessType = argv[firstParam+1];
        firstParam += 2;
    }         
    if (subsqlAccessType != NULL) { 
        if (strcmp(subsqlAccessType, "read-only") == 0) { 
            accessType = FASTDB_NS::dbDatabase::dbReadOnly;
        } else if (strcmp(subsqlAccessType, "concurrent-read") == 0) { 
            accessType = FASTDB_NS::dbDatabase::dbConcurrentRead;
        } else if (strcmp(subsqlAccessType, "concurrent-update") == 0) { 
            accessType = FASTDB_NS::dbDatabase::dbConcurrentUpdate;
        } else if (strcmp(subsqlAccessType, "normal") && strcmp(subsqlAccessType, "all-access") 
                   && strcmp(subsqlAccessType, "read-write") && strcmp(subsqlAccessType, "update"))
        {
            fprintf(stderr, "Invalid access type: '%s'\n", subsqlAccessType);
            return 1;
        }
    }
    FASTDB_NS::dbSubSql db(accessType);
    db.run(firstParam, argc, argv);
    return 0;
}
    
