//-< WWWAPI.H >------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     27-Mar-99    K.A. Knizhnik  * / [] \ *
//                          Last update:  1-Jul-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// API for creating Internet applications 
//-------------------------------------------------------------------*--------*

#ifndef __WWWAPI_H__
#define __WWWAPI_H__

#include "stdtp.h"
#include "sync.h"
#include "sockio.h"
#include "database.h"

BEGIN_FASTDB_NAMESPACE

enum WWWencodingType { 
    TAG  = 0, // HTML tags (no conversion)
    HTML = 1, // replace ('<','>','"','&') with (&lt; &gt; &amp; &qout;)
    URL  = 2  // replace spaces with '+', and other special characters with %XX
};
//
// Automatic state shifts after each append operation:
//   TAG->HTML
//   HTML->TAG
//   URL->TAG
//

class FASTDB_DLL_ENTRY WWWconnection {  
    friend class WWWapi;
    friend class CGIapi;
    friend class QueueManager;
    friend class HTTPapi;
    
  public:
    
    typedef void (*UserDataDestructor)(void* userData);
    typedef bool (*handler)(WWWconnection& con);

    void*              userData;
    UserDataDestructor userDataDestructor;

    void setUserData(void* data, UserDataDestructor destructor = NULL) { 
        userData = data;
        userDataDestructor = destructor;
    }

    //
    // Append string to reply buffer
    //
    WWWconnection& append(char const* str);
    WWWconnection& append(wchar_t const* str);
    //
    // Append binary data
    // 
    WWWconnection& append(const void *buf, int len);
    
    WWWconnection& operator << (char const* str) { 
        return append(str);
    }
     WWWconnection& operator << (wchar_t const* str) { 
        return append(str);
    }
   
    void setEncoding(WWWencodingType type) { encoding = type; }

    WWWconnection& operator << (WWWencodingType type) { 
        setEncoding(type);
        return *this;
    }
    WWWconnection& operator << (int value) { 
        char buf[32];
        sprintf(buf, "%d", value);
        return append(buf);
    }
    WWWconnection& operator << (double value) {
        char buf[32];
        sprintf(buf, "%f", value);
        return append(buf);
    }
  
    WWWconnection& operator << (db_int8 value) {
        char buf[32];
        sprintf(buf, INT8_FORMAT, value);
        return append(buf);
    }
    
    WWWconnection& operator << (oid_t value) {        
        char buf[32];   
        sprintf(buf, "%ld", (long)value);
        return append(buf);
    }

    char* getStub() { return stub; }

    char* getAddress() { return address; }

    char* getPeer() { return peer; }

    //
    // Compare content of the string with the end of the reply buffer
    //
    bool terminatedBy(char const* str) const;

    //
    // Get value of variable from request string. If name is not present in 
    // string NULL is returned. Parameter 'n' can be used to get n-th
    // value of variable for multiple selection slot. Zero value of n 
    // corresponds to the first variable's value, 1 - to the second,...
    // When no more values are available NULL is returned.
    //
    char* get(char const* name, int n = 0);
    
    //
    // Associatte value with name
    //
    void addPair(char const* name, char const* value);
    
    WWWconnection();
    ~WWWconnection();

  protected: 
    enum { hash_table_size = 1013 };
    socket_t*   sock;
    char*       reply_buf;
    size_t      reply_buf_size;
    size_t      reply_buf_used;
    char*       stub;
    char*       address;
    char*       peer;
    WWWconnection*  next;
    WWWencodingType encoding;
   

    struct name_value_pair { 
        name_value_pair* next;
        char const*      name;
        char const*      value;
        unsigned         hash_code;
    };

    name_value_pair* hash_table[hash_table_size];
    name_value_pair* free_pairs;

    char* extendBuffer(size_t inc);


    //
    // Deallocate all resources hold by connection. It is not possible to 
    // call get_value() or reply() method after this. Method reset()
    // is implicitly called by WWWapi::get() method.
    //
    void reset();

    //
    // Unpack requests paramters
    //
    char* unpack(char* body, size_t body_length);
};


class FASTDB_DLL_ENTRY WWWapi { 
  public:
    struct dispatcher { 
        char const*         page;
        WWWconnection::handler func;
        // filled by contracutor of WWWapi
        unsigned            hash_code;
        dispatcher*         collision_chain;
    };

  protected:
    socket_t*   sock;
    bool        canceled;
    char*       address;
    dbDatabase& db;
    enum { hash_table_size = 113  };
    dispatcher* hash_table[hash_table_size];

    bool dispatch(WWWconnection& con, char* page);

  public:
    WWWapi(dbDatabase& db, int n_handlers, dispatcher* dispatch_table);
    virtual~WWWapi();

    //
    // Bind and listen socket
    //
    bool open(char const* socket_address = "localhost:80", 
              socket_t::socket_domain domain = socket_t::sock_global_domain, 
              int listen_queue = DEFAULT_LISTEN_QUEUE_SIZE);


    //
    // Read and execute requests
    //
    virtual bool serve(WWWconnection& con) = 0;

    //
    // Accept new connection by the socket
    //
    bool connect(WWWconnection& con);

    //
    // Cancel acception of connections
    // 
    void cancel();

    //
    // Close socket
    // 
    void close();
};


//
// Interaction with WWW server by means of CGI protocol and CGIatub program
//
class FASTDB_DLL_ENTRY CGIapi : public WWWapi { 
  public:
    virtual bool serve(WWWconnection& con);

    CGIapi(dbDatabase& db, int n_handlers, dispatcher* dispatch_table) 
    : WWWapi(db, n_handlers, dispatch_table) {}
};

    
// 
// Built-in implementation of sunset of subset of HTTP protocol
//
class FASTDB_DLL_ENTRY HTTPapi : public WWWapi { 
  protected:
    time_t connectionHoldTimeout;
    bool   keepConnectionAlive;

    bool handleRequest(WWWconnection& con, char* begin, char* end, 
                       char* host, bool& result);

  public:
    virtual bool serve(WWWconnection& con);

    HTTPapi(dbDatabase& db, int n_handlers, dispatcher* dispatch_table, 
            bool persistentConnections = false,
            time_t connectionHoldTimeoutSec = WAIT_FOREVER) 
    : WWWapi(db, n_handlers, dispatch_table) 
    {
        keepConnectionAlive = persistentConnections;
        connectionHoldTimeout = connectionHoldTimeoutSec;
    }
};

class FASTDB_DLL_ENTRY QueueManager { 
    WWWconnection*   connectionPool;
    WWWconnection*   freeList;
    WWWconnection*   waitList;
    dbMutex          mutex;
    dbLocalSemaphore go;
    dbLocalEvent     done;
    dbThread*        threads;
    int              nThreads;
    WWWapi*          server;
    dbDatabase&      db;

    static void thread_proc handleThread(void* arg);
    void handle();
    
  public:
    void stop();
    void start();

    QueueManager(WWWapi& api, // WWWapi should be opened
                 dbDatabase& db,
                 int     nThreads = 8, 
                 int     connectionQueueLen = 64);
    ~QueueManager();
};


END_FASTDB_NAMESPACE

#endif



