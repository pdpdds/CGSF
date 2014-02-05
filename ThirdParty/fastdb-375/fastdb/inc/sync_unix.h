//-< SYNC_UNIX.H >---------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 20-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Intertask synchonization primitives for Unix platforms
//-------------------------------------------------------------------*--------*

#ifndef __SYNC_UNIX_H__
#define __SYNC_UNIX_H__

// Standard includes for all Unix platforms
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#ifdef VXWORKS
#include "fastdbShim.h"
#else
#include <sys/time.h>
#endif // VXWORKS
#include <sys/types.h>
#include <assert.h>
#include <errno.h>

#if !defined(USE_POSIX_SEMAPHORES) || !defined(USE_POSIX_MMAP) || !USE_POSIX_MMAP
#ifndef VXWORKS
#include <sys/ipc.h> 
#endif // ndef VXWORKS
extern char const* keyFileDir; // default value: "/tmp/" 
#endif

#if defined(USE_POSIX_SEMAPHORES)
#include <semaphore.h>  // For POSIX style semaphores
#else
#include <sys/sem.h>    // For SysV style semaphores
#endif

#if defined(USE_POSIX_MMAP) && USE_POSIX_MMAP
#include <sys/mman.h>   // For mmap()
#else
#ifndef VXWORKS
#include <sys/shm.h>    
#endif // ndef VXWORKS
#include <sys/mman.h>
#endif

BEGIN_FASTDB_NAMESPACE

#define thread_proc

//////////////////////////////////////////////////////////
// If this system uses pthread based threads, then define
//   dbMutex(), dbThread(), dbLocalEvent(), etc as pthread-based implemenations

#ifndef NO_PTHREADS

// Use pthread based implementation
END_FASTDB_NAMESPACE
#include <pthread.h>
BEGIN_FASTDB_NAMESPACE

class dbMutex { 
    friend class dbLocalEvent;
    friend class dbLocalSemaphore;
    pthread_mutex_t cs;
    bool            initialized;
  public:
    dbMutex() {
#ifdef NDEBUG
        pthread_mutex_init(&cs, NULL);
#else
#ifdef VXWORKS
	memset(&cs, '\0', sizeof(cs));
#endif // VXWORKS
        int rc = pthread_mutex_init(&cs, NULL);
        assert(rc == 0);
#endif
        initialized = true;
    }
    ~dbMutex() {
#ifdef NDEBUG
        pthread_mutex_destroy(&cs);
#else
        int rc = pthread_mutex_destroy(&cs);
        assert(rc == 0);
#endif
        initialized = false;
    }
    bool isInitialized() { 
        return initialized;
    }
    void lock() {
        if (initialized) { 
#ifdef NDEBUG
            pthread_mutex_lock(&cs);
#else
            int rc = pthread_mutex_lock(&cs);
            assert(rc == 0);
#endif
        }
    }
    void unlock() {
        if (initialized) { 
#ifdef NDEBUG
            pthread_mutex_unlock(&cs);
#else
            int rc = pthread_mutex_unlock(&cs);
            assert(rc == 0);
#endif
        }
    }
};


const size_t dbThreadStackSize = 1024*1024;

class dbThread { 
    pthread_t thread;
  public:
    typedef void (thread_proc* thread_proc_t)(void*);
    
    static void sleep(time_t sec) { 
        ::sleep(sec);
    }

    void create(thread_proc_t f, void* arg) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
#if !defined(__linux__)
        pthread_attr_setstacksize(&attr, dbThreadStackSize);
#endif
#if defined(_AIX41)
        // At AIX 4.1, 4.2 threads are by default created detached
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_UNDETACHED);
#endif
        pthread_create(&thread, &attr, (void*(*)(void*))f, arg);
        pthread_attr_destroy(&attr);
    }

    void join() { 
        void* result;
        pthread_join(thread, &result);
    }
    void detach() { 
        pthread_detach(thread);
    }

    enum ThreadPriority { 
        THR_PRI_LOW, 
        THR_PRI_HIGH
    };
    void setPriority(ThreadPriority pri) { 
#if defined(PRI_OTHER_MIN) && defined(PRI_OTHER_MAX)
        struct sched_param sp;
        sp.sched_priority = pri == THR_PRI_LOW ? PRI_OTHER_MIN : PRI_OTHER_MAX;
        pthread_setschedparam(thread, SCHED_OTHER, &sp); 
#endif
    }

    static long getCurrentThreadId() {
        return (long)pthread_self();
    }

    static int numberOfProcessors();
};


class dbLocalEvent { 
    pthread_cond_t   cond;
    int              signaled;
  public:
    void wait(dbMutex& mutex) { 
        while (!signaled) { 
            pthread_cond_wait(&cond, &mutex.cs);
        }
    }
    bool wait(dbMutex& mutex, time_t timeout) {
        if (!signaled) { 
            struct timespec abs_ts; 
#ifdef PTHREAD_GET_EXPIRATION_NP
            struct timespec rel_ts; 
            rel_ts.tv_sec = timeout/1000; 
            rel_ts.tv_nsec = timeout%1000*1000000;
            pthread_get_expiration_np(&rel_ts, &abs_ts);
#else
            struct timeval cur_tv;
            gettimeofday(&cur_tv, NULL);
            abs_ts.tv_sec = cur_tv.tv_sec + timeout/1000; 
            abs_ts.tv_nsec = cur_tv.tv_usec*1000 + timeout%1000*1000000;
            if (abs_ts.tv_nsec > 1000000000) { 
                abs_ts.tv_nsec -= 1000000000;
                abs_ts.tv_sec += 1;
            }
#endif
            do { 
                int rc = pthread_cond_timedwait(&cond, &mutex.cs, &abs_ts);
                if (rc != 0) {
                    return false;
                }
            } while (!signaled);

        }
        return true;
    }
    void signal() {
        signaled = true;
        pthread_cond_broadcast(&cond);
    }
    void reset() {
        signaled = false;
    }
    void open(bool initValue = false) { 
        signaled = initValue;
        pthread_cond_init(&cond, NULL);
    }
    void close() {
        pthread_cond_destroy(&cond);
    }
};

class dbLocalSemaphore { 
    pthread_cond_t   cond;
    int              count;
  public:
    void wait(dbMutex& mutex) { 
        while (count == 0) { 
            pthread_cond_wait(&cond, &mutex.cs);
        }
        count -= 1;
    }
    bool wait(dbMutex& mutex, time_t timeout) {
        if (count == 0) { 
            struct timespec abs_ts; 
#ifdef PTHREAD_GET_EXPIRATION_NP
            struct timespec rel_ts; 
            rel_ts.tv_sec = timeout/1000; 
            rel_ts.tv_nsec = timeout%1000*1000000;
            pthread_get_expiration_np(&rel_ts, &abs_ts);
#else
            struct timeval cur_tv;
            gettimeofday(&cur_tv, NULL);
            abs_ts.tv_sec = cur_tv.tv_sec + timeout/1000; 
            abs_ts.tv_nsec = cur_tv.tv_usec*1000 + timeout%1000*1000000;
            if (abs_ts.tv_nsec > 1000000000) { 
                abs_ts.tv_nsec -= 1000000000;
                abs_ts.tv_sec += 1;
            }
#endif
            do { 
                int rc = pthread_cond_timedwait(&cond, &mutex.cs, &abs_ts);
                if (rc != 0) { 
                    return false;
                }
            } while (count == 0);
        }
        count -= 1;
        return true;
    }
    void signal(unsigned inc = 1) {
        count += inc;
        if (inc > 1) { 
            pthread_cond_broadcast(&cond);
        } else if (inc == 1) { 
            pthread_cond_signal(&cond);
        }
    }
    void open(unsigned initValue = 0) { 
        pthread_cond_init(&cond, NULL);
        count = initValue;
    }
    void close() {
        pthread_cond_destroy(&cond);
    }
};

template<class T> 
class dbThreadContext { 
    pthread_key_t key;
  public:
    T* get() { 
        T* p_val = (T*)pthread_getspecific(key);
#ifdef VXWORKS
	// HACK - on VXWORKS, setting to NULL releases the key for _all_ threads.
	// when windriver defect number WIND00119383 is fixed - this hack may be
	// removed.
	if (p_val == (T*)-1)
	    p_val = (T*)NULL;
#endif // VXWORKS
	return p_val;
    }
    void set(T* value) { 
#ifdef VXWORKS
	// HACK - on VXWORKS, setting to NULL releases the key for _all_ threads.
	// when windriver defect number WIND00119383 is fixed - this hack may be
	// removed.
	if (value == NULL)
	    value = (T*)-1;
#endif // VXWORKS
        pthread_setspecific(key, value);
    }
    dbThreadContext() { 
        pthread_key_create(&key, NULL);
    }
    ~dbThreadContext() { 
        pthread_key_delete(key);
    }
};

class dbProcessId { 
    int       pid;
    pthread_t tid;
  public:
    bool operator != (dbProcessId const& other) const { 
        return pid != other.pid || tid != other.tid;
    }

    void clear() { 
        pid = 0;
        tid = 0;
    }

    static dbProcessId getCurrent() {
        dbProcessId curr;
        curr.pid = getpid();
        curr.tid = pthread_self();
        return curr;
    }
};

#else // NO_PTHREAD

// Non pthread based threads, mutexes, etc.
// Maps to skeleton  functions, this implementation isn't using threads.

class dbMutex {
    bool initialized;

   public:
    dbMutex() {
        initialized = true;
    }

    ~dbMutex() { 
        initialized = false;
    }

    bool isInitialized() { 
        return initialized;
    }

    void lock() {}
    void unlock() {}
};

class dbThread { 
  public:
    typedef void (thread_proc* thread_proc_t)(void*);
    void create(thread_proc_t f, void* arg) { f(arg); }
    void join() {}
    void detach() {}
    enum ThreadPriority { 
        THR_PRI_LOW, 
        THR_PRI_HIGH
    };
    void setPriority(ThreadPriority pri) { }
    static int numberOfProcessors() { return 1; }
};

class dbLocalSemaphore { 
    int count;
  public:
    void wait(dbMutex&) { 
        assert (count > 0);
        count -= 1;
    }
    void signal(unsigned inc = 1) {
        count += inc;
    }
    void open(unsigned initValue = 0) {
        count = initValue;
    }
    void close() {}
};

class dbLocalEvent { 
    bool signaled;
  public:
    void wait(dbMutex&) { 
        assert(signaled);
    }
    bool wait(dbMutex& mutex, time_t timeout) {
        return true;
    }
    void signal() {
        signaled = true;
    }
    void reset() {
        signaled = false;
    }
    void open(bool initValue = false) {
        signaled = initValue;
    }
    void close() {}
};

template<class T>
class dbThreadContext { 
    T* value;
  public:
    T* get() { 
        return value;
    }
    void set(T* value) { 
        this->value = value;
    }
    dbThreadContext() { value = NULL; }
};


class dbProcessId { 
    int       pid;
  public:
    bool operator != (dbProcessId const& other) const { 
        return pid != other.pid;
    }
    
    void clear() { 
        pid = 0;
    }

    static dbProcessId getCurrent() {
        dbProcessId curr;
        curr.pid = getpid();
        return curr;
    }
};

#endif // NO_PTHREAD


#define INFINITE (~0U)


#ifdef USE_POSIX_SEMAPHORES

// Initialization Mutex using Posix based semaphores
class dbInitializationMutex { 
    sem_t* sem;
    char*  name;
  public: 
    enum initializationStatus { 
        InitializationError, 
        AlreadyInitialized,
        NotYetInitialized
    };
    initializationStatus initialize(char const* name) { 
        initializationStatus status;
        this->name = new char[strlen(name)+2];
        if (*name != '/') { 
            strcpy(this->name+1, name);
            *this->name = '/';
        } else { 
            strcpy(this->name, name);
        }
        while (true) {
            sem = sem_open(this->name, 0);
            if (sem == SEM_FAILED) { 
                if (errno == ENOENT) {
                    sem = sem_open(this->name, O_CREAT|O_EXCL, 0777, 0);
                    if (sem != SEM_FAILED) { 
                        status = NotYetInitialized;
                        break;
                    } else if (errno != EEXIST) { 
                        status = InitializationError;
                        break;
                    }
                } else { 
                    status = InitializationError;
                    break;
                }
            } else { 
                status = (sem_wait(sem) == 0 && sem_post(sem) == 0) 
                    ? AlreadyInitialized : InitializationError;
                break;
            }
        }
        return status;
    }

    void done() { 
        sem_post(sem);
    }
    bool close() {
        sem_close(sem);
        return true;
    }
    void erase() {
        sem_unlink(name);
        delete[] name;
    }
};

class dbSemaphore { 
  protected:
    sem_t* sem;
    char*  name;
  public:
    void wait() { 
#ifdef NDEBUG
        sem_wait(sem);
#else
        int rc = sem_wait(sem);
        assert(rc == 0);
#endif
    }

    bool wait(unsigned msec) { 
#ifdef POSIX_1003_1d
        struct timespec abs_ts;
        struct timeval  cur_tv;
        clock_gettime(CLOCK_REALTIME, &cur_tv);
        abs_ts.tv_sec = cur_tv.tv_sec + (msec + cur_tv.tv_usec / 1000) / 1000000; 
        abs_ts.tv_nsec = (msec + cur_tv.tv_usec / 1000) % 1000000 * 1000;
        int rc = sem_timedwait(sem, &abs_ts);
        if (rc < 0) { 
            assert(errno == ETIMEDOUT);
            return false;
        }
        return true;
#else 
#ifdef NDEBUG
        sem_wait(sem);
#else
        int rc = sem_wait(sem);
        assert(rc == 0);
#endif
        return true;
#endif  
    }

    void signal(unsigned inc = 1) {
        while (inc-- > 0) { 
            sem_post(sem);
        }
    }
    void reset() { 
        while (sem_trywait(sem) == 0);
    }    
    bool open(char const* name, unsigned initValue = 0) {
        this->name = new char[strlen(name)+2];
        if (*name != '/') { 
            strcpy(this->name+1, name);
            *this->name = '/';
        } else { 
            strcpy(this->name, name);
        }
        sem = sem_open(this->name, O_CREAT, 0777, initValue);
        return sem != NULL; 
    }
    void close() {
        if (sem != NULL) { 
            sem_close(sem);
            sem = NULL;
        }
    }
    void erase() { 
        close();
        sem_unlink(name);
        delete[] name;
    }
};

class dbEvent : public dbSemaphore { 
  public:
    void wait() { 
        dbSemaphore::wait();
        sem_post(sem);
    }
    bool wait(unsigned msec) { 
        if (dbSemaphore::wait(msec)) { 
            sem_post(sem);
            return true;
        }
        return false;
    }
    void signal() {
        while (sem_trywait(sem) == 0);
        sem_post(sem);
    }
    void reset() {
        while (sem_trywait(sem) == 0);
    }
    bool open(char const* name, bool signaled = false) {
        return dbSemaphore::open(name, (int)signaled);
    }
};
#else // USE_POSIX_SEMAPHORES

class FASTDB_DLL_ENTRY dbWatchDog { 
    bool open(char const* name, int flags);
  public:
    bool watch();
    void close(); 
    bool open(char const* name);
    bool create(char const* name);
    int id;
};

// Define local implemenation of InitializationMutex in sync.cpp
class dbInitializationMutex { 
    int semid;
  public: 
    enum initializationStatus { 
        InitializationError, 
        AlreadyInitialized,
        NotYetInitialized
    };
    initializationStatus initialize(char const* name);
    void done(); 
    bool close();
    void erase();
};


class dbSemaphore { 
    int s;
  public:
    bool wait(unsigned msec = INFINITE);
    void signal(unsigned inc = 1);
    bool open(char const* name, unsigned initValue = 0);
    void reset();
    void close();
    void erase();
};

class dbEvent { 
    int e;
  public:
    bool wait(unsigned msec = INFINITE);
    void signal();
    void reset();
    bool open(char const* name, bool signaled = false);
    void close();
    void erase();
};
#endif // USE_POSIX_SEMAPHORES


// Define dbSharedObject and dbSharedMemory
#if defined(USE_POSIX_MMAP) && USE_POSIX_MMAP

// For POSIX dbSharedObject, we use mmap()
template<class T>
class dbSharedObject { 
    char* name;
    T*  ptr;
    int fd;
  public:

    dbSharedObject() { 
        name = NULL;
        ptr = NULL;
        fd = -1;
    }

    bool open(char* fileName) { 
        delete[] name;
        name = new char[strlen(fileName) + 1];
        strcpy(name, fileName);
        fd = ::open(fileName, O_RDWR|O_CREAT, 0777);
        if (fd < 0) { 
            return false;
        }
        if (ftruncate(fd, sizeof(T)) < 0) {
            ::close(fd);
            return false;
        }
        ptr = (T*)mmap(NULL,
                       DOALIGN(sizeof(T), 4096),
                       PROT_READ|PROT_WRITE,
                       MAP_SHARED,
                       fd,
                       0);
        if (ptr == MAP_FAILED) { 
            ptr = NULL;
            ::close(fd);
            return false;
        }
        return true;
    }

    T* get() { return ptr; }

    void close() { 
        if (ptr != NULL) { 
            munmap((char*)ptr, DOALIGN(sizeof(T), 4096));
        }
        if (fd > 0) { 
            ::close(fd);
        }
    }
    void erase() {
        close();
        unlink(name);   
    }  

    ~dbSharedObject() { 
        delete[] name;
    }
};

#else // USE_POSIX_MMAP

// Non POSIX, internal implementations of SharedMemory and SharedObject
extern char const* keyFileDir; // default value: "/tmp/" 
class dbSharedMemory { 
  protected:
    char*  ptr;
    int    shm;

  public:
    bool  open(char const* name, size_t size); 
    void  close();
    void  erase(); 
    char* get_base() { 
        return ptr;
    }
};

template<class T>
class dbSharedObject : public dbSharedMemory { 
  public:
    bool open(char* name) { 
        return dbSharedMemory::open(name, sizeof(T));
    }
    T* get() { return (T*)ptr; }
};

#endif

//////////////////////////////////////////////////////////////////////////
// Define dBGlobalCriticalSection for various platforms

// QNX uses a pthread based mutex for its implementation
//     Use only if pthread support is also enabled, else we'll use the default case
#if !defined(NO_PTHREADS) && defined(USE_SHARED_PTHREAD_MUTEX)
typedef pthread_mutex_t sharedsem_t;

class dbGlobalCriticalSection { 
    sharedsem_t* sem;
  public:
    void enter() {
#ifdef NDEBUG
        pthread_mutex_lock(sem);
#else
        int rc = pthread_mutex_lock(sem);
        assert(rc == 0);
#endif
    }
    void leave() { 
#ifdef NDEBUG
        pthread_mutex_unlock(sem);
#else
        int rc = pthread_mutex_unlock(sem);
        assert(rc == 0);
#endif
    }
    bool open(char const*, sharedsem_t* shr) { 
        sem = shr;
        return true;
    }
    bool create(char const*, sharedsem_t* shr) { 
        pthread_mutexattr_t attr;
        sem = shr;
        if (pthread_mutexattr_init(&attr) != 0) { 
            return false;
        }
        if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0) { 
            return false;
        }
#ifdef VXWORKS
        memset(sem, 0, sizeof(*sem));
#endif 
        return pthread_mutex_init(sem, &attr) == 0;
    }
    void close() {}
    void erase() {
        pthread_mutex_destroy(sem);
    }
};


#elif defined(__osf__) && !defined(RECOVERABLE_CRITICAL_SECTION)
// OSF uses "shared memory semaphores", located within a region mapped with mmap().

typedef msemaphore sharedsem_t;

class dbGlobalCriticalSection { 
    sharedsem_t* sem;
  public:
    void enter() { 
        int rc;
        while ((rc = msem_lock(sem, 0)) < 0 && errno == EINTR);
        assert(rc == 0);
    }
    void leave() { 
        int rc = msem_unlock(sem, 0);
        assert(rc == 0);        
    }
    bool open(char const*, sharedsem_t* shr) { 
        sem = shr;
        return true;
    }
    bool create(char const*, sharedsem_t* shr) { 
        sem = shr;
        msem_init(shr, MSEM_UNLOCKED);
        return true;
    }
    void close() {}
    void erase() {
        msem_remove(sem);
    }
};
        

#elif defined(__sun) && !defined(RECOVERABLE_CRITICAL_SECTION)
// Sun uses the Solaris style semaphore implemenation (sema_init(), sema_post())

END_FASTDB_NAMESPACE
#include <synch.h>
BEGIN_FASTDB_NAMESPACE
 
typedef sema_t sharedsem_t;

class dbGlobalCriticalSection { 
    sharedsem_t* sem;
  public:
    void enter() { 
#ifdef NDEBUG
        while (sema_wait(sem) < 0 && errno == EINTR);
#else
        int rc;
        while ((rc = sema_wait(sem)) < 0 && errno == EINTR);
        assert(rc == 0);
#endif
    }
    void leave() { 
#ifdef NDEBUG
        sema_post(sem);
#else
        int rc = sema_post(sem);
        assert(rc == 0);
#endif
    }
    bool open(char const*, sharedsem_t* shr) { 
        sem = shr;
        return true;
    }
    bool create(char const*, sharedsem_t* shr) { 
        sem = shr;
        return sema_init(shr, 1, USYNC_PROCESS, NULL) == 0;
    }
    void close() {}
    void erase() {
        sema_destroy(sem);
    }
};

#elif defined(USE_POSIX_SEMAPHORES) && !defined(RECOVERABLE_CRITICAL_SECTION)
// Everyone else uses the POSIX style semaphores (sem_wait(), sem_post(), etc) if defined
typedef sem_t sharedsem_t;

class dbGlobalCriticalSection { 
    sharedsem_t* sem;

  public:
    void enter() { 
#ifdef NDEBUG
        sem_wait(sem);
#else
        int rc = sem_wait(sem);
        assert(rc == 0);
#endif
    }
    void leave() { 
#ifdef NDEBUG
        sem_post(sem);
#else
        int rc = sem_post(sem);
        assert(rc == 0);
#endif
    }
    
#ifdef __APPLE__ // MacOS doesn't support sem_init
    char* sem_name;

    bool open(char const* name, sharedsem_t* shr) { 
        delete[] sem_name;
        sem_name = new char[strlen(name)+1];
        strcpy(sem_name, name);
        sem = sem_open(name, 0);
        return sem != SEM_FAILED;
    }

    bool create(char const* name, sharedsem_t* shr) {   
        delete[] sem_name;
        sem_name = new char[strlen(name)+1];
        strcpy(sem_name, name);
        sem = sem_open(name, O_CREAT|O_EXCL, 0777, 0);
        return sem != SEM_FAILED && sem_post(sem) == 0;
    }

    void erase() { 
        sem_unlink(sem_name);
    }
    
    dbGlobalCriticalSection() : sem_name(NULL) {
    }

    ~dbGlobalCriticalSection() { 
        delete[] sem_name;
    }
#else
    bool open(char const* name, sharedsem_t* shr) { 
        sem = shr;
        return true;
    }

    bool create(char const* name, sharedsem_t* shr) {   
        sem = shr;
        return sem_init(sem, 1, 1) == 0;
    }
    void erase() { 
        sem_destroy(sem);
    }
#endif


    void close() {}
};

#else

#define USE_INTERNAL_CS_IMPL

#define GLOBAL_CS_DEBUG 1

// Lastly, use the local implementation
typedef int sharedsem_t;

class dbGlobalCriticalSection { 
    int          semid;
    sharedsem_t* count;
#if GLOBAL_CS_DEBUG
    pthread_t    owner;
#endif

  public:
    void enter(); 
    void leave();
    bool open(char const* name, sharedsem_t* shr);
    bool create(char const* name, sharedsem_t* shr);
    void close() {}
    void erase();
};
#endif //dbGLobalCriticalSection switch

END_FASTDB_NAMESPACE

#endif //__SYNC_UNIX_H__
