//-< W32SOCK.CXX >---------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1997  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:      8-May-97    K.A. Knizhnik  * / [] \ *
//                          Last update: 19-May-97    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Windows sockets  
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "stdtp.h"
#include "w32sock.h"
#include "sync.h"

BEGIN_FASTDB_NAMESPACE

#define MAX_HOST_NAME         256
#define MILLISECOND           1000

static HANDLE WatchDogMutex;

#if (defined(_MSC_VER) && _MSC_VER <= 1310) || defined(__GNUC__) || defined(__BORLANDC__)
#define MEMORY_BARRIER_NOT_DEFINED true
#endif 

#ifdef MEMORY_BARRIER_NOT_DEFINED

#ifdef __GNUC__

#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)
#define MemoryBarrier() __sync_synchronize()
#else
inline void MemoryBarrier() { 
    asm volatile("mfence":::"memory");
}
#endif

#else

#if defined(_M_AMD64)
#define MemoryBarrier __faststorefence
#elif defined(_M_IA64)
#define MemoryBarrier __mf
#elif (defined(_M_IX86) || defined(_M_X64)) && !defined(__BORLANDC__)
inline void MemoryBarrier() {
    LONG Barrier;
    __asm {
        xchg Barrier, eax
    }
}
#else
static dbMutex barrierMutex;
inline void MemoryBarrier() { dbCriticalSection cs(barrierMutex); }
#endif
#endif
#endif

class win_socket_library { 
  public:
    SYSTEM_INFO sinfo;
    
    win_socket_library() { 
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(1, 1), &wsa) != 0) {
            fprintf(stderr,"Failed to initialize windows sockets: %d\n",
                    WSAGetLastError());
        }
        //
        // This mutex is used to recognize process termination
        //
        WatchDogMutex = CreateMutex(FASTDB_SECURITY_ATTRIBUTES, TRUE, NULL);

#ifdef PHAR_LAP
        sinfo.wProcessorLevel = 5;
#else
        GetSystemInfo(&sinfo);  
#endif
    }
    ~win_socket_library() {
        //      WSACleanup();
    }
};

static win_socket_library ws32_lib;

bool win_socket::open(int listen_queue_size)
{
    unsigned short port;
    char* p;
    char hostname[MAX_HOST_NAME];

    assert(address != NULL);

    if ((p = strchr(address, ':')) == NULL 
        || sscanf(p+1, "%hu", &port) != 1) 
    {
        errcode = bad_address;
        TRACE_MSG(("Invalid address: %s\n", address));
        return false;
    }
    memcpy(hostname, address, p - address);
    hostname[p - address] = '\0';

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) { 
        errcode = WSAGetLastError();
        TRACE_MSG(("Socket create is failed: %d\n", errcode));
        return false;
    }
    struct sockaddr_in insock;
    insock.sin_family = AF_INET;
    if (*hostname && stricmp(hostname, "localhost") != 0) {
        struct hostent* hp;  // entry in hosts table
        if ((hp = gethostbyname(hostname)) == NULL 
            || hp->h_addrtype != AF_INET) 
        {
            TRACE_MSG(("Failed to get host by name: %s\n", errno));
            errcode = bad_address;
            return false;
        }
        memcpy(&insock.sin_addr, hp->h_addr, sizeof insock.sin_addr);
    } else {
        insock.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    insock.sin_port = htons(port);

    int on = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof on);
    
    if (bind(s, (sockaddr*)&insock, sizeof(insock)) != 0) { 
        errcode = WSAGetLastError();
        TRACE_MSG(("Socket bind is failed: %d\n", errcode));
        closesocket(s);
        return false;
    }
    if (listen(s, listen_queue_size) != 0) {
        errcode = WSAGetLastError();
        TRACE_MSG(("Socket listen is failed: %d\n", errcode));
        closesocket(s);
        return false;
    } 
    errcode = ok;
    state = ss_open;
    return true;
}

bool win_socket::is_ok()
{
    return errcode == ok;
}

socket_handle_t win_socket::get_handle()
{
    return s;
}

void win_socket::get_error_text(char* buf, size_t buf_size)
{
    char* msg; 
    char  msgbuf[64];

    switch(errcode) { 
      case ok:
        msg = "ok";
        break;
      case not_opened:
        msg = "socket not opened";
        break;
      case bad_address: 
        msg = "bad address";
        break;
      case connection_failed: 
        msg = "exceed limit of attempts of connection to server";
        break;
      case broken_pipe:
        msg = "connection is broken";
        break; 
      case invalid_access_mode:
        msg = "invalid access mode";
        break;
      default: 
#ifndef PHAR_LAP
        {
          int   len;
#if defined(_WINCE) || defined(UNICODE)
          wchar_t cnvBuf[CNV_BUF_SIZE];
          FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        errcode,
                        0,
                        cnvBuf,
                        CNV_BUF_SIZE-1,
                        NULL);
          cnvBuf[CNV_BUF_SIZE-1] = '\0';
          len = wcstombs(buf, cnvBuf, buf_size);
#else
          len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                              NULL,
                              errcode,
                              0,
                              buf,
                              (DWORD)buf_size-1,
                              NULL);
#endif
          if (len == 0) { 
              sprintf(msgbuf, "unknown error code %u", errcode);
              msg = msgbuf;
          } else { 
              return;
          }
        }
#else
        sprintf(msgbuf, "System error code: %u", errcode);
        msg = msgbuf;
#endif
    }
    strncpy(buf, msg, buf_size-1);
    buf[buf_size-1] = '\0';
}

socket_t* win_socket::accept()
{
    if (state != ss_open) { 
        errcode = not_opened;
        TRACE_MSG(("Socket not openned\n"));
        return NULL;
    }

    SOCKET new_sock = ::accept(s, NULL, NULL );

    if (new_sock == INVALID_SOCKET) { 
        errcode = WSAGetLastError();
        TRACE_MSG(("Socket accept failed: %d\n", errcode));
        return NULL;
    } else { 
#if SOCK_LINGER
        static struct linger l = {1, LINGER_TIME};
        if (setsockopt(new_sock, SOL_SOCKET, SO_LINGER, (char*)&l, sizeof l) != 0) { 
            errcode = WSAGetLastError();
            TRACE_MSG(("Failed to set socket options: %d\n", errcode));
            closesocket(new_sock);
            return NULL; 
        }
#endif
#if SOCK_NO_DELAY
        int enabled = 1;
        if (setsockopt(new_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&enabled, 
                       sizeof enabled) != 0)
        {
            errcode = WSAGetLastError();
            TRACE_MSG(("Failed to set socket options: %d\n", errcode));
            closesocket(new_sock);      
            return NULL;
        }
#endif
#if SOCK_SNDBUF_SIZE
        int size = SOCK_SNDBUF_SIZE;
        setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof size);
#endif
        errcode = ok;
        return new win_socket(new_sock); 
    }
}

bool win_socket::cancel_accept() 
{
    bool result = close();
    // Wakeup listener
    delete socket_t::connect(address, sock_global_domain, 1, 0);
    return result;
}    


bool win_socket::connect(int max_attempts, time_t timeout)
{
    char hostname[MAX_HOST_NAME];
    char *p;
    unsigned short port;

    assert(address != NULL);

    if ((p = strchr(address, ':')) == NULL 
        || (size_t)(p - address) >= sizeof(hostname) 
        || sscanf(p+1, "%hu", &port) != 1) 
    {
        errcode = bad_address;
        TRACE_MSG(("Invalid address: %s\n", address));
        return false;
    }
    memcpy(hostname, address, p - address);
    hostname[p - address] = '\0';

    struct sockaddr_in insock;  // inet socket address
    struct hostent*    hp;      // entry in hosts table

    if ((hp = gethostbyname(hostname)) == NULL || hp->h_addrtype != AF_INET) {
        TRACE_MSG(("Host name can not be resolved: %d\n", WSAGetLastError()));
        errcode = bad_address;
        return false;
    }
    insock.sin_family = AF_INET;
    insock.sin_port = htons(port);
    
    while (true) {
        for (int i = 0; hp->h_addr_list[i] != NULL; i++) { 
            memcpy(&insock.sin_addr, hp->h_addr_list[i],
                   sizeof insock.sin_addr);
            if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) { 
                errcode = WSAGetLastError();
                TRACE_MSG(("Failed to create socket: %d\n", errcode));
                return false;
            }
            if (::connect(s, (sockaddr*)&insock, sizeof insock) != 0) { 
                errcode = WSAGetLastError();
                closesocket(s);
                if (errcode != WSAECONNREFUSED) {
                    TRACE_MSG(("Failed to establish connection: %d\n", errcode));
                    return false;
                }
            } else {
#if SOCK_NO_DELAY
                int enabled = 1;
                if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char*)&enabled, 
                               sizeof enabled) != 0)
                {
                    errcode = WSAGetLastError();
                    TRACE_MSG(("Failed to set socket options: %d\n", errcode));
                    closesocket(s);     
                    return false;
                }
#endif
#if SOCK_LINGER
                static struct linger l = {1, LINGER_TIME};
                if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char*)&l, sizeof l) != 0) { 
                    errcode = WSAGetLastError();
                    TRACE_MSG(("Failed to set socket options: %d\n", errcode));
                    closesocket(s);
                    return NULL; 
                }
#endif
                errcode = ok;
                state = ss_open;
                return true;
            }
        }
        if (--max_attempts > 0) {  
            Sleep((DWORD)(timeout*MILLISECOND));
        } else { 
            errcode = connection_failed;
            TRACE_MSG(("All attempts to establish connection are failed\n"));
            return false;
        }
    }
}

int win_socket::read(void* buf, size_t min_size, size_t max_size, 
                     time_t timeout)
{ 
    size_t size = 0;
    time_t start = 0;
    if (state != ss_open) { 
        errcode = not_opened;
        TRACE_MSG(("Socket is not openned\n"));
        return -1;
    }
    if (timeout != WAIT_FOREVER) { 
        start = time(NULL); 
    }

    do { 
        int rc;
        if (timeout != WAIT_FOREVER) { 
            fd_set events;
            struct timeval tm;
            FD_ZERO(&events);
            FD_SET(s, &events);
            tm.tv_sec = (long)timeout;
            tm.tv_usec = 0;
            rc = select((int)s+1, &events, NULL, NULL, &tm);
            if (rc < 0) { 
                errcode = WSAGetLastError();
                TRACE_MSG(("Socket select is failed: %d\n", errcode));
                return -1;
            }
            if (rc == 0) {
                return (int)size;
            }
            time_t now = time(NULL);
            timeout = start + timeout >= now ? timeout + start - now : 0;  
        }
        rc = recv(s, (char*)buf + size, (int)(max_size - size), 0);
        if (rc < 0) { 
            errcode = WSAGetLastError();
            TRACE_MSG(("Socket read is failed: %d\n", errcode));
            return -1;
        } else if (rc == 0) {
            errcode = broken_pipe;
            TRACE_MSG(("Socket is disconnected\n"));
            return -1; 
        } else { 
            size += rc; 
        }
    } while (size < min_size); 

    return (int)size;
}
        
bool win_socket::write(void const* buf, size_t size, time_t timeout)
{ 
    time_t start = 0;
    if (state != ss_open) { 
        errcode = not_opened;
        TRACE_MSG(("Socket is not openned\n"));
        return false;
    }
    if (timeout != WAIT_FOREVER) { 
        start = time(NULL); 
    }
    
    do { 
        int rc;
        if (timeout != WAIT_FOREVER) { 
            fd_set events;
            struct timeval tm;
            FD_ZERO(&events);
            FD_SET(s, &events);
            tm.tv_sec = (long)timeout;
            tm.tv_usec = 0;
            rc = select((int)s+1, NULL, &events, NULL, &tm);
            if (rc <= 0) { 
                errcode = WSAGetLastError();
                TRACE_MSG(("Socket select is failed: %d\n", errcode));
                return false;
            }
            time_t now = time(NULL);
            timeout = start + timeout >= now ? timeout + start - now : 0;  
        }
        rc = send(s, (char*)buf, (int)size, 0);
        if (rc < 0) { 
            errcode = WSAGetLastError();
            TRACE_MSG(("Socket write is failed: %d\n", errcode));
            return false;
        } else if (rc == 0) {
            errcode = broken_pipe;
            TRACE_MSG(("Socket is disconnected\n"));
            return false; 
        } else { 
            buf = (char*)buf + rc; 
            size -= rc; 
        }
    } while (size != 0); 

    return true;
}

bool win_socket::shutdown()
{
    if (state == ss_open) { 
        state = ss_shutdown;
        int rc = ::shutdown(s, 2);
        if (rc != 0) {
            errcode = WSAGetLastError();
            TRACE_MSG(("Socket shutdown is failed: %d\n", errcode));
            return false;
        } 
    } 
    errcode = ok;
    return true;
}


bool win_socket::close()
{
    if (state != ss_close) { 
        state = ss_close;
        if (closesocket(s) == 0) { 
            errcode = ok;
            return true;
        } else { 
            errcode = WSAGetLastError();
            TRACE_MSG(("Socket close is failed: %d\n", errcode));
            return false;
        }
    }
    return true;
}

char* win_socket::get_peer_name()
{
    if (state != ss_open) { 
        errcode = not_opened;
        return NULL;
    }
    struct sockaddr_in insock;
    int len = sizeof(insock);
    if (getpeername(s, (struct sockaddr*)&insock, &len) != 0) { 
        errcode = WSAGetLastError();
        return NULL;
    }
    char* addr = inet_ntoa(insock.sin_addr);
    if (addr == NULL) { 
        errcode = WSAGetLastError();
        return NULL;
    }
    char* addr_copy = new char[strlen(addr)+1];
    strcpy(addr_copy, addr);
    errcode = ok;
    return addr_copy;
}

win_socket::~win_socket()
{
    close();
    delete[] address;
}

win_socket::win_socket(const char* addr)
{ 
    address = new char[strlen(addr)+1];
    strcpy(address, addr); 
    errcode = ok;
    s = INVALID_SOCKET;
}

win_socket::win_socket(SOCKET new_sock) 
{ 
    s = new_sock; 
    address = NULL; 
    state = ss_open;
    errcode = ok;
}

socket_t* socket_t::create_local(char const* address, int listen_queue_size)
{
#ifdef _WINCE
    return NULL;
#else
    local_win_socket* sock = new local_win_socket(address);
    sock->open(listen_queue_size);
    return sock;
#endif
}

socket_t* socket_t::create_global(char const* address, int listen_queue_size)
{
    win_socket* sock = new win_socket(address);
    sock->open(listen_queue_size); 
    return sock;
}

socket_t* socket_t::connect(char const* address, 
                            socket_domain domain, 
                            int max_attempts,
                            time_t timeout)
{
#ifndef _WINCE
    char   hostname[MAX_HOST_NAME];
    size_t hostname_len;
    char const* port;
    
    if (domain == sock_local_domain 
        || (domain == sock_any_domain 
            && ((port = strchr(address, ':')) == NULL 
                || ((hostname_len = port - address) == 9 
                    && strncmp(address, "localhost", hostname_len) == 0)
                || (gethostname(hostname, sizeof hostname) == 0 
                    && strlen(hostname) == hostname_len 
                    && strncmp(address, hostname, hostname_len) == 0))))
    {
        local_win_socket* s = new local_win_socket(address);
        s->connect(max_attempts, timeout); 
        return s;
    } 
    else 
#endif
    { 
        win_socket* s = new win_socket(address);
        s->connect(max_attempts, timeout); 
        return s;
    }  
}
    
#ifndef _WINCE

//
// Local windows sockets
//

int local_win_socket::read(void* buf, size_t min_size, size_t max_size, 
                           time_t timeout)
{
    time_t start = 0;
    char* dst = (char*)buf;
    size_t size = 0;
    Error = ok;
    if (timeout != WAIT_FOREVER) { 
        start = time(NULL); 
        timeout *= 1000; // convert seconds to miliseconds
    }
    while (size < min_size && state == ss_open) {       
        RcvBuf->RcvWaitFlag = true;
        MemoryBarrier();
        size_t begin = RcvBuf->DataBeg;
        size_t end = RcvBuf->DataEnd;
        size_t rcv_size = (begin <= end)
            ? end - begin : sizeof(RcvBuf->Data) - begin;
        if (rcv_size > 0) { 
            RcvBuf->RcvWaitFlag = false;
            if (rcv_size >= max_size) { 
                memcpy(dst, &RcvBuf->Data[begin], max_size);
                begin += max_size;
                size += max_size;
            } else { 
                memcpy(dst, &RcvBuf->Data[begin], rcv_size);
                begin += rcv_size;
                dst += rcv_size;
                size += rcv_size;
                max_size -= rcv_size;
            } 
            RcvBuf->DataBeg = (begin == sizeof(RcvBuf->Data)) ? 0 : (int)begin;
            MemoryBarrier();
            if (RcvBuf->SndWaitFlag) { 
                SetEvent(Signal[RTR]);
            }           
        } else {
            HANDLE h[2];
            h[0] = Signal[RD];
            h[1] = Mutex;
            int rc = WaitForMultipleObjects(2, h, false, (DWORD)timeout);
            RcvBuf->RcvWaitFlag = false;
            if (rc != WAIT_OBJECT_0) {
                if (rc == WAIT_OBJECT_0+1 || rc == WAIT_ABANDONED+1) { 
                    Error = broken_pipe;
                    ReleaseMutex(Mutex);
                } else if (rc == WAIT_TIMEOUT) { 
                    return (int)size;
                } else { 
                    Error = GetLastError();
                }
                return -1;
            }
            if (timeout != WAIT_FOREVER) { 
                time_t now = time(NULL);
                timeout = timeout >= (now - start)*1000 
                    ? timeout - (now - start)*1000 : 0;  
            }
        }
    }                   
    return size < min_size ? -1 : (int)size;
}


bool local_win_socket::write(const void* buf, size_t size, time_t timeout)
{
    char* src = (char*)buf;
    Error = ok;
    while (size > 0 && state == ss_open) {      
        SndBuf->SndWaitFlag = true;
        MemoryBarrier();
        size_t begin = SndBuf->DataBeg;
        size_t end = SndBuf->DataEnd;
        size_t snd_size = (begin <= end) 
            ? sizeof(SndBuf->Data) - end - (begin == 0)
            : begin - end - 1;
        if (snd_size > 0) { 
            SndBuf->SndWaitFlag = false;
            if (snd_size >= size) { 
                memcpy(&SndBuf->Data[end], src, size);
                end += size;
                size = 0;
            } else { 
                memcpy(&SndBuf->Data[end], src, snd_size);
                end += snd_size;
                src += snd_size;
                size -= snd_size;
            } 
            SndBuf->DataEnd = (end == sizeof(SndBuf->Data)) ? 0 : (int)end;
            MemoryBarrier();
            if (SndBuf->RcvWaitFlag) { 
                SetEvent(Signal[TD]);
            }           
        } else {
            HANDLE h[2];
            h[0] = Signal[RTT];
            h[1] = Mutex;
            int rc = WaitForMultipleObjects(2, h, false, (DWORD)timeout);
            SndBuf->SndWaitFlag = false;
            if (rc != WAIT_OBJECT_0) {
                if (rc == WAIT_OBJECT_0+1 || rc == WAIT_ABANDONED+1) { 
                    Error = broken_pipe;
                    ReleaseMutex(Mutex);
                } else { 
                    Error = GetLastError();
                }       
                return false;
            }
        }
    }                           
    return size == 0;
}

#define MAX_ADDRESS_LEN 64

local_win_socket::local_win_socket(const char* address)
{
    Name = new char[strlen(address)+1];
    strcpy(Name, address);
    Error = not_opened;
    Mutex = NULL;
}
 
bool local_win_socket::open(int)
{
    char buf[MAX_ADDRESS_LEN];  
    int  i;

    for (i = RD; i <= RTT; i++) {  
        sprintf(buf, "%s.%c", Name, i + '0');
        Signal[i] = CreateEvent(FASTDB_SECURITY_ATTRIBUTES, false, false, WC_STRING(buf));
        if (GetLastError() == ERROR_ALREADY_EXISTS) { 
            WaitForSingleObject(Signal[i], 0);
        }
        if (!Signal[i]) {
            Error = GetLastError();
            while (--i >= 0) { 
                CloseHandle(Signal[i]);
            }
            return false;
        }       
    }
    sprintf(buf, "%s.shr", Name);
    BufHnd = CreateFileMapping(INVALID_HANDLE_VALUE, FASTDB_SECURITY_ATTRIBUTES, PAGE_READWRITE,
                               0, sizeof(socket_buf)*2, WC_STRING(buf));
    if (!BufHnd) {
        Error = GetLastError();
        for (i = RD; i <= RTT; i++) {  
            CloseHandle(Signal[i]);
        }
        return false;
    }
    RcvBuf = (socket_buf*)MapViewOfFile(BufHnd, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!RcvBuf) {
        Error = GetLastError();
        CloseHandle(BufHnd);
        for (i = RD; i <= RTT; i++) {  
            CloseHandle(Signal[i]);
        }
        return false;
    }   
    SndBuf = RcvBuf+1;
    RcvBuf->DataBeg = RcvBuf->DataEnd = 0;
    SndBuf->DataBeg = SndBuf->DataEnd = 0;       
    Error = ok;
    state = ss_open;
    return true;
}

local_win_socket::local_win_socket()
{
    int i;
    BufHnd = NULL;
    Mutex = NULL; 
    Name = NULL;

    for (i = RD; i <= RTT; i++) {  
        Signal[i] = CreateEvent(FASTDB_SECURITY_ATTRIBUTES, false, false, NULL);
        if (!Signal[i]) {
            Error = GetLastError();
            while (--i >= 0) { 
                CloseHandle(Signal[i]);
            }
            return;
        }       
    }
    // create anonymous shared memory section
    BufHnd = CreateFileMapping(INVALID_HANDLE_VALUE, FASTDB_SECURITY_ATTRIBUTES, PAGE_READWRITE,
                               0, sizeof(socket_buf)*2, NULL);
    if (!BufHnd) {
        Error = GetLastError();
        for (i = RD; i <= RTT; i++) {  
            CloseHandle(Signal[i]);
        }
        return;
    }
    RcvBuf = (socket_buf*)MapViewOfFile(BufHnd, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!RcvBuf) {
        Error = GetLastError();
        CloseHandle(BufHnd);
        for (i = RD; i <= RTT; i++) {  
            CloseHandle(Signal[i]);
        }
        BufHnd = NULL;
        return;
    }   
    SndBuf = RcvBuf+1;
    RcvBuf->DataBeg = RcvBuf->DataEnd = 0;
    SndBuf->DataBeg = SndBuf->DataEnd = 0;       
    Error = ok;
    state = ss_open;
}

local_win_socket::~local_win_socket()
{
    close();
    delete[] Name;
}       

socket_t* local_win_socket::accept()
{   
    HANDLE h[2];

    if (state != ss_open) {     
        return NULL;
    }
                    
    connect_data* cdp = (connect_data*)SndBuf->Data;
    cdp->Pid = GetCurrentProcessId();
    cdp->Mutex = WatchDogMutex;
    while (true) { 
        SetEvent(Signal[RTR]);
        int rc = WaitForSingleObject(Signal[RD], ACCEPT_TIMEOUT);
        if (rc == WAIT_OBJECT_0) {
            if (state != ss_open) { 
                Error = not_opened;
                return NULL;
            }
            Error = ok;
            break;
        } else if (rc != WAIT_TIMEOUT) { 
            Error = GetLastError();
            return NULL;
        }
    }
    local_win_socket* sock = new local_win_socket();
    sock->Mutex = ((connect_data*)RcvBuf->Data)->Mutex;
    accept_data* adp = (accept_data*)SndBuf->Data;
    adp->BufHnd = sock->BufHnd;
    for (int i = RD; i <= RTT; i++) { 
        adp->Signal[(i + TD - RD) & RTT] = sock->Signal[i]; 
    }
    SetEvent(Signal[TD]);
    h[0] = Signal[RD];
    h[1] = sock->Mutex;
    int rc = WaitForMultipleObjects(2, h, false, INFINITE);
    if (rc != WAIT_OBJECT_0) {
        if (rc == WAIT_OBJECT_0+1 || rc == WAIT_ABANDONED+1) { 
            Error = broken_pipe;
            ReleaseMutex(Mutex);
        } else { 
            Error = GetLastError();
        }       
        delete sock;
        return NULL;
    }    
    return sock;
}

bool local_win_socket::cancel_accept() 
{
    state = ss_shutdown;
    SetEvent(Signal[RD]);
    SetEvent(Signal[RTT]);
    return true;
}    

char* local_win_socket::get_peer_name()
{
    if (state != ss_open) { 
        Error = not_opened;
        return NULL;
    }
    char* addr = "127.0.0.1";
    char* addr_copy = new char[strlen(addr)+1];
    strcpy(addr_copy, addr);
    Error = ok;
    return addr_copy;
}

bool local_win_socket::is_ok()
{
    return !Error;
}

bool local_win_socket::close()
{
    if (state != ss_close) {            
        state = ss_close;
        if (Mutex) { 
            CloseHandle(Mutex);
        }
        for (int i = RD; i <= RTT; i++) { 
            CloseHandle(Signal[i]);
        }
        UnmapViewOfFile(RcvBuf < SndBuf ? RcvBuf : SndBuf);
        CloseHandle(BufHnd);    
        Error = not_opened;
    }
    return true;
}

void local_win_socket::get_error_text(char* buf, size_t buf_size)
{
    switch (Error) { 
      case ok:
        strncpy(buf, "ok", buf_size-1);
        break;
      case not_opened:
        strncpy(buf, "socket not opened", buf_size-1);
        break;
      case broken_pipe:
        strncpy(buf, "connection is broken", buf_size-1);
        break;
      case timeout_expired:
        strncpy(buf, "connection timeout expired", buf_size-1);
        break;
      default:  
#ifndef PHAR_LAP
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                      NULL,
                      Error,
                      0,
                      WC_STRING(buf),
                      (DWORD)buf_size-1,
                      NULL);
#else
        strncpy(buf, "Unknown socket error", buf_size-1);
#endif
    }
    buf[buf_size-1] = '\0';
}


bool local_win_socket::shutdown()
{
    if (state == ss_open) { 
        state = ss_shutdown;
        SetEvent(Signal[RD]);   
        SetEvent(Signal[RTT]);  
    }
    return true;
}

bool local_win_socket::connect(int max_attempts, time_t timeout)
{
    char buf[MAX_ADDRESS_LEN];
    int  rc, i, error_code;
    HANDLE h[2];

    for (i = RD; i <= RTT; i++) {  
        sprintf(buf, "%s.%c", Name, ((i + TD - RD) & RTT) + '0');
        Signal[i] = CreateEvent(FASTDB_SECURITY_ATTRIBUTES, false, false, WC_STRING(buf));
        if (!Signal[i]) {
            Error = GetLastError();
            while (--i >= 0) { 
                CloseHandle(Signal[i]);
            }
            return false;
        }       
    }
    sprintf(buf, "%s.shr", Name);
    BufHnd = CreateFileMapping(INVALID_HANDLE_VALUE, FASTDB_SECURITY_ATTRIBUTES, PAGE_READWRITE,
                               0, sizeof(socket_buf)*2, WC_STRING(buf));
    if (!BufHnd) {
        Error = GetLastError();
        for (i = RD; i <= RTT; i++) {  
            CloseHandle(Signal[i]);
        }
        return false;
    }
    SndBuf = (socket_buf*)MapViewOfFile(BufHnd, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!SndBuf) { 
        Error = GetLastError();
        for (i = RD; i <= RTT; i++) {  
            CloseHandle(Signal[i]);
        }
        CloseHandle(BufHnd);
        return false;
    }
    RcvBuf = SndBuf+1;
    state = ss_shutdown;
    Mutex = NULL;

    rc = WaitForSingleObject(Signal[RTT], (DWORD)(timeout*max_attempts*MILLISECOND));
    if (rc != WAIT_OBJECT_0) {
        error_code = rc == WAIT_TIMEOUT ? timeout_expired : GetLastError();
        close();
        Error = error_code;
        return false;
    }
    connect_data* cdp = (connect_data*)RcvBuf->Data;
    HANDLE hServer = OpenProcess(STANDARD_RIGHTS_REQUIRED|PROCESS_DUP_HANDLE,
                                 false, cdp->Pid);
    if (!hServer) { 
        error_code = GetLastError();
        close();
        Error = error_code;
        return false;
    }
    HANDLE hSelf = GetCurrentProcess();
    if (!DuplicateHandle(hServer, cdp->Mutex, hSelf, &Mutex, 
                         0, false, DUPLICATE_SAME_ACCESS) ||
        !DuplicateHandle(hSelf, WatchDogMutex, hServer, 
                         &((connect_data*)SndBuf->Data)->Mutex, 
                         0, false, DUPLICATE_SAME_ACCESS))
    {
        error_code = GetLastError();
        CloseHandle(hServer);
        close();
        Error = error_code;
        return false;
    }
    SetEvent(Signal[TD]);
    h[0] = Signal[RD];
    h[1] = Mutex;
    rc = WaitForMultipleObjects(2, h, false, INFINITE);

    if (rc != WAIT_OBJECT_0) { 
        if (rc == WAIT_OBJECT_0+1 || rc == WAIT_ABANDONED+1) { 
            error_code = broken_pipe;
            ReleaseMutex(Mutex);
        } else { 
            error_code = GetLastError();
        }
        CloseHandle(hServer);
        close();
        Error = error_code;
        return false;
    }
    accept_data ad = *(accept_data*)RcvBuf->Data;

    SetEvent(Signal[TD]);
    for (i = RD; i <= RTT; i++) { 
        CloseHandle(Signal[i]);
    }
    UnmapViewOfFile(SndBuf);
    CloseHandle(BufHnd);        
    BufHnd = NULL;

    if (!DuplicateHandle(hServer, ad.BufHnd, hSelf, &BufHnd, 
                         0, false, DUPLICATE_SAME_ACCESS))
    {
        Error = GetLastError();
        CloseHandle(hServer);
        CloseHandle(Mutex); 
        return false;
    } else { 
        for (i = RD; i <= RTT; i++) { 
            if (!DuplicateHandle(hServer, ad.Signal[i], 
                                 hSelf, &Signal[i], 
                                 0, false, DUPLICATE_SAME_ACCESS))
            {
                Error = GetLastError();
                CloseHandle(hServer);
                CloseHandle(BufHnd); 
                CloseHandle(Mutex); 
                while (--i >= 0) CloseHandle(Signal[1]);
                return false;
            }
        }
    }
    CloseHandle(hServer);

    SndBuf = (socket_buf*)MapViewOfFile(BufHnd, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!SndBuf) { 
        Error = GetLastError();
        CloseHandle(BufHnd); 
        CloseHandle(Mutex); 
        for (i = RD; i <= RTT; i++) {  
            CloseHandle(Signal[i]);
        }
        return false;
    }
    RcvBuf = SndBuf+1;
    Error = ok;
    state = ss_open; 
    return true;
}

socket_handle_t local_win_socket::get_handle()
{
    return (socket_handle_t)-1;
}

#endif

END_FASTDB_NAMESPACE
