#define _JNI_IMPLEMENTATION_ 1
#include <jni.h>

#include "fastdb.h"
#include "localcli.h"

static cli_var_type const cliTypeMap[] = {
    cli_oid, 
    cli_bool,   // TP_BOOL
    cli_int2,   // TP_CHAR
    cli_int1,   // TP_INT1
    cli_int2,   // TP_INT2
    cli_int4,   // TP_INT4
    cli_int8,   // TP_INT8
    cli_real4,  // TP_REAL4
    cli_real8,  // TP_REAL8
    cli_asciiz, // TP_STRING
    cli_int8,   // TP_DATE 
    cli_array_of_bool, // TP_BOOL_ARRAY
    cli_array_of_int2, // TP_CHAR_ARRAY
    cli_array_of_int1, // TP_INT1_ARRAY
    cli_array_of_int2, // TP_INT2_ARRAY
    cli_array_of_int4, // TP_INT4_ARRAY
    cli_array_of_int8, // TP_INT8_ARRAY
    cli_array_of_real4, // TP_REAL4_ARRAY
    cli_array_of_real8, // TP_REAL8_ARRAY
    cli_array_of_string, // TP_STRING_ARRAY
};

typedef char* charptr;
struct _abool { char n; bool v; };
struct _achar { char n; char v; };
struct _ajchar { char n; jchar v; };
struct _ajbyte { char n; jbyte v; };
struct _ajshort { char n; jshort v; };
struct _ajint { char n; jint v; };
struct _ajlong { char n; jlong v; };
struct _ajfloat { char n; jfloat v; };
struct _ajdouble { char n; jdouble v; };
struct _acharptr { char n; charptr v; };
struct _acli_array_t { char n; cli_array_t v; };

#define ALIGNOF(type) offsetof(_a##type, v)
#define SIZE_AND_ALIGNMENT(type) sizeof(type), ALIGNOF(type)
#define APPEND_FIXED(type) appendFixed(SIZE_AND_ALIGNMENT(type))

inline char const* getStringBody(JNIEnv* env, jstring str) {
    return env->GetStringUTFChars(str, 0);
}

inline void releaseStringBody(JNIEnv* env, jstring str, char const* body) {
    env->ReleaseStringUTFChars(str, body);
}

inline int getStringLength(JNIEnv* env, jstring str) {
    return env->GetStringUTFLength(str);
}

inline jstring newString(JNIEnv* env, char const* body) {
    return env->NewStringUTF(body);
}

class ClassDescriptor {
  public:
    int nColumns;
    cli_field_descriptor* columns;
    char*                 names;

    ~ClassDescriptor() { 
        delete[] columns;
        delete[] names;
    }
};

const size_t VARYING_BLOCK_SIZE = 1024;

struct VaryingBlock { 
    VaryingBlock* next;
    char          data[1];
};

class ObjectBuffer {
    dbSmallBuffer fixedPart;
    VaryingBlock* varyingPart;
    size_t fixedPos;
    size_t varyingPos;

  public:
    void* base() {
        return fixedPart.base();
    }

    ObjectBuffer() { 
        fixedPos = 0;
        varyingPos = 0;
        varyingPart = NULL;
    }

    ~ObjectBuffer() { 
        VaryingBlock *curr, *next;
        for (curr = varyingPart; curr != NULL; curr = next) { 
            next = curr->next;
            dbFree(curr);
        }
    }

    void* appendFixed(size_t size, size_t alignment) { 
        fixedPos = DOALIGN(fixedPos, alignment);
        fixedPart.put(fixedPos + size);
        void* ptr = fixedPart.base() + fixedPos;
        fixedPos += size;
        return ptr;
    }

    void* appendVarying(size_t nElems, size_t elemSize, size_t elemAlignment) { 
        varyingPos = DOALIGN(varyingPos, elemAlignment);
        size_t size = nElems*elemSize;
        void* ptr;
        if (varyingPart != NULL && varyingPos + size <= VARYING_BLOCK_SIZE) { 
            ptr = &varyingPart->data[varyingPos];
            varyingPos += size;
        } else {
            VaryingBlock* blk = (VaryingBlock*)
                dbMalloc(sizeof(VaryingBlock*) + (size < VARYING_BLOCK_SIZE ? VARYING_BLOCK_SIZE : size));
            blk->next = varyingPart;
            varyingPart = blk;
            ptr = varyingPart->data;
            varyingPos = size;
        }
        return ptr;
    }
};

class JniResultSet {
  public:
    byte*  record;
    size_t pos;
    dbAnyCursor cursor;
    bool   first;

    JniResultSet(dbTableDescriptor* desc) 
    : record(new byte[desc->size()]), pos(0), cursor(*desc, dbCursorViewOnly, record), first(true)
    {
        memset(record, 0, desc->size());
        cursor.reset();
    }

    int size() { 
        return cursor.getNumberOfRecords();
    }

    oid_t next() {
        pos = 0;
        if (first) { 
            if (cursor.isEmpty()) { 
                delete this;
                return 0;
            }
            first = false;
        } else if (!cursor.gotoNext()) {
            delete this;
            return 0;
        }
        cursor.fetch();
        return cursor.currId;
    }

    jbyte nextByte() { 
        return (jbyte)record[pos++];
    }

    jshort nextShort() { 
        pos = DOALIGN(pos, ALIGNOF(jshort));
        jshort val = *(jshort*)(record + pos);
        pos += sizeof(jshort);
        return val;
    }

    jint nextInt() { 
        pos = DOALIGN(pos, ALIGNOF(jint));
        jint val = *(jint*)(record + pos);
        pos += sizeof(jint);
        return val;
    }

    jlong nextLong() { 
        pos = DOALIGN(pos, ALIGNOF(jlong));
        jlong val = *(jlong*)(record + pos);
        pos += sizeof(jlong);
        return val;
    }

    jstring nextString(JNIEnv* env) { 
        pos = DOALIGN(pos, ALIGNOF(charptr));
        jstring val = newString(env, *(char**)(record + pos));
        pos += sizeof(char*);
        return val;
    }
       
    jbooleanArray nextBoolArray(JNIEnv* env) { 
        pos = DOALIGN(pos, ALIGNOF(cli_array_t));
        cli_array_t* arr = (cli_array_t*)(record + pos);
        jbooleanArray val = env->NewBooleanArray(arr->size);
        env->SetBooleanArrayRegion(val, 0, arr->size, (jboolean*)arr->data);
        pos += sizeof(cli_array_t);
        return val;
    }

    jcharArray nextCharArray(JNIEnv* env) { 
        pos = DOALIGN(pos, ALIGNOF(cli_array_t));
        cli_array_t* arr = (cli_array_t*)(record + pos);
        jcharArray val = env->NewCharArray(arr->size);
        env->SetCharArrayRegion(val, 0, arr->size, (jchar*)arr->data);
        pos += sizeof(cli_array_t);
        return val;
    }

    jbyteArray nextByteArray(JNIEnv* env) { 
        pos = DOALIGN(pos, ALIGNOF(cli_array_t));
        cli_array_t* arr = (cli_array_t*)(record + pos);
        jbyteArray val = env->NewByteArray(arr->size);
        env->SetByteArrayRegion(val, 0, arr->size, (jbyte*)arr->data);
        pos += sizeof(cli_array_t);
        return val;
    }
       
    jshortArray nextShortArray(JNIEnv* env) { 
        pos = DOALIGN(pos, ALIGNOF(cli_array_t));
        cli_array_t* arr = (cli_array_t*)(record + pos);
        jshortArray val = env->NewShortArray(arr->size);
        env->SetShortArrayRegion(val, 0, arr->size, (jshort*)arr->data);
        pos += sizeof(cli_array_t);
        return val;
    }
       
    jintArray nextIntArray(JNIEnv* env) { 
        pos = DOALIGN(pos, ALIGNOF(cli_array_t));
        cli_array_t* arr = (cli_array_t*)(record + pos);
        jintArray val = env->NewIntArray(arr->size);
        env->SetIntArrayRegion(val, 0, arr->size, (jint*)arr->data);
        pos += sizeof(cli_array_t);
        return val;
    }

    jlongArray nextLongArray(JNIEnv* env) { 
        pos = DOALIGN(pos, ALIGNOF(cli_array_t));
        cli_array_t* arr = (cli_array_t*)(record + pos);
        jlongArray val = env->NewLongArray(arr->size);
        env->SetLongArrayRegion(val, 0, arr->size, (jlong*)arr->data);
        pos += sizeof(cli_array_t);
        return val;
    }

    jfloatArray nextFloatArray(JNIEnv* env) { 
        pos = DOALIGN(pos, ALIGNOF(cli_array_t));
        cli_array_t* arr = (cli_array_t*)(record + pos);
        jfloatArray val = env->NewFloatArray(arr->size);
        env->SetFloatArrayRegion(val, 0, arr->size, (jfloat*)arr->data);
        pos += sizeof(cli_array_t);
        return val;
    }

    jdoubleArray nextDoubleArray(JNIEnv* env) { 
        pos = DOALIGN(pos, ALIGNOF(cli_array_t));
        cli_array_t* arr = (cli_array_t*)(record + pos);
        jdoubleArray val = env->NewDoubleArray(arr->size);
        env->SetDoubleArrayRegion(val, 0, arr->size, (jdouble*)arr->data);
        pos += sizeof(cli_array_t);
        return val;
    }

    jobjectArray nextStringArray(JNIEnv* env) { 
        pos = DOALIGN(pos, ALIGNOF(cli_array_t));
        cli_array_t* arr = (cli_array_t*)(record + pos);
        int size = (int)arr->size;
        jobjectArray val = env->NewObjectArray(size, env->FindClass("java/lang/String"), NULL);
        char** spp = (char**)arr->data;
        for (int i = 0; i < size; i++) {
            env->SetObjectArrayElement(val, i, newString(env, spp[i]));
        }
        pos += sizeof(cli_array_t);
        return val;
    }

    ~JniResultSet() { 
        delete[] record;
    }
};

class JniDatabase : public dbDatabase {
  private:
    oid_t   tableId;
    oid_t   nextTableId;
    JNIEnv* env;

  public:
    JniDatabase(JNIEnv* env, dbAccessType type, size_t poolSize) 
    : dbDatabase(type, poolSize)
    {
        this->env = env;
    }

    virtual void handleError(dbErrorClass error, char const* msg, int arg = 0) {
        if (error != InconsistentInverseReference) { 
            jclass cls = env->FindClass("jnicli/CliException");
            if (cls != NULL) { 
                env->ThrowNew(cls, msg);        
            }
        }
    }

    void open(char const* databaseName, char const* databasePath, int transactionCommitDelay)
    {
        if (!dbDatabase::open(databaseName, databasePath, transactionCommitDelay)) {
            handleError(dbDatabase::DatabaseOpenError, "Failed to open database");
            return;
        }
        dbTable* table = (dbTable*)getRow(dbMetaTableId);
        dbTableDescriptor* metatable = new dbTableDescriptor(table);
        linkTable(metatable, dbMetaTableId);
        oid_t tid = table->firstRow;
        nextTableId = tid;
        while (tid != 0) {
            table = (dbTable*)getRow(tid);
            dbTableDescriptor* desc = new dbTableDescriptor(table);
            linkTable(desc, tid);
            desc->setFlags();
            tid = table->next;
        }
    }  

    jstring nextTable() {
        tableId = nextTableId;
        if (tableId == 0) { 
            if (!completeDescriptorsInitialization()) {
                close();
                handleError(dbDatabase::DatabaseOpenError, "Referenced table not found");
            }
            return NULL;
        }
        dbTable* table = (dbTable*)getRow(tableId);
        jstring name = newString(env, (char*)((byte*)table + table->name.offs));
        nextTableId = table->next;
        return name;
    }

    dbTableDescriptor* createTable(char const* name, ClassDescriptor* clsDesc) {
        if (accessType == dbConcurrentRead || accessType == dbReadOnly) { 
            handleError(dbDatabase::DatabaseOpenError, "Can not create tables in read-only mode");
            return NULL;
        } else { 
            beginTransaction(dbExclusiveLock);
            dbTableDescriptor* tableDesc = NULL;
            int rc = dbCLI::create_table(this, name, clsDesc->nColumns, clsDesc->columns);
            if (rc == cli_unsupported_type) {
                handleError(dbDatabase::DatabaseOpenError, "Unsupported table field type");
                return NULL;
            }
            return findTableByName(name);
        }
    }
        
    dbTableDescriptor* updateTable(char const* name, ClassDescriptor* clsDesc) {
        if (accessType == dbConcurrentRead || accessType == dbReadOnly) { 
            beginTransaction(dbSharedLock);
            return findTableByName(name);
        } else { 
            beginTransaction(dbExclusiveLock);
            int rc = dbCLI::alter_table(this, name, clsDesc->nColumns, clsDesc->columns);
            if (rc == cli_unsupported_type) {
                handleError(dbDatabase::DatabaseOpenError, "Unsupported table field type");
                return NULL;
            }
            return findTableByName(name);
        }
    }

    oid_t insert(dbTableDescriptor* desc, ObjectBuffer* buf) { 
        dbAnyReference ref;
        char** ptr = (char**)buf->base();
        insertRecord(desc, &ref, buf->base());
        delete buf;
        return ref.getOid();
    }

            
    JniResultSet* select(dbTableDescriptor* desc, char const* query) { 
        JniResultSet* rs = new JniResultSet(desc);        
        rs->cursor.select(query, dbCursorViewOnly, NULL);
        return rs;
    }
            

    void update(oid_t oid, dbTableDescriptor* desc, ObjectBuffer* buf) {
        dbDatabase::update(oid, desc, buf->base());
        delete buf;
    }

    int remove(dbTableDescriptor* desc, char const* query) { 
        JniResultSet rs(desc);        
        int result = rs.cursor.select(query, dbCursorForUpdate, NULL);
        rs.cursor.removeAllSelected();
        return result;
    }

    dbDatabaseThreadContext* getThreadContext() { 
        return threadContext.get();
    }
    
    void setThreadContext(dbDatabaseThreadContext* ctx) { 
        threadContext.set(ctx);
    }
            
    ClassDescriptor* createTableDescriptor(JNIEnv* env, jstring table, jobjectArray fields, jintArray types, jobjectArray referencedTables, jobjectArray inverseFields, jintArray constraintMasks)
    {
        int i;
        size_t totalNameLength = 0;
        int nFields = env->GetArrayLength(fields);
        cli_field_descriptor* columns = new cli_field_descriptor[nFields];
        for (i = 0; i < nFields; i++) { 
            jstring referencedTable = (jstring)env->GetObjectArrayElement(referencedTables, i);
            if (referencedTable != NULL) {                 
                totalNameLength += getStringLength(env, referencedTable) + 1;
                jstring inverseField = (jstring)env->GetObjectArrayElement(inverseFields, i);
                if (inverseField != NULL) {                 
                    totalNameLength += getStringLength(env, inverseField) + 1;
                }
            }
            totalNameLength += getStringLength(env, (jstring)env->GetObjectArrayElement(fields, i)) + 1;
        }
        char* names = new char[totalNameLength];
        char* np = names;
        jint* jniTypes = env->GetIntArrayElements(types, 0);
        jint* flags = env->GetIntArrayElements(constraintMasks, 0);

        for (i = 0; i < nFields; i++) { 
            jstring referencedTable = (jstring)env->GetObjectArrayElement(referencedTables, i);
            columns[i].flags = flags[i];
            columns[i].refTableName = NULL;
            columns[i].inverseRefFieldName = NULL;
            cli_var_type cliType = cliTypeMap[jniTypes[i]];
            if (referencedTable != NULL) {
                switch (cliType) {
                  case cli_int4:
                  case cli_int8:
                    cliType = cli_oid;
                    break;
                  case cli_array_of_int4:
                  case cli_array_of_int8:
                    cliType = cli_array_of_oid;
                    break;
                  default:
                    handleError(dbDatabase::DatabaseOpenError, "Reference field should have int or long type");
                }                    
                columns[i].refTableName = np;
                int len = getStringLength(env, referencedTable);
                char const* body = getStringBody(env, referencedTable);
                memcpy(np, body, len);
                np[len] = 0;
                releaseStringBody(env, referencedTable, body);
                np += len+1;

                jstring inverseField = (jstring)env->GetObjectArrayElement(inverseFields, i);
                if (inverseField != NULL) {                 
                    columns[i].inverseRefFieldName = np;
                    int len = getStringLength(env, inverseField);
                    char const* body = getStringBody(env, inverseField);
                    memcpy(np, body, len);
                    np[len] = 0;
                    releaseStringBody(env, inverseField, body);
                    np += len+1;
                }                    
            } else if (columns[i].flags & AUTOINCREMENT) { 
                cliType = cli_autoincrement;
            }
            columns[i].type = cliType;
            columns[i].name = np;
            jstring fieldName = (jstring)env->GetObjectArrayElement(fields, i);
            int len = getStringLength(env, fieldName);
            char const* body = getStringBody(env, fieldName);
            memcpy(np, body, len);
            np[len] = 0;
            releaseStringBody(env, fieldName, body);
            np += len+1;
        }
        env->ReleaseIntArrayElements(constraintMasks, flags, 0);
        env->ReleaseIntArrayElements(types, jniTypes, 0);
        ClassDescriptor* desc = new ClassDescriptor();
        desc->columns = columns;
        desc->names = names;
        desc->nColumns = nFields;
        char const* body = getStringBody(env, table);
        releaseStringBody(env, table, body);
        return desc;
    }
};

    

jlong JNICALL   jniOpen(JNIEnv* env, jclass, jint accessType, jstring databaseName, jstring databasePath, jlong initSize, jint transactionCommitDelay)
{
    JniDatabase* db = new JniDatabase(env,
                                      (dbDatabase::dbAccessType)accessType, 
                                      (size_t)initSize);
    char const* name = getStringBody(env, databaseName);
    char const* path = getStringBody(env, databasePath);
    db->open(name, path, transactionCommitDelay);
    releaseStringBody(env, databaseName, name);
    releaseStringBody(env, databasePath, path);
    return (jlong)db;
}
        
void JNICALL   jniClose(JNIEnv* env, jclass, jlong db)
{
    JniDatabase* jdb = (JniDatabase*)db;
    jdb->close();
    delete jdb;
    dbTableDescriptor::cleanup();
}

jstring JNICALL jniNextTable(JNIEnv* env, jclass, jlong db)
{
    return ((JniDatabase*)db)->nextTable();
}

jlong JNICALL   jniUpdateTable(JNIEnv* env, jclass, jlong db, jstring table, jlong desc)
{
    char const* name = getStringBody(env, table);
    dbTableDescriptor* td = ((JniDatabase*)db)->updateTable(name, (ClassDescriptor*)desc);
    releaseStringBody(env, table, name);
    return (jlong)td;
}


jlong JNICALL   jniCreateBuffer(JNIEnv* env, jclass)
{
    return (jlong)new ObjectBuffer();
}

void JNICALL   jniSetByte(JNIEnv* env, jclass, jlong bptr, jbyte v)
{
    ObjectBuffer* buf = (ObjectBuffer*)bptr;
    *(jbyte*)buf->APPEND_FIXED(jbyte) = v;
}
    
void JNICALL   jniSetShort(JNIEnv* env, jclass, jlong bptr, jshort v)
{
    ObjectBuffer* buf = (ObjectBuffer*)bptr;
    *(jshort*)buf->APPEND_FIXED(jshort) = v;
}

void JNICALL   jniSetInt(JNIEnv* env, jclass, jlong bptr, jint v)
{
    ObjectBuffer* buf = (ObjectBuffer*)bptr;
    *(jint*)buf->APPEND_FIXED(jint) = v;
}

void JNICALL   jniSetLong(JNIEnv* env, jclass, jlong bptr, jlong v)
{
    ObjectBuffer* buf = (ObjectBuffer*)bptr;
    *(jlong*)buf->APPEND_FIXED(jlong) = v;
}

void JNICALL   jniSetString(JNIEnv* env, jclass, jlong bptr, jstring v)
{
    ObjectBuffer* buf = (ObjectBuffer*)bptr;
    if (v == NULL) { 
        *(char const**)buf->APPEND_FIXED(charptr) = "";
    } else { 
        char const* s = getStringBody(env, v);
        size_t len = getStringLength(env, v);
        char* str = (char*)buf->appendVarying(len+1, SIZE_AND_ALIGNMENT(char));
        memcpy(str, s, len);    
        str[len] = 0;
        *(char**)buf->APPEND_FIXED(charptr) = str;
        releaseStringBody(env, v, s);
    }
}

void JNICALL   jniSetBoolArray(JNIEnv* env, jclass, jlong bptr, jbooleanArray v)
{
    ObjectBuffer* buf = (ObjectBuffer*)bptr;
    size_t len = 0;
    char* elems = NULL;
    if (v != NULL) { 
        jboolean* booleans = env->GetBooleanArrayElements(v, 0);
        len = env->GetArrayLength(v);
        elems = (char*)buf->appendVarying(len, SIZE_AND_ALIGNMENT(bool));
        memcpy(elems, booleans, len);    
        env->ReleaseBooleanArrayElements(v, booleans, 0);
    }
    cli_array_t* arr = (cli_array_t*)buf->APPEND_FIXED(cli_array_t);
    arr->size = len;
    arr->data = elems;
    arr->allocated = 0;
}

void JNICALL   jniSetCharArray(JNIEnv* env, jclass, jlong bptr, jcharArray v)
{
    ObjectBuffer* buf = (ObjectBuffer*)bptr;
    size_t len = 0;
    jchar* elems = NULL;
    if (v != NULL) { 
        jchar* chars = env->GetCharArrayElements(v, 0);
        len = env->GetArrayLength(v);
        elems = (jchar*)buf->appendVarying(len, SIZE_AND_ALIGNMENT(jchar));
        memcpy(elems, chars, len*sizeof(jchar));    
        env->ReleaseCharArrayElements(v, chars, 0);
    }
    cli_array_t* arr = (cli_array_t*)buf->APPEND_FIXED(cli_array_t);
    arr->size = len;
    arr->data = elems;
    arr->allocated = 0;
}

void JNICALL   jniSetByteArray(JNIEnv* env, jclass, jlong bptr, jbyteArray v)
{
    ObjectBuffer* buf = (ObjectBuffer*)bptr;
    size_t len = 0;
    char* elems = NULL;
    if (v != NULL) { 
        jbyte* bytes = env->GetByteArrayElements(v, 0);
        len = env->GetArrayLength(v);
        elems = (char*)buf->appendVarying(len, SIZE_AND_ALIGNMENT(jbyte));
        memcpy(elems, bytes, len);    
        env->ReleaseByteArrayElements(v, bytes, 0);
    }
    cli_array_t* arr = (cli_array_t*)buf->APPEND_FIXED(cli_array_t);
    arr->size = len;
    arr->data = elems;
    arr->allocated = 0;
}

void JNICALL   jniSetShortArray(JNIEnv* env, jclass, jlong bptr, jshortArray v)
{
    ObjectBuffer* buf = (ObjectBuffer*)bptr;
    size_t len = 0;
    SHORT* elems = NULL;
    if (v != NULL) { 
        jshort* shorts = env->GetShortArrayElements(v, 0);
        len = env->GetArrayLength(v);
        elems = (short*)buf->appendVarying(len, SIZE_AND_ALIGNMENT(jshort));
        memcpy(elems, shorts, len*sizeof(short));    
        env->ReleaseShortArrayElements(v, shorts, 0);
    }
    cli_array_t* arr = (cli_array_t*)buf->APPEND_FIXED(cli_array_t);
    arr->size = len;
    arr->data = elems;
    arr->allocated = 0;
}

void JNICALL   jniSetIntArray(JNIEnv* env, jclass, jlong bptr, jintArray v)
{
    ObjectBuffer* buf = (ObjectBuffer*)bptr;
    size_t len = 0;
    int* elems = NULL;
    if (v != NULL) { 
        jint* ints = env->GetIntArrayElements(v, 0);
        len = env->GetArrayLength(v);
        elems = (int*)buf->appendVarying(len, SIZE_AND_ALIGNMENT(jint));
        memcpy(elems, ints, len*sizeof(int));    
        env->ReleaseIntArrayElements(v, ints, 0);
    }
    cli_array_t* arr = (cli_array_t*)buf->APPEND_FIXED(cli_array_t);
    arr->size = len;
    arr->data = elems;
    arr->allocated = 0;
}

void JNICALL   jniSetLongArray(JNIEnv* env, jclass, jlong bptr, jlongArray v)
{
    ObjectBuffer* buf = (ObjectBuffer*)bptr;
    size_t len = 0;
    jlong* elems = NULL;
    if (v != NULL) { 
        jlong* longs = env->GetLongArrayElements(v, 0);
        len = env->GetArrayLength(v);
        elems = (jlong*)buf->appendVarying(len, SIZE_AND_ALIGNMENT(jlong));
        memcpy(elems, longs, len*sizeof(jlong));    
        env->ReleaseLongArrayElements(v, longs, 0);
    }
    cli_array_t* arr = (cli_array_t*)buf->APPEND_FIXED(cli_array_t);
    arr->size = len;
    arr->data = elems;
    arr->allocated = 0;
}

void JNICALL   jniSetFloatArray(JNIEnv* env, jclass, jlong bptr, jfloatArray v)
{
    ObjectBuffer* buf = (ObjectBuffer*)bptr;
    size_t len = 0;
    float* elems = NULL;
    if (v != NULL) { 
        jfloat* floats = env->GetFloatArrayElements(v, 0);
        len = env->GetArrayLength(v);
        elems = (float*)buf->appendVarying(len, SIZE_AND_ALIGNMENT(jfloat));
        memcpy(elems, floats, len*sizeof(float));    
        env->ReleaseFloatArrayElements(v, floats, 0);
    }
    cli_array_t* arr = (cli_array_t*)buf->APPEND_FIXED(cli_array_t);
    arr->size = len;
    arr->data = elems;
    arr->allocated = 0;
}

void JNICALL   jniSetDoubleArray(JNIEnv* env, jclass, jlong bptr, jdoubleArray v)
{
    ObjectBuffer* buf = (ObjectBuffer*)bptr;
    size_t len = 0;
    double* elems = NULL;
    if (v != NULL) { 
        jdouble* doubles = env->GetDoubleArrayElements(v, 0);
        len = env->GetArrayLength(v);
        elems = (double*)buf->appendVarying(len, SIZE_AND_ALIGNMENT(jdouble));
        memcpy(elems, doubles, len*sizeof(double));    
        env->ReleaseDoubleArrayElements(v, doubles, 0);
    }
    cli_array_t* arr = (cli_array_t*)buf->APPEND_FIXED(cli_array_t);
    arr->size = len;
    arr->data = elems;
    arr->allocated = 0;
}

void JNICALL   jniSetStringArray(JNIEnv* env, jclass, jlong bptr, jobjectArray v)
{
    ObjectBuffer* buf = (ObjectBuffer*)bptr;
    int len = 0;
    char** elems = NULL;
    if (v != NULL) { 
        int len = (int)env->GetArrayLength(v);
        size_t total_length = 0;
        int i;
        for (i = 0; i < len; i++) { 
            total_length += getStringLength(env, (jstring)env->GetObjectArrayElement(v, i));
        }
        elems = (char**)buf->appendVarying(len + ((total_length+len) + sizeof(char*) - 1) / sizeof(char*), SIZE_AND_ALIGNMENT(charptr));
        char* body = (char*)(elems + len);
        for (i = 0; i < len; i++) { 
            jstring str = (jstring)env->GetObjectArrayElement(v, i);
            char const* sb = getStringBody(env, str);
            size_t strlen =  getStringLength(env, str);
            elems[i] = body;
            memcpy(body, sb, strlen);
            body += strlen;
            *body++ = 0;
            releaseStringBody(env, str, sb);
        }
    }
    cli_array_t* arr = (cli_array_t*)buf->APPEND_FIXED(cli_array_t);
    arr->size = len;
    arr->data = elems;
    arr->allocated = 0;
}


jlong JNICALL   jniCreateTable(JNIEnv* env, jclass, jlong db, jstring table, jlong desc)
{
    char const* name = getStringBody(env, table);
    dbTableDescriptor* td = ((JniDatabase*)db)->createTable(name, (ClassDescriptor*)desc);
    releaseStringBody(env, table, name);
    return (jlong)td;
}

jlong jniNext(JNIEnv* env, jclass, jlong cursor)
{
    return (jlong)((JniResultSet*)cursor)->next();
}

jbyte JNICALL   jniGetByte(JNIEnv* env, jclass, jlong cursor)
{
    return ((JniResultSet*)cursor)->nextByte();
}

jshort JNICALL  jniGetShort(JNIEnv* env, jclass, jlong cursor)
{
    return ((JniResultSet*)cursor)->nextShort();
}

jint JNICALL    jniGetInt(JNIEnv* env, jclass, jlong cursor)
{
    return ((JniResultSet*)cursor)->nextInt();
}

jlong JNICALL   jniGetLong(JNIEnv* env, jclass, jlong cursor)
{
    return ((JniResultSet*)cursor)->nextLong();
}


jstring JNICALL jniGetString(JNIEnv* env, jclass, jlong cursor)
{
    return ((JniResultSet*)cursor)->nextString(env);
}

jbooleanArray JNICALL jniGetBoolArray(JNIEnv* env, jclass, jlong cursor)
{
    return ((JniResultSet*)cursor)->nextBoolArray(env);
}

jcharArray JNICALL jniGetCharArray(JNIEnv* env, jclass, jlong cursor)
{
    return ((JniResultSet*)cursor)->nextCharArray(env);
}

jbyteArray JNICALL jniGetByteArray(JNIEnv* env, jclass, jlong cursor)
{
    return ((JniResultSet*)cursor)->nextByteArray(env);
}

jshortArray JNICALL jniGetShortArray(JNIEnv* env, jclass, jlong cursor)
{
    return ((JniResultSet*)cursor)->nextShortArray(env);
}

jintArray JNICALL jniGetIntArray(JNIEnv* env, jclass, jlong cursor)
{
    return ((JniResultSet*)cursor)->nextIntArray(env);
}

jlongArray JNICALL jniGetLongArray(JNIEnv* env, jclass, jlong cursor)
{
    return ((JniResultSet*)cursor)->nextLongArray(env);
}

jfloatArray JNICALL jniGetFloatArray(JNIEnv* env, jclass, jlong cursor)
{
    return ((JniResultSet*)cursor)->nextFloatArray(env);
}

jdoubleArray JNICALL jniGetDoubleArray(JNIEnv* env, jclass, jlong cursor)
{
    return ((JniResultSet*)cursor)->nextDoubleArray(env);
}

jobjectArray JNICALL jniGetStringArray(JNIEnv* env, jclass, jlong cursor)
{
    return ((JniResultSet*)cursor)->nextStringArray(env);
}

jint JNICALL    jniGetNumberOfSelectedRecords(JNIEnv* env, jclass, long cursor)
{
    return ((JniResultSet*)cursor)->size();
}

void jniCloseCursor(JNIEnv* env, jclass, jlong cursor)
{
    delete (JniResultSet*)cursor;
}

jlong JNICALL   jniInsert(JNIEnv* env, jclass, jlong db, jlong desc, jlong buf)
{
    return (jlong)((JniDatabase*)db)->insert((dbTableDescriptor*)desc, (ObjectBuffer*)buf);
}

void JNICALL   jniUpdate(JNIEnv* env, jclass, jlong db, jlong oid, jlong desc, jlong buf)
{
    ((JniDatabase*)db)->update((oid_t)oid, (dbTableDescriptor*)desc, (ObjectBuffer*)buf);
}

jlong JNICALL   jniSelect(JNIEnv* env, jclass, jlong db, jlong desc, jstring condition)
{
    char const* query = getStringBody(env, condition);
    JniResultSet* rs = ((JniDatabase*)db)->select((dbTableDescriptor*)desc, query);
    releaseStringBody(env, condition, query);
    return (jlong)rs;
}

jint JNICALL    jniDelete(JNIEnv* env, jclass, jlong db, jlong desc, jstring condition)
{
    char const* query = getStringBody(env, condition);
    int result = ((JniDatabase*)db)->remove((dbTableDescriptor*)desc, query);
    releaseStringBody(env, condition, query);
    return result;
}

void JNICALL    jniCommit(JNIEnv* env, jclass, jlong db)
{
    ((JniDatabase*)db)->commit();
}

void JNICALL    jniRollback(JNIEnv* env, jclass, jlong db)
{
    ((JniDatabase*)db)->rollback();
}

void JNICALL    jniLock(JNIEnv* env, jclass, jlong db)
{
    ((JniDatabase*)db)->lock();
}

void JNICALL    jniAttach(JNIEnv* env, jclass, jlong db)
{
    ((JniDatabase*)db)->attach();
}

void JNICALL    jniDetach(JNIEnv* env, jclass, jlong db, jint flags)
{
    ((JniDatabase*)db)->detach(flags);
}

void JNICALL    jniSetThreadContext(JNIEnv* env, jclass, jlong db, jlong ctx)
{
    ((JniDatabase*)db)->setThreadContext((dbDatabaseThreadContext*)ctx);
}

jlong JNICALL   jniGetThreadContext(JNIEnv* env, jclass, jlong db)
{
    return (jlong)((JniDatabase*)db)->getThreadContext();
}

jlong JNICALL   jniCreateTableDescriptor(JNIEnv* env, jclass, jlong db, jstring table, jobjectArray fields, jintArray types, jobjectArray referencedTables,  jobjectArray inverseFields, jintArray constraintMasks)
{
    return (jlong)((JniDatabase*)db)->createTableDescriptor(env, table, fields, types, referencedTables, inverseFields, constraintMasks);   
}

    
static JNINativeMethod fastdbApiMethods[] = 
{ 
    {    
        "jniOpen", 
        "(ILjava/lang/String;Ljava/lang/String;JI)J",
        (void*)&jniOpen
    }, 
    {    
        "jniClose", 
        "(J)V",
        (void*)&jniClose
    }, 
    {    
        "jniNextTable", 
        "(J)Ljava/lang/String;",
        (void*)&jniNextTable
    }, 
    {    
        "jniSetByte", 
        "(JB)V",
        (void*)&jniSetByte
    }, 
    {    
        "jniSetShort", 
        "(JS)V",
        (void*)&jniSetShort
    }, 
    {    
        "jniSetInt", 
        "(JI)V",
        (void*)&jniSetInt
    }, 
    {    
        "jniSetLong", 
        "(JJ)V",
        (void*)&jniSetLong
    }, 
    {    
        "jniSetString", 
        "(JLjava/lang/String;)V",
        (void*)&jniSetString
    }, 
    {    
        "jniSetBoolArray", 
        "(J[Z)V",
        (void*)&jniSetBoolArray
    }, 
    {    
        "jniSetCharArray", 
        "(J[C)V",
        (void*)&jniSetCharArray
    }, 
    {    
        "jniSetByteArray", 
        "(J[B)V",
        (void*)&jniSetByteArray
    }, 
    {    
        "jniSetShortArray", 
        "(J[S)V",
        (void*)&jniSetShortArray
    }, 
    {    
        "jniSetIntArray", 
        "(J[I)V",
        (void*)&jniSetIntArray
    }, 
    {    
        "jniSetLongArray", 
        "(J[J)V",
        (void*)&jniSetLongArray
    }, 
    {    
        "jniSetFloatArray", 
        "(J[F)V",
        (void*)&jniSetFloatArray
    }, 
    {    
        "jniSetDoubleArray", 
        "(J[D)V",
        (void*)&jniSetDoubleArray
    }, 
    {    
        "jniSetStringArray", 
        "(J[Ljava/lang/String;)V",
        (void*)&jniSetStringArray
    }, 
    {    
        "jniCreateBuffer", 
        "()J",
        (void*)&jniCreateBuffer
    }, 
    {    
        "jniCreateTable", 
        "(JLjava/lang/String;J)J",
        (void*)&jniCreateTable
    }, 
    {    
        "jniUpdateTable", 
        "(JLjava/lang/String;J)J",
        (void*)&jniUpdateTable
    }, 
    {    
        "jniNext", 
        "(J)J",
        (void*)&jniNext
    }, 
    {    
        "jniCloseCursor", 
        "(J)V",
        (void*)&jniCloseCursor
    }, 
    {    
        "jniGetByte", 
        "(J)B",
        (void*)&jniGetByte
    }, 
    {    
        "jniGetShort", 
        "(J)S",
        (void*)&jniGetShort
    }, 
    {    
        "jniGetInt", 
        "(J)I",
        (void*)&jniGetInt
    }, 
    {    
        "jniGetLong", 
        "(J)J",
        (void*)&jniGetLong
    }, 
    {    
        "jniGetString", 
        "(J)Ljava/lang/String;",
        (void*)&jniGetString
    }, 
    {    
        "jniGetBoolArray", 
        "(J)[Z",
        (void*)&jniGetBoolArray
    }, 
    {    
        "jniGetCharArray", 
        "(J)[C",
        (void*)&jniGetCharArray
    }, 
    {    
        "jniGetByteArray", 
        "(J)[B",
        (void*)&jniGetByteArray
    }, 
    {    
        "jniGetShortArray", 
        "(J)[S",
        (void*)&jniGetShortArray
    }, 
    {    
        "jniGetIntArray", 
        "(J)[I",
        (void*)&jniGetIntArray
    }, 
    {    
        "jniGetLongArray", 
        "(J)[J",
        (void*)&jniGetLongArray
    }, 
    {    
        "jniGetFloatArray", 
        "(J)[F",
        (void*)&jniGetFloatArray
    }, 
    {    
        "jniGetDoubleArray", 
        "(J)[D",
        (void*)&jniGetDoubleArray
    }, 
    {    
        "jniGetStringArray", 
        "(J)[Ljava/lang/String;",
        (void*)&jniGetStringArray
    },
    {    
        "jniInsert", 
        "(JJJ)J",
        (void*)&jniInsert
    }, 
    {    
        "jniUpdate", 
        "(JJJJ)V",
        (void*)&jniUpdate
    }, 
    {    
        "jniSelect", 
        "(JJLjava/lang/String;)J",
        (void*)&jniSelect
    }, 
    {    
        "jniGetNumberOfSelectedRecords", 
        "(J)I",
        (void*)&jniGetNumberOfSelectedRecords
    }, 
    {    
        "jniDelete", 
        "(JJLjava/lang/String;)I",
        (void*)&jniDelete
    }, 
    {    
        "jniCommit", 
        "(J)V",
        (void*)&jniCommit
    }, 
    {    
        "jniRollback", 
        "(J)V",
        (void*)&jniRollback
    }, 
    {    
        "jniLock", 
        "(J)V",
        (void*)&jniLock
    }, 
    {    
        "jniAttach", 
        "(J)V",
        (void*)&jniAttach
    }, 
    {    
        "jniDetach", 
        "(JI)V",
        (void*)&jniDetach
    }, 
    {    
        "jniGetThreadContext", 
        "(J)J",
        (void*)&jniGetThreadContext
    }, 
    {    
        "jniSetThreadContext", 
        "(JJ)V",
        (void*)&jniSetThreadContext
    }, 
    {    
        "jniCreateTableDescriptor", 
        "(JLjava/lang/String;[Ljava/lang/String;[I[Ljava/lang/String;[Ljava/lang/String;[I)J",
        (void*)&jniCreateTableDescriptor
    }
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *)
{
    JNIEnv *env;
    vm->GetEnv((void**)&env, JNI_VERSION_1_2);
    jclass cls = env->FindClass("jnicli/DatabaseJNI");
    if (cls == NULL) { 
        printf("Class jnicli/DatabaseJNI is not found\n");
    } else { 
        env->RegisterNatives(cls, fastdbApiMethods, itemsof(fastdbApiMethods));    
    }
    return JNI_VERSION_1_2;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *)
{
    JNIEnv *env;
    vm->GetEnv((void**)&env, JNI_VERSION_1_2);
    jclass cls = env->FindClass("jnicli/DatabaseJNI");
    env->UnregisterNatives(cls);
}






