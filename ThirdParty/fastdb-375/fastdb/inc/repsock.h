//-< REPSOCK.H >-----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:      6-May-2003  K.A. Knizhnik  * / [] \ *
//                          Last update:  6-Apr-2003  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Replication socket
//-------------------------------------------------------------------*--------*

#ifndef __REPSOCK_H__
#define __REPSOCK_H__

#include "sockio.h"

BEGIN_FASTDB_NAMESPACE


//
// Socket use for CLI level replication
//
class FASTDB_DLL_ENTRY replication_socket_t : public socket_t {
  public:
    virtual int       read(void* buf, size_t min_size, size_t max_size, time_t timeout);
    virtual bool      write(void const* buf, size_t size, time_t timeout);

    virtual bool      is_ok();
    virtual void      get_error_text(char* buf, size_t buf_size);

    virtual bool      shutdown();

    virtual bool      close();

    virtual void      handleError(int socket, const char* operation, const char* error);

    static replication_socket_t*  connect(char const* addresses[],
                                          int n_addresses, 
                                          int max_attempts = DEFAULT_CONNECT_MAX_ATTEMPTS,
                                          time_t timeout = DEFAULT_RECONNECT_TIMEOUT);

    // 
    // Not implemented for replication socket
    //
    virtual socket_t* accept();
    virtual bool      cancel_accept();
    virtual char*     get_peer_name();
    virtual socket_handle_t get_handle();

    enum { 
        MaxSockets = 8
    };


    ~replication_socket_t();

  protected:
    replication_socket_t(char const* addresses[], int n_adresses, int max_attempts, time_t timeout);

    socket_t**   sockets;
    int          n_sockets;
    bool         succeed;
};

END_FASTDB_NAMESPACE

#endif



