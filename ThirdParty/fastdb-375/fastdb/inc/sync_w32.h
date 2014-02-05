//-< SYNC_W32.H >----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 20-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Intertask synchonization primitives
//-------------------------------------------------------------------*--------*

#ifndef __SYNC_W32_H__
#define __SYNC_W32_H__

BEGIN_FASTDB_NAMESPACE

#ifdef SET_NULL_DACL
class FASTDB_DLL_ENTRY dbNullSecurityDesciptor { 
  public:
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa; 

    dbNullSecurityDesciptor() { 
        InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = TRUE; 
        sa.lpSecurityDescriptor = &sd;
    }
    
    static dbNullSecurityDesciptor instance;
};
#define FASTDB_SECURITY_ATTRIBUTES &dbNullSecurityDesciptor::instance.sa
#else    
#define FASTDB_SECURITY_ATTRIBUTES NULL
#endif

class FASTDB_DLL_ENTRY dbMutex { 
    CRITICAL_SECTION cs;
    bool             initialized;
  public:
    dbMutex() {
        InitializeCriticalSection(&cs);
        initialized = true;
    }
    ~dbMutex() {
        DeleteCriticalSection(&cs);
        initialized = false;
    }
    bool isInitialized() { 
        return initialized;
    }
    void lock() {
        if (initialized) { 
            EnterCriticalSection(&cs);
        }
    }
    void unlock() {
        if (initialized) { 
            LeaveCriticalSection(&cs);
        }
    }
};

#define thread_proc WINAPI

class FASTDB_DLL_ENTRY dbThread { 
    HANDLE h;
  public:
    typedef void (thread_proc* thread_proc_t)(void*);

    static void sleep(time_t sec) { 
        Sleep((DWORD)(sec*1000));
    }


    void create(thread_proc_t f, void* arg) { 
        DWORD threadid;
        h = CreateThread(FASTDB_SECURITY_ATTRIBUTES, 0, LPTHREAD_START_ROUTINE(f), arg,
                         0, &threadid);
    }
    enum ThreadPriority { 
        THR_PRI_LOW, 
        THR_PRI_HIGH
    };

    void setPriority(ThreadPriority pri) { 
        SetThreadPriority(h, pri == THR_PRI_LOW ? THREAD_PRIORITY_IDLE : THREAD_PRIORITY_HIGHEST);
    }
        
    void join() { 
        WaitForSingleObject(h, INFINITE);
        CloseHandle(h);
        h = NULL;
    }
    void detach() { 
        if (h != NULL) { 
            CloseHandle(h);
            h = NULL;
        }
    }   
    dbThread() { 
        h = NULL; 
    }
    ~dbThread() { 
        if (h != NULL) { 
            CloseHandle(h);
        }
    }

    static long getCurrentThreadId() {
        return GetCurrentThreadId();
    }

    static int numberOfProcessors() { 
#ifdef PHAR_LAP
        return 1;
#else
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return sysinfo.dwNumberOfProcessors;
#endif
    }
};
    
class FASTDB_DLL_ENTRY dbProcessId { 
    DWORD tid;
  public:
    bool operator != (dbProcessId const& other) const { 
        return tid != other.tid;
    }

    void clear() { 
        tid = 0;
    }

    static dbProcessId getCurrent() {
        dbProcessId curr;
        curr.tid = GetCurrentThreadId();
        return curr;
    }
};



class FASTDB_DLL_ENTRY dbInitializationMutex { 
    HANDLE m;
  public: 
    enum initializationStatus { 
        InitializationError, 
        AlreadyInitialized,
        NotYetInitialized
    };
    initializationStatus initialize(char_t const* name) { 
        initializationStatus status;
        m = CreateMutex(FASTDB_SECURITY_ATTRIBUTES, true, name);
        if (GetLastError() == ERROR_ALREADY_EXISTS) { 
            status = WaitForSingleObject(m, INFINITE) == WAIT_OBJECT_0 
                   ? AlreadyInitialized : InitializationError;
            ReleaseMutex(m);
        } else if (m != NULL) { 
            status = NotYetInitialized;
        } else { 
            status = InitializationError;
        }
        return status;
    }
    void done() { 
        ReleaseMutex(m);
    }
    bool close() {
        CloseHandle(m);
        return false;
    }
    void erase() { 
        close();
    }
    dbInitializationMutex() { 
        m = NULL;
    }
};


const int dbMaxSemValue = 1000000;


class FASTDB_DLL_ENTRY dbSemaphore { 
  protected:
    HANDLE s;
  public:
    bool wait(unsigned msec = INFINITE) { 
        int rc = WaitForSingleObject(s, msec);
        assert(rc == WAIT_OBJECT_0 || rc == WAIT_TIMEOUT);
        return rc == WAIT_OBJECT_0;
    }
    void signal(unsigned inc = 1) {
        if (inc != 0) { 
            ReleaseSemaphore(s, inc, NULL);
        }
    }
    void reset() { 
        while (WaitForSingleObject(s, 0) == WAIT_OBJECT_0);
    }    
    bool open(char_t const* name, unsigned initValue = 0) {
        s = CreateSemaphore(FASTDB_SECURITY_ATTRIBUTES, initValue, dbMaxSemValue, name);
        return s != NULL; 
    }
    void close() {
        CloseHandle(s);
    }
    void erase() { 
        close();
    }
    dbSemaphore() { 
        s = NULL;
    }
};

class FASTDB_DLL_ENTRY dbEvent { 
  protected:
    HANDLE e;
  public:
    bool wait(unsigned msec = INFINITE) { 
        int rc = WaitForSingleObject(e, msec);
        assert(rc == WAIT_OBJECT_0 || rc == WAIT_TIMEOUT);
        return rc == WAIT_OBJECT_0;
    }
    void signal() {
        SetEvent(e);
    }
    void reset() {
        ResetEvent(e);
    }
    bool open(char_t const* name, bool signaled = false) {
        e = CreateEvent(FASTDB_SECURITY_ATTRIBUTES, true, signaled, name);
        return e != NULL; 
    }
    void close() {
        CloseHandle(e);
    }
    void erase() { 
        close();
    }
    dbEvent() { 
        e = NULL;
    }
};

class FASTDB_DLL_ENTRY dbLocalSemaphore : public dbSemaphore { 
  public:
    bool wait(dbMutex& mutex, time_t timeoutMsec) { 
        mutex.unlock();
        int rc = WaitForSingleObject(s, (DWORD)timeoutMsec);
        assert(rc == WAIT_OBJECT_0 || rc == WAIT_TIMEOUT);
        mutex.lock();
        return rc == WAIT_OBJECT_0;
    }
    void wait(dbMutex& mutex) { 
        mutex.unlock();
        int rc = WaitForSingleObject(s, INFINITE);
        assert(rc == WAIT_OBJECT_0);
        mutex.lock();
    }
    bool open(unsigned initValue = 0) {
        return dbSemaphore::open(NULL, initValue);
    }
};

class FASTDB_DLL_ENTRY dbLocalEvent : public dbEvent { 
  public:
    bool wait(dbMutex& mutex, time_t timeoutMsec) { 
        mutex.unlock();
        int rc = WaitForSingleObject(e, (DWORD)timeoutMsec);
        assert(rc == WAIT_OBJECT_0 || rc == WAIT_TIMEOUT);
        mutex.lock();
        return rc == WAIT_OBJECT_0;
    }
    void wait(dbMutex& mutex) { 
        mutex.unlock();
        int rc = WaitForSingleObject(e, INFINITE);
        assert(rc == WAIT_OBJECT_0);
        mutex.lock();
    }
    bool open(bool signaled = false) {
         return dbEvent::open(NULL, signaled);
     }
};

class FASTDB_DLL_ENTRY dbWatchDog { 
  public:
    bool watch() { 
        return WaitForSingleObject(mutex, INFINITE) == ERROR_WAIT_NO_CHILDREN;
    }
    void close() { 
        CloseHandle(mutex);
    }
    bool create(char_t const* name) {
        mutex = CreateMutex(FASTDB_SECURITY_ATTRIBUTES, true, name);
        return mutex != NULL;
    }
    bool open(char_t const* name) {
#if defined(_WINCE) || defined(UNICODE)
        return create(name);
#else
        mutex = OpenMutex(MUTEX_ALL_ACCESS, false, name);
        return mutex != NULL;
#endif
    }
    HANDLE mutex;
};

template<class T>
class dbThreadContext { 
    unsigned int index;
  public:
    T* get() { 
        return (T*)TlsGetValue(index);
    }
    void set(T* value) { 
        TlsSetValue(index, value);
    }
    dbThreadContext() { 
        index = TlsAlloc();
        assert(index != TLS_OUT_OF_INDEXES);
    }
    ~dbThreadContext() { 
        TlsFree(index);
    }
};

template<class T>
class dbSharedObject { 
    T*     ptr;
    HANDLE h;
  public:

    bool open(char_t* name) { 
#ifdef NO_MMAP
        ptr = new T();
#else
        h = CreateFileMapping(INVALID_HANDLE_VALUE,
                              FASTDB_SECURITY_ATTRIBUTES, PAGE_READWRITE, 0, 
                              sizeof(T), name);
        if (h == NULL) { 
            return false;
        }
        ptr = (T*)MapViewOfFile(h, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (ptr == NULL) { 
            CloseHandle(h);
            return false;
        }
#endif
        return true;
    }

    T* get() { return ptr; }

    void close() { 
#ifdef NO_MMAP
        delete[] ptr;
#else
        UnmapViewOfFile(ptr);
        CloseHandle(h);
#endif
    }
    void erase() { 
        close();
    }
    dbSharedObject() { 
        ptr = NULL;
        h = NULL;
    }
};

typedef long sharedsem_t;

#ifdef RECOVERABLE_CRITICAL_SECTION
class FASTDB_DLL_ENTRY dbGlobalCriticalSection { 
    HANDLE mutex;
  public:
    void enter() { 
        int rc = WaitForSingleObject(mutex, INFINITE);
        assert (rc == WAIT_OBJECT_0 || rc == WAIT_ABANDONED);
    }

    void leave() {
        ReleaseMutex(mutex);
    }

    bool create(char_t const* name, long* count) { 
        mutex = CreateMutex(FASTDB_SECURITY_ATTRIBUTES, false, name);
        return mutex != NULL;
    }

    bool open(char_t const* name, long* count) { 
        mutex = OpenMutex(MUTEX_ALL_ACCESS, true, name);
        return mutex != NULL;
    }

    void close() { 
        CloseHandle(mutex);
    }
    void erase() { 
        close();
    }
    dbGlobalCriticalSection() {
        mutex = NULL;
    }
        
};
#else
class FASTDB_DLL_ENTRY dbGlobalCriticalSection { 
    HANDLE       event;
    sharedsem_t* count;

  public:
    void enter() { 
        if (InterlockedDecrement(count) != 0) { 
            // another process is in critical section
            int rc = WaitForSingleObject(event, INFINITE);
            assert (rc == WAIT_OBJECT_0);
        }
    }

    void leave() { 
        if (InterlockedIncrement(count) <= 0) { 
            // some other processes try to enter critical section
            SetEvent(event);
        }
    }

    bool open(char_t const* name, long* count) { 
        this->count = count;
        event = OpenEvent(EVENT_ALL_ACCESS, FALSE, name);
        return event != NULL;
    }
    bool create(char_t const* name, long* count) { 
        this->count = count;
        *count = 1;
        event = CreateEvent(FASTDB_SECURITY_ATTRIBUTES, false, false, name);
        return event != NULL;
    }
    void close() { 
        CloseHandle(event);
    }
    void erase() { 
        close();
    }
    dbGlobalCriticalSection() {
        event = NULL;
    }
        
};
#endif

END_FASTDB_NAMESPACE

#endif //__SYNC_W32_H__
