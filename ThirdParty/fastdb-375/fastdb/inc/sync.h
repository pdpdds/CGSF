//-< SYNC.H >--------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 20-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Intertask synchonization primitives
//-------------------------------------------------------------------*--------*

#ifndef __SYNC_H__
#define __SYNC_H__

#ifdef _WIN32
#include "sync_w32.h"    
#else // Unix
#include "sync_unix.h"
#endif

BEGIN_FASTDB_NAMESPACE

class FASTDB_DLL_ENTRY dbSystem { 
  public:
    static unsigned getCurrentTimeMsec();
};

// Common decls for all platforms
class FASTDB_DLL_ENTRY dbCriticalSection { 
  private:
    dbMutex& mutex;
  public:
    dbCriticalSection(dbMutex& guard) : mutex(guard) {
        mutex.lock();
    }
    ~dbCriticalSection() { 
        mutex.unlock();
    }
};
        
#define SMALL_BUF_SIZE 512

class FASTDB_DLL_ENTRY dbSmallBuffer { 
  protected:
    char   smallBuf[SMALL_BUF_SIZE];
    char*  buf;
    size_t used;

  public:
    dbSmallBuffer(size_t size) { 
        if (size > SMALL_BUF_SIZE) { 
            buf = new char[size];
        } else { 
            buf = smallBuf;
        }
        used = size;
    }

    dbSmallBuffer() { 
        used = 0;
        buf = smallBuf;
    }

    void put(size_t size) { 
        if (size > SMALL_BUF_SIZE && size > used) { 
            if (buf != smallBuf) { 
                delete[] buf;
            }
            buf = new char[size];
            used = size;
        }
    }

    operator char*() { return buf; }
    char* base() { return buf; }

    ~dbSmallBuffer() { 
        if (buf != smallBuf) { 
            delete[] buf;
        }
    }
};

class dbThreadPool;

class FASTDB_DLL_ENTRY dbPooledThread { 
  private:
    friend class dbThreadPool;

    dbThread                thread;
    dbThreadPool*           pool;
    dbPooledThread*         next;
    dbThread::thread_proc_t f;
    void*                   arg;
    bool                    running;
    dbLocalSemaphore        startSem;
    dbLocalSemaphore        readySem;
    
    static void thread_proc  pooledThreadFunc(void* arg);

    void run();
    void stop();

    dbPooledThread(dbThreadPool* threadPool); 
    ~dbPooledThread(); 
};

class FASTDB_DLL_ENTRY dbThreadPool { 
    friend class dbPooledThread;
    dbPooledThread* freeThreads;
    dbMutex         mutex;

  public:
    dbPooledThread* create(dbThread::thread_proc_t f, void* arg);
    void join(dbPooledThread* thr);
    dbThreadPool();
    ~dbThreadPool();
};    

END_FASTDB_NAMESPACE
    
#endif // __SYNC_H__


