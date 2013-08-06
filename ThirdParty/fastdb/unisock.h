//-< UNISOCK.H >-----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1997  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:      7-Jan-97    K.A. Knizhnik  * / [] \ *
//                          Last update:  7-Jan-97    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Unix socket 
//-------------------------------------------------------------------*--------*

#ifndef __UNISOCK_H__
#define __UNISOCK_H__

#include "sockio.h"

BEGIN_FASTDB_NAMESPACE

class unix_socket : public socket_t { 
  protected: 
    int           fd; 
    int           errcode;     // error code of last failed operation 
    char*         address;     // host address
    socket_domain domain;      // Unix domain or INET socket
    bool          create_file; // Unix domain sockets use files for connection

    enum error_codes { 
        ok = 0,
        not_opened = -1,
        bad_address = -2,
        connection_failed = -3,
        broken_pipe = -4, 
        invalid_access_mode = -5
    };

  public: 
    //
    // Directory for Unix Domain socket files. This directory should be 
    // either empty or be terminated with "/". Dafault value is "/tmp/"
    //
    static char* unix_socket_dir; 

    bool      open(int listen_queue_size);
    bool      connect(int max_attempts, time_t timeout);

    int       read(void* buf, size_t min_size, size_t max_size, time_t timeout);
    bool      write(void const* buf, size_t size, time_t timeout);

    bool      is_ok(); 
    bool      shutdown();
    bool      close();
    char*     get_peer_name();
    void      get_error_text(char* buf, size_t buf_size);

    socket_t* accept();
    bool      cancel_accept();
    
    socket_handle_t get_handle();

    unix_socket(const char* address, socket_domain domain); 
    unix_socket(int new_fd);

    ~unix_socket();
};

END_FASTDB_NAMESPACE

#endif





