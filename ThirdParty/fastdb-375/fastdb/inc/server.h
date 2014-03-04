//-< SERVER.CPP >----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     13-Jan-2000  K.A. Knizhnik  * / [] \ *
//                          Last update: 13-Jan-2000  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// CLI multithreaded server class
//-------------------------------------------------------------------*--------*

#ifndef __SERVER_H__
#define __SERVER_H__

#include "sockio.h"

BEGIN_FASTDB_NAMESPACE

class dbColumnBinding { 
  public:
    dbColumnBinding*   next;
    dbFieldDescriptor* fd;
    int                cliType;
    int                len;
    char*              ptr;

    int  unpackArray(char* dst, size_t& offs);
    void unpackScalar(char* dst, bool insert);

    dbColumnBinding(dbFieldDescriptor* field, int type) { 
        fd = field;
        cliType = type;
        next = NULL;
    }
};

struct dbParameterBinding { 
    union { 
        int1       i1;
        int2       i2;
        int4       i4;
        db_int8    i8;
        real4      r4;
        real8      r8;
        oid_t      oid;
        bool       b;
        char*      str;
        rectangle  rect;
    } u;
    int type;
};

const int dbQueryMaxIdLength = 256;

class dbQueryScanner { 
  public:
    char*    p;
    db_int8     ival;
    real8    fval;
    char     buf[dbQueryMaxIdLength];
    char*    ident;

    int  get();

    void reset(char* stmt) { 
        p = stmt;
    }
};
    
class dbStatement { 
  public:
    int                 id;
    bool                firstFetch;
    dbStatement*        next;
    dbAnyCursor*        cursor; 
    dbQuery             query;
    dbColumnBinding*    columns;
    char*               buf;
    int                 buf_size;
    int                 n_params;
    int                 n_columns;
    dbParameterBinding* params;
    dbTableDescriptor*  table;
    
    void reset();

    dbStatement(int stmt_id) { 
        id = stmt_id;
        columns = NULL;
        params = NULL;
        buf = NULL;
        buf_size = 0;
        table = NULL;
        cursor = NULL;
    }
    ~dbStatement() { 
        reset(); 
        delete[] buf;
    }
};

class dbSession { 
  public:
    dbSession*         next;  
    dbStatement*       stmts;
    dbQueryScanner     scanner;
    socket_t*          sock;
    bool               in_transaction;
    dbTableDescriptor* dropped_tables;
    dbTableDescriptor* existed_tables;
};

class FASTDB_DLL_ENTRY dbServer {     
  protected:
    static dbServer* chain;
    dbServer*        next;
    char*            URL;
    dbSession*       freeList;
    dbSession*       waitList;
    dbSession*       activeList;
    int              optimalNumberOfThreads;
    int              nActiveThreads;
    int              nIdleThreads;
    int              waitListLength;
    bool             cancelWait;
    bool             cancelAccept;
    bool             cancelSession;
    dbMutex          mutex;
    dbLocalSemaphore go;
    dbLocalSemaphore done;
    socket_t*        globalAcceptSock;
    socket_t*        localAcceptSock;
    dbThread         localAcceptThread;
    dbThread         globalAcceptThread;
    dbDatabase*      db;

    static void thread_proc serverThread(void* arg);
    static void thread_proc acceptLocalThread(void* arg);
    static void thread_proc acceptGlobalThread(void* arg);

    void serveClient();
    void acceptConnection(socket_t* sock);
    
    
    bool freeze(dbSession* session, int stmt_id);
    bool unfreeze(dbSession* session, int stmt_id);
    bool get_first(dbSession* session, int stmt_id);
    bool get_last(dbSession* session, int stmt_id);
    bool get_next(dbSession* session, int stmt_id);
    bool get_prev(dbSession* session, int stmt_id);
    bool seek(dbSession* session, int stmt_id, char* buf);
    bool skip(dbSession* session, int stmt_id, char* buf);
    bool fetch(dbSession* session, dbStatement* stmt, oid_t result);
    bool fetch(dbSession* session, dbStatement* stmt) { 
        return fetch(session, stmt, stmt->cursor->currId);
    }
    bool remove(dbSession* session, int stmt_id);
    bool remove_current(dbSession* session, int stmt_id);
    bool update(dbSession* session, int stmt_id, char* new_data);
    bool insert(dbSession* session, int stmt_id, char* data, bool prepare);
    bool select(dbSession* session, int stmt_id, char* data, bool prepare);
    bool show_tables(dbSession* session); 
    bool describe_table(dbSession* session, char const* table);
    bool create_table(dbSession* session, char* data, bool create);
    bool drop_table(dbSession* session, char* data);
    bool alter_index(dbSession* session, char* data);

    char* checkColumns(dbStatement* stmt, int n_columns, 
                       dbTableDescriptor* desc, char* data, 
                       int4& reponse, bool select);
      
    dbStatement* findStatement(dbSession* stmt, int stmt_id);

  public:
    static dbServer* find(char const* serverURL);
    static void      cleanup();

    void stop();
    void start();

    dbServer(dbDatabase* db,
             char const* serverURL, 
             int optimalNumberOfThreads = 8,  
             int connectionQueueLen = 64);
    ~dbServer();
};

END_FASTDB_NAMESPACE

#endif
