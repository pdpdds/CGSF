//-< UNISOCK.CPP >---------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:      8-Feb-97    K.A. Knizhnik  * / [] \ *
//                          Last update: 18-May-97    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Unix sockets  
//-------------------------------------------------------------------*--------*

#include "unisock.h"
#undef BYTE_ORDER

#ifdef VXWORKS
#include "fastdbShim.h"
#else
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#ifndef HPUX11
#include <sys/select.h>
#endif
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#if defined(_AIX)
#include <strings.h>
#endif  /* _AIX */
#include <stddef.h>
#include <assert.h>
#include <errno.h>

extern "C" {
#include <netdb.h>
}

#include <signal.h>
#endif // VXWORKS

BEGIN_FASTDB_NAMESPACE

const int MAX_HOST_NAME = 256;
const int GETHOSTBYNAME_BUF_SIZE = 1024;

#ifdef VXWORKS
char* unix_socket::unix_socket_dir = "/comp/socket";
#else
char* unix_socket::unix_socket_dir = "/tmp/";
#endif // VXWORKS

class unix_socket_library { 
  public: 
    unix_socket_library() { 
        static struct sigaction sigpipe_ignore; 
        sigpipe_ignore.sa_handler = SIG_IGN;
        sigaction(SIGPIPE, &sigpipe_ignore, NULL);
    }
};

static unix_socket_library unisock_lib;

bool unix_socket::open(int listen_queue_size)
{
    char hostname[MAX_HOST_NAME];
    unsigned short port;
    char* p;
#ifdef VXWORKS
    int proto = SOCK_STREAM;
#endif // VXWORKS

    assert(address != NULL);

    if ((p = strchr(address, ':')) == NULL 
        || unsigned(p - address) >= sizeof(hostname) 
        || sscanf(p+1, "%hu", &port) != 1) 
    {
        TRACE_IMSG(("Invalid address: %s\n", address));
        errcode = bad_address;
        return false;
    }
    memcpy(hostname, address, p - address);
    hostname[p - address] = '\0';
    
    create_file = false; 
    union { 
        sockaddr    sock;
        sockaddr_in sock_inet;
#ifdef VXWORKS
        struct sockaddr_un usock;
#endif
        char        name[MAX_HOST_NAME];
    } u;
    int len;

    if (domain == sock_local_domain) { 
#ifdef VXWORKS
        memset(&u.usock, 0, sizeof(struct sockaddr_un));
        u.usock.sun_family = AF_UNIX;
        proto = SOCK_SEQPACKET;
        u.usock.sun_len = len = sizeof (struct sockaddr_un);
        sprintf(u.usock.sun_path, "%s/0x%x", unix_socket_dir, port);
        TRACE_IMSG(("Sock %s %d\n", u.usock.sun_path,u.usock.sun_len));
        unlink(u.usock.sun_path); // remove file if existed
        create_file = true;
#else
        u.sock.sa_family = AF_UNIX;

        assert(strlen(unix_socket_dir) + strlen(address) 
               < MAX_HOST_NAME - offsetof(sockaddr,sa_data)); 
        
        len = offsetof(sockaddr,sa_data) + 
            sprintf(u.sock.sa_data, "%s%s.%u", unix_socket_dir, hostname, port);

        unlink(u.sock.sa_data); // remove file if existed
        create_file = true; 
#endif // VXWORKS
    } else {
        u.sock_inet.sin_family = AF_INET;
        if (*hostname && strcmp(hostname, "localhost") != 0) {
            struct hostent* hp;
#if defined(HAVE_GETHOSTBYNAME_R) && !defined(NO_PTHREADS)
            struct hostent ent;  // entry in hosts table
            char buf[GETHOSTBYNAME_BUF_SIZE];
            int h_err;
#if defined(__sun)
            if ((hp = gethostbyname_r(hostname, &ent, buf, sizeof buf, &h_err)) == NULL
#else
            if (gethostbyname_r(hostname, &ent, buf, sizeof buf, &hp, &h_err) != 0
                || hp == NULL
#endif
                || hp->h_addrtype != AF_INET)
#else
            if ((hp = gethostbyname(hostname)) == NULL || hp->h_addrtype != AF_INET) 
#endif
            {
                TRACE_IMSG(("Failed to get host by name: %s\n", errno));
                errcode = bad_address;
                return false;
            }
            memcpy(&u.sock_inet.sin_addr, hp->h_addr, 
                   sizeof u.sock_inet.sin_addr);
        } else {
            u.sock_inet.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        u.sock_inet.sin_port = htons(port);
        len = sizeof(sockaddr_in);      
    } 

#ifdef VXWORKS
    if ((fd = socket(u.sock.sa_family, proto, 0)) < 0) {
        errcode = errno;
        TRACE_IMSG(("Socket create is failed: %d", errcode));
        return false;
    }
#else
    if ((fd = socket(u.sock.sa_family, SOCK_STREAM, 0)) < 0) { 
        errcode = errno;
        TRACE_IMSG(("Socket create is failed: %d\n", errcode));
        return false;
    }
#endif // VXWORKS
    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof on);
#ifdef VXWORKS
    if (bind(fd, reinterpret_cast<struct sockaddr*>(&u.usock), len) < 0) {
        errcode = errno;
        TRACE_IMSG(("Socket bind is failed: %d", errcode));
        ::close(fd);
        return false;
    }
#else
    if (bind(fd, &u.sock, len) < 0) {
        errcode = errno;
        TRACE_IMSG(("Socket bind is failed: %d\n", errcode));
        ::close(fd);
        return false;
    }
#endif // VXWORKS
    if (listen(fd, listen_queue_size) < 0) {
        errcode = errno;
        TRACE_IMSG(("Socket listen is failed: %d\n", errcode));
        ::close(fd);
        return false;
    }
    errcode = ok;
    state = ss_open;
    return true;
}

char* unix_socket::get_peer_name()
{
    if (state != ss_open) { 
        errcode = not_opened;
        return NULL;
    }
    struct sockaddr_in insock;
    #if defined(__linux__) || (defined(__FreeBSD__) && __FreeBSD__ > 3) || defined(_AIX43) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(HPUX11) || defined(_SOCKLEN_T)
    socklen_t len = sizeof(insock);
#elif defined(_AIX41)
    size_t len = sizeof(insock);
#else
    int len = sizeof(insock);
#endif
    if (getpeername(fd, (struct sockaddr*)&insock, &len) != 0) { 
        errcode = errno;
        return NULL;
    }
    char* addr = inet_ntoa(insock.sin_addr);
    if (addr == NULL) { 
        errcode = errno;
        return NULL;
    }
    char* addr_copy = new char[strlen(addr)+1];
    strcpy(addr_copy, addr);
    errcode = ok;
    return addr_copy;
}

bool  unix_socket::is_ok()
{
    return errcode == ok;
}

void unix_socket::get_error_text(char* buf, size_t buf_size)
{
    char* msg; 
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
        msg = strerror(errcode);
    }
    strncpy(buf, msg, buf_size-1);
    buf[buf_size-1] = '\0';
}

socket_t* unix_socket::accept()
{
    int s;

    if (state != ss_open) { 
        errcode = not_opened;
        TRACE_IMSG(("Socket not openned\n"));
        return NULL;
    }

    while((s = ::accept(fd, NULL, NULL )) < 0 && errno == EINTR);

    if (s < 0) { 
        errcode = errno;
        TRACE_IMSG(("Socket accept failed: %d\n", errcode));
        return NULL;
    } else if (state != ss_open) {
        errcode = not_opened;
        TRACE_IMSG(("Socket not openned\n"));
        return NULL;
    } else { 
#if SOCK_NO_DELAY
        if (domain == sock_global_domain) { 
            int enabled = 1;
            if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char*)&enabled, 
                           sizeof enabled) != 0)
            {
                errcode = errno;
                TRACE_IMSG(("Failed to set socket options: %d\n", errcode));
                ::close(s);     
                return NULL;
            }
        }
#endif
#if SOCK_LINGER
        static struct linger l = {1, LINGER_TIME};
        if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char*)&l, sizeof l) != 0) { 
            TRACE_IMSG(("Failed to set socket options: %d\n", errno));
            errcode = invalid_access_mode; 
            ::close(s);
            return NULL; 
        }
#endif
#if SOCK_SNDBUF_SIZE 
        int size = SOCK_SNDBUF_SIZE;
        setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof size);
#endif
        errcode = ok;
        return new unix_socket(s); 
    }
}

bool unix_socket::cancel_accept() 
{
    bool result = close();
    // Wakeup listener
    delete socket_t::connect(address, domain, 1, 0);
    return result;
}    


bool unix_socket::connect(int max_attempts, time_t timeout)
{
    int   rc;
    char* p;
    struct utsname local_host;
    char hostname[MAX_HOST_NAME];
    unsigned short port;
#ifdef VXWORKS
    int proto = SOCK_STREAM;
#endif // VXWORKS

    assert(address != NULL);

    if ((p = strchr(address, ':')) == NULL 
        || unsigned(p - address) >= sizeof(hostname) 
        || sscanf(p+1, "%hu", &port) != 1) 
    {
        errcode = bad_address;
        TRACE_IMSG(("Invalid address: %s\n", address));
        return false;
    }
    memcpy(hostname, address, p - address);
    hostname[p - address] = '\0';
    
    create_file = false; 
    uname(&local_host);

    if (domain == sock_local_domain || (domain == sock_any_domain && 
        (strcmp(hostname, local_host.nodename) == 0
         || strcmp(hostname, "localhost") == 0)))
    {
        // connect UNIX socket
        union { 
            sockaddr sock;
            char     name[MAX_HOST_NAME];
        } u;
        u.sock.sa_family = AF_UNIX;

#ifdef VXWORKS
        if ( domain == sock_local_domain )
            proto = SOCK_SEQPACKET;
#endif
        assert(strlen(unix_socket_dir) + strlen(address) 
               < MAX_HOST_NAME - offsetof(sockaddr,sa_data)); 
 
#ifdef VXWORKS
        int len = offsetof(sockaddr,sa_data) +
            sprintf(u.sock.sa_data, "%s/0x%x", unix_socket_dir, port);
#else            
        int len = offsetof(sockaddr,sa_data) +
            sprintf(u.sock.sa_data, "%s%s.%u", unix_socket_dir, hostname, port);
#endif // VXWORKS
        
        while (true) {
#ifdef VXWORKS
            if ((fd = socket(u.sock.sa_family, proto, 0)) < 0) {
                errcode = errno;
               TRACE_IMSG(("Failed to create socket: %d", errcode));
               return false;
            }
#else
            if ((fd = socket(u.sock.sa_family, SOCK_STREAM, 0)) < 0) { 
                errcode = errno;
                TRACE_IMSG(("Failed to create socket: %d\n", errcode));
                return false;
            }
#endif // VXWORKS
            do { 
                rc = ::connect(fd, &u.sock, len);
            } while (rc < 0 && errno == EINTR);
            
            if (rc < 0) { 
                errcode = errno;
                ::close(fd);
                if (errcode == ENOENT || errcode == ECONNREFUSED) {
                    if (--max_attempts > 0) { 
                        sleep(timeout);
                    } else { 
                        TRACE_IMSG(("All attempts to establish connection are failed\n"));
                        break;
                    }
                } else {
                    TRACE_IMSG(("Failed to establish connection: %d\n", errcode));
                    return false;
                }
            } else {
                errcode = ok;
                state = ss_open;
                return true;
            }
        }
    } else { 
        sockaddr_in sock_inet;
        struct hostent* hp;
#if defined(HAVE_GETHOSTBYNAME_R) && !defined(NO_PTHREADS)
        struct hostent ent;  // entry in hosts table
        char buf[GETHOSTBYNAME_BUF_SIZE];
        int h_err;
#if defined(__sun)
        if ((hp = gethostbyname_r(hostname, &ent, buf, sizeof buf, &h_err)) == NULL
#else
        if (gethostbyname_r(hostname, &ent, buf, sizeof buf, &hp, &h_err) != 0
            || hp == NULL
#endif
            || hp->h_addrtype != AF_INET)
#else
        if ((hp = gethostbyname(hostname)) == NULL || hp->h_addrtype != AF_INET) 
#endif
        {
            TRACE_IMSG(("Host name can not be resolved: %d\n", errno));
            errcode = bad_address;
            return false;
        }
        sock_inet.sin_family = AF_INET;  
        sock_inet.sin_port = htons(port);
        //fprintf(stderr, "Try to connect to '%s' port %d\n", hostname, port);
        
        while (true) {
            for (int i = 0; hp->h_addr_list[i] != NULL; i++) { 
                memcpy(&sock_inet.sin_addr, hp->h_addr_list[i],
                       sizeof sock_inet.sin_addr);
                if ((fd = socket(sock_inet.sin_family, SOCK_STREAM, 0)) < 0) { 
                    errcode = errno;
                    TRACE_IMSG(("Failed to create socket: %d\n", errcode));
                    return false;
                }
                do { 
                    rc = ::connect(fd,(sockaddr*)&sock_inet,sizeof(sock_inet));
                } while (rc < 0 && errno == EINTR);
                
                if (rc < 0) { 
                    errcode = errno;
                    ::close(fd);
                    if (errcode != ENOENT && errcode != ECONNREFUSED) {
                        TRACE_IMSG(("Failed to establish connection: %d\n", errcode));
                        return false;
                    }
                } else {
#if SOCK_NO_DELAY
                    int enabled = 1;
                    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, 
                                   (char*)&enabled, sizeof enabled) != 0)
                    {
                        errcode = errno;
                        TRACE_IMSG(("Failed to set socket option TCP_NODELAY: %d\n", errcode));
                        ::close(fd);    
                        return false;
                    }
#endif
#if SOCK_LINGER
                    static struct linger l = {1, LINGER_TIME};
                    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, (char*)&l, sizeof l) != 0) { 
                        errcode = errno;
                        TRACE_IMSG(("Failed to set socket option SOL_SOCKET: %d\n", errcode));
                        ::close(fd);
                        return NULL; 
                    }
#endif
                    errcode = ok;
                    state = ss_open;
                    return true;
                }
            }
            if (--max_attempts > 0) { 
                sleep(timeout);
            } else { 
                TRACE_IMSG(("All attempts to establish connection are failed\n"));
                break;
            }
        }
    }
    errcode = connection_failed;
    return false;
}

int unix_socket::read(void* buf, size_t min_size, size_t max_size, 
                      time_t timeout)
{ 
    size_t size = 0;
    time_t start = 0;
    if (state != ss_open) { 
        errcode = not_opened;
        TRACE_IMSG(("Socket is not openned\n"));
        return -1;
    }
    if (timeout != WAIT_FOREVER) { 
        start = time(NULL); 
    }
    do { 
        ssize_t rc; 
        if (timeout != WAIT_FOREVER) { 
            fd_set events;
            struct timeval tm;
            FD_ZERO(&events);
            FD_SET(fd, &events);
            tm.tv_sec = timeout;
            tm.tv_usec = 0;
            while ((rc = select(fd+1, &events, NULL, NULL, &tm)) < 0 
                   && errno == EINTR);
            if (rc < 0) { 
                errcode = errno;
                TRACE_IMSG(("Socket select is failed: %d\n", errcode));
                return -1;
            }
            if (rc == 0) {
                return size;
            }
            time_t now = time(NULL);
            timeout = start + timeout >= now ? timeout + start - now : 0;  
        }
        while ((rc = ::read(fd, (char*)buf + size, max_size - size)) < 0 
               && errno == EINTR); 
        if (rc < 0) { 
            errcode = errno;
            TRACE_IMSG(("Socket read is failed: %d\n", errcode));
            return -1;
        } else if (rc == 0) {
            errcode = broken_pipe;
            TRACE_IMSG(("Socket is disconnected\n"));
            return -1; 
        } else { 
            size += rc; 
        }
    } while (size < min_size); 

    return (int)size;
}

        
bool unix_socket::write(void const* buf, size_t size, time_t timeout)
{ 
    time_t start = 0;
    if (state != ss_open) { 
        errcode = not_opened;
        TRACE_IMSG(("Socket is not openned\n"));
        return -1;
    }
    if (timeout != WAIT_FOREVER) { 
        start = time(NULL); 
    }
    
    do { 
        ssize_t rc; 
        if (timeout != WAIT_FOREVER) { 
            fd_set events;
            struct timeval tm;
            FD_ZERO(&events);
            FD_SET(fd, &events);
            tm.tv_sec = timeout;
            tm.tv_usec = 0;
            while ((rc = select(fd+1, NULL, &events, NULL, &tm)) < 0 
                   && errno == EINTR);
            if (rc <= 0) { 
                errcode = errno;
                TRACE_IMSG(("Socket select is failed: %d\n", errcode));
                return false;
            }
            time_t now = time(NULL);
            timeout = start + timeout >= now ? timeout + start - now : 0;  
        }
        while ((rc = ::write(fd, (char*)buf, size)) < 0 && errno == EINTR); 
        if (rc < 0) { 
            errcode = errno;
            TRACE_IMSG(("Socket write is failed: %d\n", errcode));
            return false;
        } else if (rc == 0) {
            errcode = broken_pipe;
            TRACE_IMSG(("Socket is disconnected\n"));
            return false; 
        } else { 
            buf = (char*)buf + rc; 
            size -= rc; 
        }
    } while (size != 0); 

    //
    // errcode is not assigned 'ok' value beacuse write function 
    // can be called in parallel with other socket operations, so
    // we want to preserve old error code here.
    //
    return true;
}
        
bool unix_socket::close()
{
    if (state != ss_close) {
        state = ss_close;
        if (::close(fd) == 0) {
            errcode = ok;
            return true;
        } else { 
            errcode = errno;
            TRACE_IMSG(("Socket close is failed: %d\n", errcode));
            return false;
        }
    }
    errcode = ok;
    return true;
}

bool unix_socket::shutdown()
{
    if (state == ss_open) { 
        state = ss_shutdown;
        int rc = ::shutdown(fd, 2);
        if (rc != 0) { 
            errcode = errno;
            TRACE_IMSG(("Socket shutdown is failed: %d\n", errcode));
            return false;
        } 
    } 
    return true;
}

unix_socket::~unix_socket()
{
    close();
    if (create_file) { 
        char name[MAX_HOST_NAME];
        char* p = strrchr(address, ':');
        sprintf(name, "%s%.*s.%s", unix_socket_dir, (int)(p - address), address, p+1);
        unlink(name);
    }
    delete[] address;
}

unix_socket::unix_socket(const char* addr, socket_domain domain)
{ 
    address = new char[strlen(addr)+1]; 
    strcpy(address, addr);
    this->domain = domain;
    create_file = false;
    errcode = ok;
}

unix_socket::unix_socket(int new_fd) 
{ 
    fd = new_fd; 
    address = NULL; 
    create_file = false;
    state = ss_open; 
    errcode = ok;
}

socket_t* socket_t::create_local(char const* address, int listen_queue_size)
{
    unix_socket* sock = new unix_socket(address, sock_local_domain);
    sock->open(listen_queue_size); 
    return sock;
}

socket_t* socket_t::create_global(char const* address, int listen_queue_size)
{
    unix_socket* sock = new unix_socket(address, sock_global_domain);
    sock->open(listen_queue_size); 
    return sock;
}

socket_t* socket_t::connect(char const* address, 
                            socket_domain domain, 
                            int max_attempts, 
                            time_t timeout)
{
    unix_socket* sock = new unix_socket(address, domain);
    sock->connect(max_attempts, timeout); 
    return sock;
}

socket_handle_t unix_socket::get_handle()
{
    return fd;
}

END_FASTDB_NAMESPACE
