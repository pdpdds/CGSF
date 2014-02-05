//-< CLI.CPP >-------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     13-Jan-2000  K.A. Knizhnik  * / [] \ *
//                          Last update: 13-Jan-2000  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Call level interface client part implementation
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "stdtp.h"
#include "sockio.h"
#include "repsock.h"
#include "sync.h"
#include "cli.h"
#include "cliproto.h"

#include <wchar.h>
#include <ctype.h>

BEGIN_FASTDB_NAMESPACE

struct parameter_binding { 
    parameter_binding* next;
    char* name;
    int   var_type;
    int   var_len;
    void* var_ptr;

    ~parameter_binding() { 
        delete[] name;
    }
};

struct column_binding { 
    column_binding* next;
    char* name;
    int   var_type;
    int*  var_len;
    void* var_ptr;
    void* arr_ptr;
    int   arr_len;
    cli_column_get_ex get_fnc;   
    cli_column_set_ex set_fnc;   
    void* user_data;

    ~column_binding() { 
        delete[] name;
    }
};

struct session_desc;

#define DEFAULT_BUF_SIZE 256

struct statement_desc {
    int                id;
    statement_desc*    next;
    char*              stmt;
    column_binding*    columns;
    parameter_binding* params;
    session_desc*      session;
    bool               for_update;
    bool               updated;
    bool               prepared;   
    bool               autoincrement;
    cli_oid_t          oid;
    int                stmt_len;
    int                n_params;
    int                n_columns;
    int                columns_len;
    char*              buf;
    size_t             buf_size;

    void deallocate() { 
        delete[] stmt;
        column_binding *cb, *next_cb;
        for (cb = columns; cb != NULL; cb = next_cb) { 
            next_cb = cb->next;
            delete cb;
        }
        if (buf != NULL) { 
            delete[] buf;
            buf_size = 0;
            buf = NULL;
        }
        parameter_binding *pb, *next_pb;
        for (pb = params; pb != NULL; pb = next_pb) { 
            next_pb = pb->next;
            delete pb;
        }
    }
    statement_desc(int id, statement_desc* next) { 
        this->id = id;
        this->next = next;
        buf = NULL;
        buf_size = 0;
    }
    statement_desc() {}
};    

struct session_desc { 
    int             id;
    session_desc*   next;    
    socket_t*       sock;
    statement_desc* stmts;
    
    session_desc(int id, session_desc* next) { 
        this->id = id;
        this->next = next;
    }
    session_desc() {}
};

template<class T>
class descriptor_table { 
  protected: 
    T**         table;
    T*          free_desc;
    int         descriptor_table_size;
    dbMutex     mutex;
    
  public:
    descriptor_table() { 
        int i;
        descriptor_table_size = 16;
        table = new T*[descriptor_table_size];
        T* next = NULL;
        for (i = 0; i < descriptor_table_size; i++) { 
            table[i] = next = new T(i, next);
        }
        free_desc = next;
    }

    ~descriptor_table() {
        for (int i = 0; i < descriptor_table_size; i++) {
            delete table[i];
        }
        delete[] table;
    }

    T* get(int desc) {
        dbCriticalSection cs(mutex);
        return (desc >= descriptor_table_size) ? (T*)0 : table[desc];
    }

    T* allocate() { 
        dbCriticalSection cs(mutex);
        if (free_desc == NULL) {
            int i, n;
            T** desc = new T*[descriptor_table_size * 2];
            memcpy(desc, table, descriptor_table_size*sizeof(T*));
            delete[] table;
            table = desc;           
            T* next = NULL;
            for (i = descriptor_table_size, n = i*2; i < n; i++) { 
                table[i] = next = new T(i, next);
            }
            free_desc = next;
            descriptor_table_size = n;
        }
        T* desc = free_desc;
        free_desc = desc->next;
        return desc;
    }

    void deallocate(T* desc) { 
        dbCriticalSection cs(mutex);
        desc->next = free_desc;
        free_desc = desc;
    }
};

static descriptor_table<session_desc>   sessions;
static descriptor_table<statement_desc> statements;

END_FASTDB_NAMESPACE
USE_FASTDB_NAMESPACE

int cli_open(char const* server_url, 
             int         max_connect_attempts,
             int         reconnect_timeout_sec)
{
    socket_t* sock;
    int n_addresses = 1;
    char const* start = server_url;
    char const* end;
    while ((end = strchr(start, ',')) != NULL) { 
        start = end + 1;
        n_addresses += 1;
    }
    if (n_addresses == 1) { 
        sock = socket_t::connect(server_url,
                                 socket_t::sock_any_domain,
                                 max_connect_attempts,
                                 reconnect_timeout_sec);
    } else { 
        char** addresses = new char*[n_addresses];
        start = server_url;
        for (int i = 0; i < n_addresses; i++) { 
            end = strchr(start, ',');
            if (end == NULL) { 
                end = start + strlen(start);
            }
            int len = (int)(end - start);
            char* addr = new char[len+1];
            memcpy(addr, start, len);
            addr[len] = '\0';
            start = end + 1;
            addresses[i] = addr;
        }
        sock = replication_socket_t::connect((char const**)addresses,
                                             n_addresses, 
                                             max_connect_attempts,
                                             reconnect_timeout_sec);
        while (--n_addresses >= 0) { 
            delete[] addresses[n_addresses];
        }
        delete[] addresses;
    }
    if (!sock->is_ok()) { 
        delete sock;
        return cli_connection_refused;
    }
    session_desc* session = sessions.allocate();
    session->sock = sock;
    session->stmts = NULL;
    return session->id;
}


int cli_close(int session)
{
    statement_desc *stmt, *next;
    session_desc* s = sessions.get(session);
    if (s == NULL) { 
        return cli_bad_descriptor;
    }
    cli_request req;
    req.length = sizeof(req);
    req.cmd = cli_cmd_close_session;
    req.pack();
    int result = cli_ok;
    if (!s->sock->write(&req, sizeof req)) { 
        result = cli_network_error;
    }
    delete s->sock;
    s->sock = NULL;     
    for (stmt = s->stmts; stmt != NULL; stmt = next) {  
        next = stmt->next;
        stmt->deallocate();
        statements.deallocate(stmt);
    }
    sessions.deallocate(s);
    return result;
}

int cli_statement(int session, char const* stmt_str)
{
    session_desc* s = sessions.get(session);
    if (s == NULL) { 
        return cli_bad_descriptor;
    }
    statement_desc* stmt = statements.allocate();
    stmt->stmt = new char[strlen(stmt_str)+1];
    stmt->columns = NULL;
    stmt->params = NULL;
    stmt->session = s;
    stmt->for_update = 0;
    stmt->prepared = false;
    stmt->n_params = 0;
    stmt->n_columns = 0;
    stmt->columns_len = 0;
    stmt->oid = 0;
    stmt->next = s->stmts;
    stmt->updated = false;
    s->stmts = stmt;
    char const* p = stmt_str;
    char* dst = stmt->stmt;
    parameter_binding** last = &stmt->params;
    while (*p != '\0') { 
        if (*p == '\'') { 
            do { 
                do { 
                    *dst++ = *p++;
                } while (*p != '\0' && *p != '\'');
                *dst++ = *p;
                if (*p == '\0') { 
                    *last = NULL;
                    stmt->deallocate();
                    statements.deallocate(stmt);
                    return cli_bad_statement;
                }
            } while (*++p == '\'');
        } else if (*p == '%') { 
            stmt->n_params += 1;
            char const* q = p++;
            while (isalnum((unsigned char)*p) || *p == '_') p += 1;
            if (*p == '%') { 
                *last = NULL;
                stmt->deallocate();
                statements.deallocate(stmt);
                return cli_bad_statement;
            }
            parameter_binding* pb = new parameter_binding;
            int len = (int)(p - q);
            pb->name = new char[len+1];
            memcpy(pb->name, q, len);
            pb->name[len] = '\0';
            *last = pb;
            last = &pb->next;
            pb->var_ptr = NULL;
            *dst++ = '\0';
        } else { 
            *dst++ = *p++;
        }
    }
    if (dst == stmt->stmt || *(dst-1) != '\0') { 
        *dst++ = '\0';
    }
    stmt->stmt_len = (int)(dst - stmt->stmt);
    *last = NULL;
    return stmt->id;
}

int cli_parameter(int         statement,
                  char const* param_name, 
                  int         var_type,
                  void*       var_ptr)
{
    if (var_type != cli_rectangle 
        && var_type != cli_wstring
        && var_type != cli_pwstring
        && ((unsigned)var_type >= cli_array_of_oid || var_type == cli_decimal)) 
    {
        return cli_unsupported_type;
    }
    statement_desc* s = statements.get(statement);
    if (s == NULL) { 
        return cli_bad_descriptor;
    }
    s->prepared = false;
    for (parameter_binding* pb = s->params; pb != NULL; pb = pb->next) { 
        if (strcmp(pb->name, param_name) == 0) { 
            pb->var_ptr  = var_ptr;
            pb->var_type = var_type;
            return cli_ok;
        }
    }
    return cli_parameter_not_found;
}

int cli_column(int         statement,
               char const* column_name, 
               int         var_type, 
               int*        var_len, 
               void*       var_ptr)
{
    statement_desc* s = statements.get(statement);
    if (s == NULL) { 
        return cli_bad_descriptor;
    }
    s->prepared = false;
    column_binding* cb = new column_binding;
    int len = (int)strlen(column_name) + 1;
    cb->name = new char[len];
    s->columns_len += len;
    cb->next = s->columns;
    s->columns = cb;
    s->n_columns += 1;
    strcpy(cb->name, column_name);
    cb->var_type = var_type;
    cb->var_len = var_len;
    cb->var_ptr = var_ptr;
    cb->set_fnc = NULL;
    cb->get_fnc = NULL;
    return cli_ok;
}

int cli_array_column(int            statement,
                     char const*    column_name,
                     int            var_type,
                     void*          var_ptr,
                     cli_column_set set,
                     cli_column_get get)
{
    return cli_array_column_ex(statement, column_name, var_type, var_ptr, 
                               (cli_column_set_ex)set, (cli_column_get_ex)get, NULL);
}

int cli_array_column_ex(int               statement,
                        char const*       column_name, 
                        int               var_type,
                        void*             var_ptr,
                        cli_column_set_ex set,
                        cli_column_get_ex get,
                        void*             user_data)
{
    statement_desc* s = statements.get(statement);
    if (s == NULL) { 
        return cli_bad_descriptor;
    }
    if (!((var_type >= cli_asciiz && var_type <= cli_array_of_string) 
          || (var_type >= cli_wstring && var_type <= cli_array_of_wstring))) 
    {
        return cli_unsupported_type;
    }
    s->prepared = false;
    column_binding* cb = new column_binding;
    int len = (int)strlen(column_name) + 1;
    cb->name = new char[len];
    s->columns_len += len;
    cb->next = s->columns;
    s->columns = cb;
    s->n_columns += 1;
    strcpy(cb->name, column_name);
    cb->var_type = var_type;
    cb->var_len = NULL;
    cb->var_ptr = var_ptr;
    cb->set_fnc = set;
    cb->get_fnc = get;
    cb->user_data = user_data;
    return cli_ok;
}
   
int cli_fetch_ex(int statement, int cursor_type, cli_cardinality_t* n_fetched_records)
{
    int rc = cli_fetch(statement,  cursor_type);
    if (rc >= 0) { 
        *n_fetched_records = rc;
        return cli_ok;
    }
    return rc;
}

int cli_fetch(int statement, int for_update)
{
    parameter_binding* pb;
    column_binding*    cb;
    statement_desc* stmt = statements.get(statement);
    char *p, *s;

    if (stmt == NULL) { 
        return cli_bad_descriptor;
    }
    stmt->for_update = for_update;
    size_t msg_size = sizeof(cli_request) + 1;
    for (pb = stmt->params; pb != NULL; pb = pb->next) { 
        if (pb->var_ptr == NULL) { 
            return cli_unbound_parameter;
        }
        if (pb->var_type == cli_asciiz) { 
            msg_size += strlen((char*)pb->var_ptr) + 1;
        } else if (pb->var_type == cli_pasciiz) { 
            msg_size += strlen(*(char**)pb->var_ptr) + 1;
        } else if (pb->var_type == cli_wstring) { 
            msg_size += (wcslen((wchar_t*)pb->var_ptr) + 1)*sizeof(wchar_t);
        } else if (pb->var_type == cli_pwstring) { 
            msg_size += (wcslen(*(wchar_t**)pb->var_ptr) + 1)*sizeof(wchar_t);
        } else { 
            msg_size += sizeof_type[pb->var_type];
        }
    }
    stmt->oid = 0;
    if (!stmt->prepared) { 
        msg_size += 4 + stmt->stmt_len + stmt->n_params;
        msg_size += stmt->columns_len + stmt->n_columns;
    }
    dbSmallBuffer buf(msg_size);
    p = buf;
    cli_request* req = (cli_request*)p;
    req->length  = (int)msg_size;
    req->cmd     = stmt->prepared 
        ? cli_cmd_execute : cli_cmd_prepare_and_execute;
    req->stmt_id = statement;
    req->pack();
    p += sizeof(cli_request);

    if (!stmt->prepared) { 
        *p++ = stmt->n_params;
        *p++ = stmt->n_columns;
        p = pack2(p, stmt->stmt_len + stmt->n_params);
        pb = stmt->params;
        char* end = p + stmt->stmt_len + stmt->n_params;
        char* src = stmt->stmt;
        while (p < end) { 
            while ((*p++ = *src++) != '\0');
            if (pb != NULL) { 
                *p++ = pb->var_type == cli_pasciiz ? cli_asciiz 
                    : pb->var_type == cli_pwstring ? cli_wstring : pb->var_type;
                pb = pb->next;
            }
        }
        for (cb = stmt->columns; cb != NULL; cb = cb->next) { 
            *p++ = cb->var_type;
            s = cb->name;
            while ((*p++ = *s++) != '\0');
        }
    }   
    *p++ = for_update;
    for (pb = stmt->params; pb != NULL; pb = pb->next) { 
        switch (pb->var_type) { 
          case cli_rectangle:
            p = pack_rectangle(p, (cli_rectangle_t*)pb->var_ptr);
            continue;
          case cli_asciiz:
            s = (char*)pb->var_ptr;
            while ((*p++ = *s++) != '\0');
            continue;
          case cli_pasciiz:
            s = *(char**)pb->var_ptr;
            while ((*p++ = *s++) != '\0');
            continue;
          case cli_wstring:
          {
              wchar_t* src = (wchar_t*)pb->var_ptr;
              wchar_t* dst = (wchar_t*)p;
              while ((*dst++ = *src++) != 0);
              p = (char*)dst;
              continue;
          }
          case cli_pwstring:
          {
              wchar_t* src = *(wchar_t**)pb->var_ptr;
              wchar_t* dst = (wchar_t*)p;
              while ((*dst++ = *src++) != 0);
              p = (char*)dst;
              continue;
          }
          default:
            switch (sizeof_type[pb->var_type]) { 
              case 1:
                *p++ = *(char*)pb->var_ptr;
                continue;
              case 2:
                p = pack2(p, *(int2*)pb->var_ptr);
                continue;
              case 4:
                p = pack4(p, *(int4*)pb->var_ptr);
                continue;
              case 8:
                p = pack8(p, *(db_int8*)pb->var_ptr);
                continue;
            }
        }
    }
    assert(msg_size == (size_t)(p - buf.base()));
    if (!stmt->session->sock->write(buf, msg_size)) { 
        return cli_network_error;
    }
    int4 response;
    if (!stmt->session->sock->read(&response, sizeof response)) { 
        return cli_network_error;
    }
    unpack4(response);
    if (response >= 0) { 
        stmt->prepared = true;
    }
    return response;
}

static int cli_send_columns(int statement, int cmd)
{
    statement_desc* s = statements.get(statement);
    column_binding* cb;
    if (s == NULL) { 
        return cli_bad_descriptor;
    }
    size_t msg_size = sizeof(cli_request);
    if (cmd == cli_cmd_update) { 
        if (!s->prepared) { 
            return cli_not_fetched;
        }
        if (s->oid == 0) { 
            return cli_not_found;
        }
        if (s->updated) {
            return cli_already_updated;
        } 
        if (!s->for_update) { 
            return cli_not_update_mode;
        }
    } else { 
        if (!s->prepared) { 
            cmd = cli_cmd_prepare_and_insert;
            msg_size += 1 + s->stmt_len + s->n_columns + s->columns_len;
        }
    }
    s->autoincrement = false;
    for (cb = s->columns; cb != NULL; cb = cb->next) { 
        if (cb->get_fnc != NULL) { 
            cb->arr_ptr = cb->get_fnc(cb->var_type, cb->var_ptr, &cb->arr_len, 
                                      cb->name, statement, cb->user_data);
            int len = cb->arr_len;
            msg_size += 4;
            if (cb->var_type == cli_array_of_string) {
                char** p = (char**)cb->arr_ptr;
                while (--len >= 0) { 
                    msg_size += strlen(*p++) + 1;
                }
            } else if (cb->var_type == cli_array_of_wstring) {
                wchar_t** p = (wchar_t**)cb->arr_ptr;
                while (--len >= 0) { 
                    msg_size += (wcslen(*p++) + 1)*sizeof(wchar_t);
                }
            } else if (cb->var_type == cli_wstring || cb->var_type == cli_pwstring) {
                msg_size += len * sizeof(wchar_t);
            } else if (cb->var_type >= cli_array_of_oid) {
                msg_size += len * sizeof_type[cb->var_type - cli_array_of_oid];
            } else { 
                msg_size += len;
            }
        } else { 
            switch (cb->var_type) { 
              case cli_autoincrement:
                s->autoincrement = true;
                break;
              case cli_asciiz:
                msg_size += 4 + (strlen((char*)cb->var_ptr) + 1);
                break;
              case cli_pasciiz:
                msg_size += 4 + (strlen(*(char**)cb->var_ptr) + 1);
                break;
              case cli_wstring:
                msg_size += 4 + (wcslen((wchar_t*)cb->var_ptr) + 1)*sizeof(wchar_t);
                break;
              case cli_pwstring:
                msg_size += 4 + (wcslen(*(wchar_t**)cb->var_ptr) + 1)*sizeof(wchar_t);
                break;
              case cli_array_of_string:
              { 
                char** p = (char**)cb->var_ptr;
                int len;
                msg_size += 4;
                for (len = *cb->var_len; --len >= 0;) { 
                    msg_size += (strlen(*p++) + 1);
                }
                break;
              }
              case cli_array_of_wstring:
              { 
                wchar_t** p = (wchar_t**)cb->var_ptr;
                int len;
                msg_size += 4;
                for (len = *cb->var_len; --len >= 0;) { 
                    msg_size += (wcslen(*p++) + 1)*sizeof(wchar_t);
                }
                break;
              }
              default:
                if (cb->var_type >= cli_array_of_oid && cb->var_type < cli_array_of_string) {
                    msg_size += 4 + *cb->var_len * sizeof_type[cb->var_type-cli_array_of_oid];
                } else { 
                    msg_size += sizeof_type[cb->var_type];
                }
            }
        }
    }
    dbSmallBuffer buf(msg_size);
    char* p = buf;
    cli_request* req = (cli_request*)p;
    req->length  = (int)msg_size;
    req->cmd     = cmd;
    req->stmt_id = statement;
    req->pack();
    p += sizeof(cli_request);
    if (cmd == cli_cmd_prepare_and_insert) { 
        char* cmd = s->stmt;
        while ((*p++ = *cmd++) != '\0');
        *p++ = s->n_columns;
        for (cb = s->columns; cb != NULL; cb = cb->next) { 
            char* src = cb->name;
            *p++ = cb->var_type;
            while ((*p++ = *src++) != '\0');
        }       
    }
    for (cb = s->columns; cb != NULL; cb = cb->next) { 
        int n = 0;
        char* src;
        if (cb->get_fnc != NULL) { 
            src = (char*)cb->arr_ptr;
            n = cb->arr_len;
        } else { 
            src = (char*)cb->var_ptr;
            if (cb->var_type >= cli_array_of_oid && (cb->var_type <= cli_array_of_string || cb->var_type == cli_array_of_wstring)) { 
                n = *cb->var_len;       
            }
        }
        if (cb->var_type >= cli_array_of_oid && (cb->var_type <= cli_array_of_string || cb->var_type == cli_array_of_wstring)) { 
            p = pack4(p, n);
            if (cb->var_type == cli_array_of_string) {
                while (--n >= 0) {
                    strcpy(p, *(char**)src);
                    p += strlen(p) + 1;
                    src += sizeof(char*);
                }
            } else if (cb->var_type == cli_array_of_wstring) {
                while (--n >= 0) {
                    wcscpy((wchar_t*)p, *(wchar_t**)src);
                    p += (wcslen((wchar_t*)p) + 1)*sizeof(wchar_t);
                    src += sizeof(wchar_t*);
                }
            } else {
                switch (sizeof_type[cb->var_type-cli_array_of_oid]) { 
                  case 2:
                    while (--n >= 0) { 
                        p = pack2(p, src);
                        src += 2;
                    }
                    break;
                  case 4:
                    while (--n >= 0) { 
                        p = pack4(p, src);
                        src += 4;
                    }
                    break;
                  case 8:
                    while (--n >= 0) { 
                        p = pack8(p, src);
                        src += 8;
                    }
                    break;
                  default:
                    memcpy(p, src, n);
                    p += n;
                }
            }
        } else if (cb->var_type == cli_asciiz) {                
            p = pack4(p, (int)strlen(src)+1);
            while ((*p++ = *src++) != 0);
        } else if (cb->var_type == cli_pasciiz) { 
            src = *(char**)src;
            p = pack4(p, (int)strlen(src)+1);
            while ((*p++ = *src++) != 0);
        } else if (cb->var_type == cli_wstring) {                
            wchar_t* body = (wchar_t*)src;
            p = pack4(p, (int)wcslen(body)+1);
            wchar_t* dst = (wchar_t*)p;
            while ((*dst++ = *body++) != 0);
            p = (char*)dst;
        } else if (cb->var_type == cli_pwstring) {                
            wchar_t* body = *(wchar_t**)src;
            p = pack4(p, (int)wcslen(body)+1);
            wchar_t* dst = (wchar_t*)p;
            while ((*dst++ = *body++) != 0);
            p = (char*)dst;
        } else if (cb->var_type == cli_rectangle) {
            p = pack_rectangle(p, (cli_rectangle_t*)src);
        } else if (cb->var_type != cli_autoincrement) { 
            switch (sizeof_type[cb->var_type]) { 
              case 2:
                p = pack2(p, src);
                break;
              case 4:
                p = pack4(p, src);
                break;
              case 8:
                p = pack8(p, src);
                break;
              default:
                *p++ = *src;
            }
        }
    }
    assert((size_t)(p - buf.base()) == msg_size);
    if (!s->session->sock->write(buf, msg_size)) { 
        return cli_network_error;
    }
    return cli_ok;
}

int cli_insert(int statement, cli_oid_t* oid)
{
    int rc = cli_send_columns(statement, cli_cmd_insert);
    if (rc == cli_ok) { 
        char buf[sizeof(cli_oid_t) + 8];
        statement_desc* s = statements.get(statement);
        if (!s->session->sock->read(buf, sizeof buf)) { 
            rc = cli_network_error;
        } else { 
            rc = unpack4(buf);
            s->prepared = true;
            s->oid = unpack_oid(buf + 8);
            if (oid != NULL) { 
                *oid = s->oid;
            }
           if (s->autoincrement) {
               int4 rowid = unpack4(buf + 4);
               for (column_binding* cb = s->columns; cb != NULL; cb = cb->next) {
                   if (cb->var_type == cli_autoincrement) {
                       *(int4*)cb->var_ptr = rowid;
                   }
               }
           }
        }
    }
    return rc;
}

int cli_update(int statement)
{
    int rc = cli_send_columns(statement, cli_cmd_update);
    if (rc == cli_ok) { 
        int4 response;
        statement_desc* s = statements.get(statement);
        s->updated = true;
        if (!s->session->sock->read(&response, sizeof response)) { 
            rc = cli_network_error;
        } else { 
            unpack4(response);
            rc = response;
        }
    }
    return rc;      
}

static int cli_get(int statement, int cmd, cli_oid_t value = 0)
{
    statement_desc* s = statements.get(statement);
    if (s == NULL) { 
        return cli_bad_descriptor;
    }
    if (!s->prepared) { 
        return cli_not_fetched;
    }
    struct get_req { 
        cli_request req;
        cli_oid_t   value;
    } get;
    int length = sizeof(cli_request);
    if (cmd == cli_cmd_skip) { 
        length += 4;
        pack4((char*)(&get.req+1), (int)value);
    } else if (cmd == cli_cmd_seek) { 
        length += sizeof(cli_oid_t);
        pack_oid((char*)(&get.req+1), value);
    }
    get.req.length  = length;
    get.req.cmd     = cmd;
    get.req.stmt_id = statement;
    get.req.pack();
    if (!s->session->sock->write(&get.req, length)) { 
        return cli_network_error;
    }   
    int4 response;
    if (!s->session->sock->read(&response, sizeof response)) { 
        return cli_network_error;
    }
    unpack4(response);
    if (response <= 0) { 
        return response;
    }
    if (s->buf_size < (size_t)response-4) { 
        delete[] s->buf;
        s->buf_size = response-4 < DEFAULT_BUF_SIZE ? DEFAULT_BUF_SIZE : response-4;
        s->buf = new char[s->buf_size];
    }
    char* buf = s->buf;
    if (!s->session->sock->read(buf, response-4)) { 
        return cli_network_error;
    }
    char* p = buf;
    int result = cli_ok;
    if (cmd == cli_cmd_seek) { 
        s->oid = value;
    } else { 
        s->oid = unpack_oid(p);
        if (s->oid == 0) { 
            return cli_not_found;
        }
    }
    p += sizeof(cli_oid_t);
    for (column_binding* cb = s->columns; cb != NULL; cb = cb->next) { 
        int type = *p++;
        if (cb->var_type == cli_any) { 
            cb->var_type = type;
        } else { 
            assert(cb->var_type == type);
        }
        if (cb->set_fnc != NULL) { 
            int len = unpack4(p);
            p += 4;
            char* dst = (char*)cb->set_fnc(cb->var_type, cb->var_ptr, len, 
                                           cb->name, statement, p, cb->user_data);
            if (dst == NULL) {
                continue;
            }
            if (cb->var_type == cli_array_of_string) { 
                char** s = (char**)dst;
                while (--len >= 0) {
                    *s++ = p;
                    p += strlen(p) + 1;
                }
            } else if (cb->var_type == cli_array_of_wstring) { 
                wchar_t** s = (wchar_t**)dst;
                while (--len >= 0) {
                    *s++ = (wchar_t*)p;
                    p += (wcslen((wchar_t*)p) + 1)*sizeof(wchar_t);
                }
            } else if (cb->var_type >= cli_array_of_oid && cb->var_type < cli_array_of_string) { 
                switch (sizeof_type[cb->var_type-cli_array_of_oid]) { 
                  case 2:                   
                    while (--len >= 0) { 
                        p = unpack2(dst, p);
                        dst += 2;
                    }
                    break;
                  case 4:
                    while (--len >= 0) { 
                        p = unpack4(dst, p);
                        dst += 4;
                    }
                    break;
                  case 8:
                    while (--len >= 0) { 
                        p = unpack8(dst, p);
                        dst += 8;
                    }
                    break;
                  default:
                    memcpy(dst, p, len);
                    p += len;
                }
            } else { 
                memcpy(dst, p, len);
                p += len;
            }
        } else { 
            if (cb->var_type >= cli_asciiz && (cb->var_type <= cli_array_of_string || cb->var_type == cli_array_of_wstring)) { 
                int len = unpack4(p);
                p += 4;
                char* dst = (char*)cb->var_ptr;
                char* src = p;
                int n = len;
                if (cb->var_len != NULL) { 
                    if (n > *cb->var_len) { 
                        n = *cb->var_len;
                    }
                    *cb->var_len = n;
                }
                if (cb->var_type == cli_wstring || cb->var_type == cli_pwstring) { 
                    if (cb->var_type == cli_pwstring) { 
                        dst = *(char**)dst;
                    }
                    memcpy(dst, p, n*sizeof(wchar_t));
                    p += len*sizeof(wchar_t);
                } else if (cb->var_type >= cli_array_of_oid) { 
                    if (cb->var_type == cli_array_of_string) { 
                        char** s = (char**)dst;
                        len -= n;
                        while (--n >= 0) {
                            *s++ = p;
                            p += strlen(p) + 1;
                        }
                        while (--len >= 0) { 
                            p += strlen(p) + 1;
                        }
                    } else if (cb->var_type == cli_array_of_wstring) { 
                        wchar_t** s = (wchar_t**)dst;
                        len -= n;
                        while (--n >= 0) {
                            *s++ = (wchar_t*)p;
                            p += (wcslen((wchar_t*)p) + 1)*sizeof(wchar_t);
                        }
                        while (--len >= 0) { 
                            p += (wcslen((wchar_t*)p) + 1)*sizeof(wchar_t);
                        }
                    } else { 
                        switch (sizeof_type[cb->var_type-cli_array_of_oid]) { 
                          case 2:                   
                            while (--n >= 0) { 
                                src = unpack2(dst, src);
                                dst += 2;
                            }
                            p += len*2;
                            break;
                          case 4:
                            while (--n >= 0) { 
                                src = unpack4(dst, src);
                                dst += 4;
                            }
                            p += len*4;
                            break;
                          case 8:
                            while (--n >= 0) { 
                                src = unpack8(dst, src);
                                dst += 8;
                            }
                            p += len*8;
                            break;
                          default:
                            memcpy(dst, p, n);
                            p += len;
                        }
                    }
                } else { 
                    if (cb->var_type == cli_pasciiz) { 
                        dst = *(char**)dst;
                    }
                    memcpy(dst, p, n);
                    p += len;
                }
            } else if (cb->var_type == cli_rectangle) { 
                p = unpack_rectangle((cli_rectangle_t*)cb->var_ptr, p);
            } else { 
                switch (sizeof_type[cb->var_type]) { 
                  case 2:
                    p = unpack2((char*)cb->var_ptr, p);
                    break;
                  case 4:
                    p = unpack4((char*)cb->var_ptr, p);
                    break;
                  case 8:
                    p = unpack8((char*)cb->var_ptr, p);
                    break;
                  default:
                    *(char*)cb->var_ptr = *p++;
                }
            }
        }
    }
    s->updated = false;
    return result;
}

int cli_get_first(int statement)
{
    return cli_get(statement, cli_cmd_get_first);
}

int cli_get_last(int statement)
{
    return cli_get(statement, cli_cmd_get_last);
}

int cli_get_next(int statement)
{
    return cli_get(statement, cli_cmd_get_next);
}

int cli_get_prev(int statement)
{
    return cli_get(statement, cli_cmd_get_prev);
}

int cli_skip(int statement, int n)
{
    return cli_get(statement, cli_cmd_skip, n);
}

int cli_seek(int statement, cli_oid_t oid)
{
    return cli_get(statement, cli_cmd_seek, oid);
}


static int send_receive(int statement, int cmd)
{
    statement_desc* s = statements.get(statement);
    if (s == NULL) { 
        return cli_bad_descriptor;
    }
    if (!s->prepared) { 
        return cli_not_fetched;
    }
    cli_request req;
    req.length = sizeof(req);
    req.cmd = cmd;
    req.stmt_id = statement;
    req.pack();
    int4 response = cli_ok;
    if (!s->session->sock->write(&req, sizeof req)) { 
        return cli_network_error;
    } 
    if (!s->session->sock->read(&response, sizeof response)) { 
        return cli_network_error;
    }
    unpack4(response);
    return response;
}

int cli_freeze(int statement)
{
    return send_receive(statement, cli_cmd_freeze);
}

int cli_unfreeze(int statement)
{
    return send_receive(statement, cli_cmd_unfreeze);
}

cli_oid_t cli_get_oid(int statement)
{
    statement_desc* s = statements.get(statement);
    if (s == NULL) { 
        return 0;
    }
    return s->oid;
}


static int cli_send_command(int session, int statement, int cmd)
{
    session_desc* s = sessions.get(session);
    if (s == NULL) { 
        return cli_bad_descriptor;
    }   
    cli_request req;
    req.length  = sizeof(cli_request);
    req.cmd     = cmd;
    req.stmt_id = statement;
    req.pack();
    if (!s->sock->write(&req, sizeof req)) { 
        return cli_network_error;
    }
    int4 response;
    if (!s->sock->read(&response, sizeof response)) { 
        return cli_network_error;
    }
    unpack4(response);
    return response;
}

int cli_close_cursor(int statement)
{
    return cli_ok;
}

int cli_free(int statement)
{
    statement_desc* stmt = statements.get(statement);
    session_desc* s = stmt->session;
    if (s == NULL) { 
        return cli_bad_descriptor;
    }   
    statement_desc *sp, **spp = &s->stmts; 
    while ((sp = *spp) != stmt) { 
        if (sp == NULL) { 
            return cli_bad_descriptor;
        }
        spp = &sp->next;
    }
    *spp = stmt->next;
    stmt->deallocate();
    statements.deallocate(stmt);
    cli_request req;
    req.length  = sizeof(cli_request);
    req.cmd     = cli_cmd_free_statement;
    req.stmt_id = statement;
    req.pack();
    if (!s->sock->write(&req, sizeof req)) { 
        return cli_network_error;
    }
    return cli_ok;
}
    
int cli_lock(int session)
{
    session_desc* s = sessions.get(session);
    if (s == NULL) {
        return cli_bad_descriptor;
    }
    cli_request req;
    req.length = sizeof(req);
    req.cmd = cli_cmd_lock;
    req.stmt_id = 0;
    req.pack();
    return s->sock->write(&req, sizeof req) ? cli_ok : cli_network_error;
} 


int cli_commit(int session)
{
    return cli_send_command(session, 0, cli_cmd_commit);
}

int cli_precommit(int session)
{
    return cli_send_command(session, 0, cli_cmd_precommit);
}

int cli_abort(int session)
{
    return cli_send_command(session, 0, cli_cmd_abort);
}

int cli_remove(int statement)
{
    statement_desc* s = statements.get(statement);
    if (s == NULL) { 
        return cli_bad_descriptor;
    }
    if (s->oid == 0) { 
        return cli_not_found;
    }
    if (!s->for_update) { 
        return cli_not_update_mode;
    }
    return cli_send_command(s->session->id, s->id, cli_cmd_remove);
}

int cli_remove_current(int statement)
{
    statement_desc* s = statements.get(statement);
    if (s == NULL) { 
        return cli_bad_descriptor;
    }
    if (s->oid == 0) { 
        return cli_not_found;
    }
    if (!s->for_update) { 
        return cli_not_update_mode;
    }
    return cli_send_command(s->session->id, s->id, cli_cmd_remove_current);
}


int cli_describe(int session, char const* table, cli_field_descriptor** fields)
{
    int len = (int)(sizeof(cli_request) + strlen(table) + 1);
    dbSmallBuffer buf(len);
    cli_request* req = (cli_request*)buf.base();
    req->length = len;
    req->cmd    = cli_cmd_describe_table;
    req->stmt_id = 0;
    strcpy((char*)(req+1), table);
    session_desc* s = sessions.get(session);
    if (s == NULL) {
        return cli_bad_descriptor;
    }
    req->pack();
    if (!s->sock->write(buf, len)) {
        return cli_network_error;
    }
    int4 response[2];
    if (!s->sock->read(&response, sizeof response)) {
        return cli_network_error;
    }
    unpack4(response[0]);
    unpack4(response[1]);
    len = response[0];
    int nFields = response[1];
    if (nFields == -1) { 
        return cli_table_not_found;
    }
    char* p = (char*)malloc(nFields*sizeof(cli_field_descriptor) + len);
    cli_field_descriptor* fp = (cli_field_descriptor*)p;
    p += nFields*sizeof(cli_field_descriptor);
    if (!s->sock->read(p, len)) {
        return cli_network_error;
    }
    *fields = fp;
    for (int i = nFields; --i >= 0; fp++) {
        fp->type = (enum cli_var_type)*p++;
        fp->flags = *p++ & 0xFF;
        fp->name = p;
        p += strlen(p) + 1;
        if (*p != 0) { 
            fp->refTableName = p;
        } else { 
            fp->refTableName = NULL;
        }
        p += strlen(p) + 1;
        if (*p != 0) { 
            fp->inverseRefFieldName = p;
        } else { 
            fp->inverseRefFieldName = NULL;
        }
        p += strlen(p) + 1;
    }
    return nFields;
}

int cli_describe_layout(int session, char const* table, cli_field_layout** fields, int* rec_size)
{
    return cli_not_implemented;
}

int cli_show_tables(int session, cli_table_descriptor** tables)
{
    session_desc* s = sessions.get(session);
    if (s == NULL) {
        return cli_bad_descriptor;
    }
    cli_request req;
    req.length  = sizeof(cli_request);
    req.cmd     = cli_cmd_show_tables;
    req.stmt_id = 0;
    req.pack();
    if (!s->sock->write(&req, sizeof req)) {
        return cli_network_error;
    }
    int4 response[2];
    if (!s->sock->read(&response, sizeof response)) {
        return cli_network_error;
    }
    unpack4(response[0]);
    unpack4(response[1]);
    int len = response[0];
    int nTables = response[1];
    if (nTables == -1) {
        return cli_table_not_found;
    }
    char* p = (char*)malloc(nTables*sizeof(cli_table_descriptor) + len);
    cli_table_descriptor* fp = (cli_table_descriptor*)p;
    p += nTables*sizeof(cli_table_descriptor);
    if (!s->sock->read(p, len)) {
        free(p);
        return cli_network_error;
    }
    *tables = fp;
    for (int i = nTables; --i >= 0; fp++) {
        fp->name = (char*)p;
        p += strlen((char*)p)+1;
    }
    return nTables;
}


int cli_create(char_t const* databaseName,
               char_t const* filePath, 
               unsigned    transactionCommitDelay, 
               int         openAttr, 
               size_t      initDatabaseSize,
               size_t      extensionQuantum,
               size_t      initIndexSize,
               size_t      fileSizeLimit)
{
    return cli_bad_address;
}


static int cli_update_table(int cmd, int session, char const* tableName, int nColumns, 
                            cli_field_descriptor* columns)
{
    int i;
    session_desc* s = sessions.get(session);
    if (s == NULL) {
        return cli_bad_descriptor;
    }
    size_t size = sizeof(cli_request) + 4 + strlen(tableName)+1;
    for (i = 0; i < nColumns; i++) { 
        size += 2 + (strlen(columns[i].name)+3);
        if (columns[i].refTableName != NULL) { 
            size += strlen(columns[i].refTableName);
        }
        if (columns[i].inverseRefFieldName != NULL) { 
            size += strlen(columns[i].inverseRefFieldName);
        }
    }
    dbSmallBuffer buf(size);
    cli_request* req = (cli_request*)buf.base();
    req->length  = (int)size;
    req->cmd     = cmd;
    req->stmt_id = 0;
    char* dst = (char*)(req + 1);
    strcpy(dst, tableName);
    dst += strlen(dst) + 1;
    *dst++ = (char)nColumns;
    for (i = 0; i < nColumns; i++) { 
        *dst++ = (char)columns[i].type;
        *dst++ = (char)columns[i].flags;
        strcpy(dst, columns[i].name);
        dst += strlen(dst) + 1;
        if (columns[i].refTableName != NULL) { 
            strcpy(dst, columns[i].refTableName);
            dst += strlen(dst) + 1;        
        } else { 
            *dst++ = 0;
        }
        if (columns[i].inverseRefFieldName != NULL) { 
            strcpy(dst, columns[i].inverseRefFieldName);
            dst += strlen(dst) + 1;
        } else { 
            *dst++ = 0;
        }
    }
    req->pack();
    if (!s->sock->write(buf, size)) {
        return cli_network_error;
    }
    int4 response;
    if (!s->sock->read(&response, sizeof response)) {
        return cli_network_error;
    }
    unpack4(response);
    return response;
}

int cli_create_table(int session, char const* tableName, int nColumns, 
                     cli_field_descriptor* columns)
{
    return cli_update_table(cli_cmd_create_table, session, tableName, nColumns, columns);
}

int cli_alter_table(int session, char const* tableName, int nColumns, 
                     cli_field_descriptor* columns)
{
    return cli_update_table(cli_cmd_alter_table, session, tableName, nColumns, columns);
}

int cli_drop_table(int session, char const* tableName)
{
    session_desc* s = sessions.get(session);
    if (s == NULL) {
        return cli_bad_descriptor;
    }
    int size = (int)(sizeof(cli_request) + strlen(tableName) + 1);
    dbSmallBuffer buf(size);
    cli_request* req = (cli_request*)buf.base();
    req->length  = size;
    req->cmd     = cli_cmd_drop_table;
    req->stmt_id = 0;
    strcpy((char*)(req + 1), tableName);
    req->pack();
    if (!s->sock->write(buf, size)) {
        return cli_network_error;
    }
    int4 response;
    if (!s->sock->read(&response, sizeof response)) {
        return cli_network_error;
    }
    unpack4(response);
    return response;
}

int cli_alter_index(int session, char const* tableName, char const* fieldName, int newFlags)
{
    session_desc* s = sessions.get(session);
    if (s == NULL) {
        return cli_bad_descriptor;
    }
    int size = (int)(sizeof(cli_request) + 1 + strlen(tableName)+strlen(fieldName)+2);
    dbSmallBuffer buf(size);
    cli_request* req = (cli_request*)buf.base();
    req->length  = size;
    req->cmd     = cli_cmd_alter_index;
    req->stmt_id = 0;
    char* dst = (char*)(req + 1);
    strcpy(dst, tableName);
    dst += strlen(dst) + 1;
    strcpy(dst, fieldName);
    dst += strlen(dst) + 1;
    *dst++ = newFlags;
    req->pack();
    if (!s->sock->write(buf, size)) {
        return cli_network_error;
    }
    int4 response;
    if (!s->sock->read(&response, sizeof response)) {
        return cli_network_error;
    }
    unpack4(response);
    return response;
}


cli_error_handler cli_set_error_handler(int session, cli_error_handler new_handler, void* context)
{
    return NULL;
}

int cli_create_replication_node(int         nodeId,
                                int         nServers,
                                char*       nodeNames[],
                                char const* databaseName, 
                                char const* filePath, 
                                int         openAttr, 
                                size_t      initDatabaseSize,
                                size_t      extensionQuantum,
                                size_t      initIndexSize,
                                size_t      fileSizeLimit)
{
    return cli_not_implemented;
}

int cli_attach(int session)
{
    return cli_not_implemented;
}

int cli_detach(int session, int detach_mode)
{
    return cli_not_implemented;
}

void cli_free_memory(int, void* ptr)
{
    free(ptr);
}

int cli_get_database_state(int session, cli_database_monitor* monitor)
{
    return cli_not_implemented;
}

void cli_set_trace_function(cli_trace_function_t) 
{ 
}

int cli_prepare_query(int session, char const* query)
{
    return cli_not_implemented;
}

int cli_execute_query(int statement, int for_update, void* record_struct, ...)
{
    return cli_not_implemented;
}

int cli_execute_query_ex(int statement, int for_update, void* record_struct, int n_params, int* param_types, void** param_value)
{
    return cli_not_implemented;
}

int cli_insert_struct(int session, char const* table_name, void* record_struct, cli_oid_t* oid)
{
    return cli_not_implemented;
}

int cli_get_field_size(cli_field_descriptor* fields, int field_no)
{
    return sizeof_type[fields[field_no].type];
}


int cli_get_field_offset(cli_field_descriptor* fields, int field_no)
{
    int offs = 0;
    int size = 0;
    for (int i = 0; i <= field_no; i++) { 
        size = sizeof_type[fields[i].type];
        offs = DOALIGN(offs, alignof_type[fields[i].type]);
        offs += size;
    }
    return offs - size;
}

int cli_xml_export(int session, FILE* out)
{
    return cli_not_implemented;
}

int cli_xml_import(int session, FILE* in)
{
    return cli_not_implemented;
}

int cli_backup(int session, char const* file_name, int compactify)
{
    return cli_not_implemented;
}

int cli_schedule_backup(int session, char const* file_name, int period)
{
    return cli_not_implemented;
}

int cli_get_wrapping_rectangle(int session, char const* table, char const* field, cli_rectangle_t* rect)
{
    return cli_not_implemented;
}
