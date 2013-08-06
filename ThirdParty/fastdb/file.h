//-< FILE.CPP >------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// System independent intrface to mapped on memory file
//-------------------------------------------------------------------*--------*

#ifndef __FILE_H__
#define __FILE_H__

#include "sync.h"

BEGIN_FASTDB_NAMESPACE

#if HAVE_ZLIB
#include "zlib.h"
#endif

#if defined(REPLICATION_SUPPORT)
const int dbModMapBlockBits = 12; // 10;
const int dbModMapBlockSize = 1 << dbModMapBlockBits;
#elif defined(NO_MMAP)
const int dbModMapBlockBits = 12;
const int dbModMapBlockSize = 1 << dbModMapBlockBits;
#endif

#ifdef REPLICATION_SUPPORT

class dbFile;
class dbReplicatedDatabase;
class socket_t;

struct ReplicationRequest { 
    enum {
        RR_CONNECT, 
        RR_RECOVERY, 
        RR_GET_STATUS, 
        RR_STATUS, 
        RR_UPDATE_PAGE,
        RR_RECOVER_PAGE,
        RR_NEW_ACTIVE_NODE, 
        RR_CHANGE_ACTIVE_NODE, 
        RR_CLOSE, 
        RR_READY,
        RR_COMMITTED
    };
    byte op;
    byte nodeId;
    byte status;
    int  size;
    struct { 
        int updateCount;
        size_t offs;
    } page;
};

struct RecoveryRequest { 
    dbFile*   file;
    int       nodeId;
    int       nPages;
    int*      updateCounters;
};
#endif

#ifdef FUZZY_CHECKPOINT
class  dbFileWriter;
#endif

class FASTDB_DLL_ENTRY dbFile { 
  protected:
#ifdef _WIN32
    HANDLE fh;
    HANDLE mh;
#else
#ifdef USE_SYSV_SHARED_MEMORY
    dbSharedMemory shmem;
#endif
    int    fd;
#endif
    char_t*  sharedName;
    char*  mmapAddr;
    size_t mmapSize;
    int    flags;

  public:
    enum { 
        ok = 0
    };
    enum OpenFlags {
        read_only       = 0x001, // open file in read-only mode
        truncate        = 0x002, // truncate file when opened
        sequential      = 0x004, // optimize for sequenial access 
        no_buffering    = 0x008, // do not use OS file cache
        no_sync         = 0x010, // do not flush data to the disk
        shared          = 0x020, // shared access to the file
        write_through   = 0x040, // write through 
        delete_on_close = 0x080, // delete file on close
        ram_file        = 0x100, // diskless mode
        force_read      = 0x200  // force load of mapped data from the file
    };
    //
    // Create backup file
    //
    virtual int   create(char_t const* name, int flags = no_buffering);
    //
    // Open database file and create file mapping object 
    //
    virtual int    open(char_t const* fileName, char_t const* sharedName,
                        int flags, size_t initSize, bool replicationSupport);
    
    virtual void*  getAddr() const { return mmapAddr; }
    virtual size_t getSize() const { return mmapSize; } 
    virtual int    setSize(size_t size, char_t const* sharedName, bool initialize = true);
    virtual int    flush(bool physical = false);
    virtual int    close();
    virtual int    erase();
    virtual int    write(void const* ptr, size_t& writtenBytes, size_t size);
    virtual int    read(void* ptr, size_t& readBytes, size_t size);
    virtual bool   write(void const* ptr, size_t size);
    virtual int    seek(size_t pos, int whence);

    static char* errorText(int code, char* buf, size_t bufSize);

    int forceRead(char* data, size_t size);

#if defined(NO_MMAP) || defined(REPLICATION_SUPPORT)

#ifdef PROTECT_DATABASE
    void protect(size_t pos, size_t size);
    void unprotect(size_t pos, size_t size);
#endif

    void markAsDirty(size_t pos, size_t size) { 
        if (!(flags & ram_file)) {
            size_t page = pos >> dbModMapBlockBits;
            size_t last = (pos + size + dbModMapBlockSize - 1) >> dbModMapBlockBits;
            assert(int(last >> 5) <= pageMapSize);
            while (page < last) { 
                pageMap[page >> 5] |= 1 << (page & 31);
                page += 1;
            }
        }
    }

  private:
    int* pageMap;
    int  pageMapSize;
    int  pageSize;

#ifdef FUZZY_CHECKPOINT
    dbFileWriter* writer;
  public:
    void setCheckpointBufferSize(size_t nPages);
#endif

  public:
    int  updateCounter;

#ifdef REPLICATION_SUPPORT
    int*      currUpdateCount;
    int*      diskUpdateCount;
    byte*     rootPage;
    bool      doSync;
    bool      closing;

    dbReplicatedDatabase* db;

    int       getUpdateCountTableSize();
    int       getMaxPages(); 

    dbMutex   replCS;
    dbMutex   syncCS;

    dbThread     syncThread;
    dbLocalEvent syncEvent;
    dbLocalEvent recoveredEvent;
    int          nRecovered;

    static int   dbSyncTimeout; // milliseconds

#ifdef _WIN32
    HANDLE    cfh;
    HANDLE    cmh;
#else
    int       cfd;
#endif

    static void thread_proc startSyncToDisk(void* arg);
    static void thread_proc startRecovery(void* arg);


    void doRecovery(int nodeId, int* updateCounters, int nPages);

    int sendChanges(int nodeId, int* updateCounters, int nPages);
    void completeRecovery(int nodeId);

    void syncToDisk();
    void startSync();
    void stopSync();

  public:
    void configure(dbReplicatedDatabase* db) { 
        this->db = db;
    }

    bool updatePages(socket_t* s, size_t pos, int updateCount, int size);
    bool concurrentUpdatePages(socket_t* s, size_t pos, int updateCount, int size);
    void recovery(int nodeId, int* updateCounters, int nPages);
#endif // REPLICATION_SUPPORT


#else
    void markAsDirty(size_t, size_t) {}
#endif

    bool write(size_t pos, void const* ptr, size_t size);

    dbFile();
    virtual ~dbFile();
};


#if HAVE_ZLIB
class dbCompressedFile : public dbFile {
  protected:
    gzFile gzh;
  public:
    virtual int    create(char_t const* name, int flags = truncate|no_buffering);

    virtual int    open(char_t const* fileName, char_t const* sharedName,
                        int flags, size_t initSize, bool replicationSupport) {
        assert(false); // not implemented
        return 0;
    }
    virtual void*  getAddr() const {SVM_ASSERTMSG(0, "Not Implemented"); return NULL;}
    virtual size_t getSize() const {SVM_ASSERTMSG(0, "Not Implemented"); return 0;}
    virtual int    setSize(size_t size, char_t const* sharedName, bool initialize = true)  {SVM_ASSERTMSG(0, "Not Implemented"); return 0;}
    virtual int    flush(bool physical = false)  {SVM_ASSERTMSG(0, "Not Implemented"); return 0;}
    virtual int    erase() {SVM_ASSERTMSG(0, "Not Implemented"); return 0;}

    virtual int    close();
    virtual int    write(void const* ptr, size_t& writtenBytes, size_t size);
    virtual int    read(void* ptr, size_t& readBytes, size_t size);
};
#endif
END_FASTDB_NAMESPACE

#endif
