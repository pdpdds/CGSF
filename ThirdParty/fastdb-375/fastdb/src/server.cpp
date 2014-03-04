//-< SERVER.CPP >----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     13-Jan-2000 K.A. Knizhnik   * / [] \ *
//                          Last update: 13-Jan-2000 K.A. Knizhnik   * GARRET *
//-------------------------------------------------------------------*--------*
// CLI multithreaded server implementation
//-------------------------------------------------------------------*--------*

#define INSIDE_FASTDB

#include "fastdb.h"
#include "compiler.h"
#include "wwwapi.h"
#include "subsql.h"
#include "symtab.h"
#include "hashtab.h"
#include "ttree.h"
#include "rtree.h"
#include "cli.h"
#include "cliproto.h"
#include "server.h"
#include "localcli.h"
#include <ctype.h>

BEGIN_FASTDB_NAMESPACE

#if !THREADS_SUPPORTED
#error Server requires multithreading support
#endif

int dbColumnBinding::unpackArray(char* dst, size_t& offs)
{
    int len = this->len;
    int i;
    switch (cliType) { 
      case cli_array_of_string:
        { 
            dbVarying* hdr = (dbVarying*)(dst + offs);
            char* body = (char*)(hdr + len);
            char* p = ptr + 4;
            int relOffs = sizeof(dbVarying)*len;
            offs += relOffs;
            for (i = 0; i < len; i++) { 
                strcpy(body, p);
                int slen = (int)strlen(p)+1;
                hdr->size = slen;
                hdr->offs = relOffs;
                body += slen;
                relOffs += slen - sizeof(dbVarying);
                p += slen;
                hdr += 1;
            }
            offs += relOffs;
        }
        break;
      case cli_array_of_wstring:
        { 
            dbVarying* hdr = (dbVarying*)(dst + offs);
            char* body = (char*)(hdr + len);
            char* p = ptr + 4;
            int relOffs = sizeof(dbVarying)*len;
            offs += relOffs;
            for (i = 0; i < len; i++) { 
                wcscpy((wchar_t*)body, (wchar_t*)p);
                int slen = (int)wcslen((wchar_t*)p)+1;
                hdr->size = slen;
                hdr->offs = relOffs;
                body += slen*sizeof(wchar_t);
                relOffs += slen*sizeof(wchar_t) - sizeof(dbVarying);
                p += slen*sizeof(wchar_t);
                hdr += 1;
            }
            offs += relOffs;
        }
        break;
      case cli_array_of_decimal:
        {
            char* p = ptr + 4;
            for (i = 0; i < len; i++) { 
                double val = 0.0;
                sscanf(p, "%lf", &val);
                p += strlen(p) + 1;
                switch (fd->components->type) {
                  case dbField::tpInt1:
                    *(dst + offs) = (int1)val;
                    offs += sizeof(int1);
                    break;
                  case dbField::tpInt2:
                    *(int2*)(dst + offs) = (int2)val;
                    offs += sizeof(int2);
                    break;
                  case dbField::tpInt4:
                    *(int4*)(dst + offs) = (int4)val;
                    offs += sizeof(int4);
                    break;
                  case dbField::tpInt8:
                    *(db_int8*)(dst + offs) = (db_int8)val;
                    offs += sizeof(db_int8);
                    break;
                  case dbField::tpReal4:
                    *(real4*)(dst + offs) = (real4)val;
                    offs += sizeof(real4);
                    break;
                  case dbField::tpReal8:
                    *(real8*)(dst + offs) = val;
                    offs += sizeof(real8);
                    break;
                }
            }
        }
        break;
      case cli_cstring:
        memcpy((char*)(dst + offs), ptr + 4, len-1);
        offs += len;
        *((char*)(dst + offs - 1)) = '\0';
        break;
      case cli_asciiz:
      case cli_pasciiz:
        memcpy((char*)(dst + offs), ptr + 4, len);
        offs += len;
        break;
      case cli_wstring:
      case cli_pwstring:
        memcpy((char*)(dst + offs), ptr + 4, len*sizeof(wchar_t));
        offs += len*sizeof(wchar_t);
        break;
      default:
        if (cliType >= cli_array_of_oid) { 
            switch (sizeof_type[cliType - cli_array_of_oid]) { 
              case 1:
                memcpy(dst + offs, ptr + 4, len);
                break;
              case 2:
                for (i = 0; i < len; i++) {
                    unpack2(dst + offs + i*2, ptr + 4 + i*2);
                }
                break;
              case 4:
                for (i = 0; i < len; i++) {
                    unpack4(dst + offs + i*4, ptr + 4 + i*4);
                }
                break;
              case 8:
                for (i = 0; i < len; i++) {
                    unpack8(dst + offs + i*8, ptr + 4 + i*8);
                }
                break;
              default:
                assert(false);
            }
        }
        offs += len*sizeof_type[cliType - cli_array_of_oid];
    }
    return len;
}

void dbColumnBinding::unpackScalar(char* dst, bool insert)
{
    if (cliType == cli_autoincrement) { 
        assert(fd->type == dbField::tpInt4);
        if (insert) { 
#ifdef AUTOINCREMENT_SUPPORT
            *(int4*)(dst+fd->dbsOffs) = fd->defTable->autoincrementCount;
#else
            *(int4*)(dst+fd->dbsOffs) = ((dbTable*)fd->defTable->db->getRow(fd->defTable->tableId))->nRows;
#endif
        }
        return;
    } 
    switch (fd->type) { 
      case dbField::tpBool:
      case dbField::tpInt1:
        switch (sizeof_type[cliType]) { 
          case 1:
            *(dst + fd->dbsOffs) = *ptr;
            break;
          case 2:
            *(dst + fd->dbsOffs) = (char)unpack2(ptr);
            break;
          case 4:
            *(dst + fd->dbsOffs) = (char)unpack4(ptr);
            break;
          case 8:
            *(dst + fd->dbsOffs) = (char)unpack8(ptr);
            break;
          default:
            assert(false);
        }
        break;          
      case dbField::tpInt2:
        switch (sizeof_type[cliType]) { 
          case 1:
            *(int2*)(dst+fd->dbsOffs) = *ptr;
            break;
          case 2:
            unpack2(dst+fd->dbsOffs, ptr);
            break;
          case 4:
            *(int2*)(dst+fd->dbsOffs) = (int2)unpack4(ptr);
            break;
          case 8:
            *(int2*)(dst+fd->dbsOffs) = (int2)unpack8(ptr);
            break;
          default:
            assert(false);
        }
        break;          
      case dbField::tpInt4:
        switch (sizeof_type[cliType]) { 
          case 1:
            *(int4*)(dst+fd->dbsOffs) = *ptr;
            break;
          case 2:
            *(int4*)(dst+fd->dbsOffs) = unpack2(ptr);
            break;
          case 4:
            unpack4(dst+fd->dbsOffs, ptr);
            break;
          case 8:
            *(int4*)(dst+fd->dbsOffs) = (int4)unpack8(ptr);
            break;
          default:
            assert(false);
        }
        break;          
      case dbField::tpInt8:
        switch (sizeof_type[cliType]) { 
          case 1:
            *(db_int8*)(dst+fd->dbsOffs) = *ptr;
            break;
          case 2:
            *(db_int8*)(dst+fd->dbsOffs) = unpack2(ptr);
            break;
          case 4:
            *(db_int8*)(dst+fd->dbsOffs) = unpack4(ptr);
            break;
          case 8:
            unpack8(dst+fd->dbsOffs, ptr);
            break;
          default:
            assert(false);
        }
        break;          
      case dbField::tpReal4:
        switch (cliType) { 
          case cli_real4:
            unpack4(dst+fd->dbsOffs, ptr);
            break;
          case cli_real8:
            {
                real8 temp;
                unpack8((char*)&temp, ptr);
                *(real4*)(dst + fd->dbsOffs) = (real4)temp;
            }
            break;
          default:
            assert(false);
        }
        break;
      case dbField::tpReal8:
        switch (cliType) { 
          case cli_real4:
            {
                real4 temp;
                unpack4((char*)&temp, ptr);
                *(real8*)(dst + fd->dbsOffs) = temp;
            }
            break;
          case cli_real8:
            unpack8(dst+fd->dbsOffs, ptr);
            break;
          default:
            assert(false);
        }
        break;
     case dbField::tpReference:
        *(oid_t*)(dst + fd->dbsOffs) = unpack_oid(ptr);
         break;
      case dbField::tpRectangle:
        unpack_rectangle((cli_rectangle_t*)(dst + fd->dbsOffs), ptr);
        break;
      default:
        assert(false);
    }
}

void dbStatement::reset()
{
    dbColumnBinding *cb, *next;
    for (cb = columns; cb != NULL; cb = next) { 
        next = cb->next;
        delete cb;
    }
    columns = NULL;
    delete[] params;
    params = NULL;
    delete cursor;
    cursor = NULL;
    query.reset();
    table = NULL;
}

int dbQueryScanner::get()
{
    int i = 0, ch, digits;
    
    do { 
        if ((ch = *p++) == '\0') { 
            return tkn_eof;
        }
    } while (isspace(ch));
    
    if (ch == '*') { 
        return tkn_all;
    } else if (isdigit(ch) || ch == '+' || ch == '-') { 
        do { 
            buf[i++] = ch;
            if (i == dbQueryMaxIdLength) { 
                // Numeric constant too long
                return tkn_error;
            }
            ch = *p++;
        } while (ch != '\0' 
                 && (isdigit(ch) || ch == '+' || ch == '-' || ch == 'e' || 
                     ch == 'E' || ch == '.'));
        p -= 1;
        buf[i] = '\0';
        if (sscanf(buf, INT8_FORMAT "%n", &ival, &digits) != 1) { 
            // Bad integer constant
            return tkn_error;
        }
        if (digits != i) { 
            if (sscanf(buf, "%lf%n", &fval, &digits) != 1 || digits != i) {
                // Bad float constant
                return tkn_error;
            }
            return tkn_fconst;
        } 
        return tkn_iconst;
    } else if (isalpha(ch) || ch == '$' || ch == '_') { 
        do { 
            buf[i++] = ch;
            if (i == dbQueryMaxIdLength) { 
                // Identifier too long
                return tkn_error;
            }
            ch = *p++;
        } while (ch != EOF && (isalnum(ch) || ch == '$' || ch == '_'));
        p -= 1;
        buf[i] = '\0';
        ident = buf;
        return dbSymbolTable::add(ident, tkn_ident);
    } else { 
        // Invalid symbol
        return tkn_error;
    }
}

dbServer* dbServer::chain;

inline dbStatement* dbServer::findStatement(dbSession* session, int stmt_id)
{
    for (dbStatement* stmt = session->stmts; stmt != NULL; stmt = stmt->next)
    {
        if (stmt->id == stmt_id) { 
            return stmt;
        }
    }
    return NULL;
}

void thread_proc dbServer::serverThread(void* arg)
{
    ((dbServer*)arg)->serveClient();
}

void thread_proc dbServer::acceptLocalThread(void* arg)
{
    dbServer* server = (dbServer*)arg;
    server->acceptConnection(server->localAcceptSock);
}

void thread_proc dbServer::acceptGlobalThread(void* arg)
{
    dbServer* server = (dbServer*)arg;
    server->acceptConnection(server->globalAcceptSock);
}

dbServer::dbServer(dbDatabase* db,
                   char const* serverURL, 
                   int optimalNumberOfThreads, 
                   int connectionQueueLen)
{
    char buf[256];
    next = chain;
    chain = this;
    this->db = db;
    this->optimalNumberOfThreads = optimalNumberOfThreads;
    this->URL = new char[strlen(serverURL)+1];
    strcpy(URL, serverURL);
    globalAcceptSock = 
        socket_t::create_global(serverURL, connectionQueueLen);
    if (!globalAcceptSock->is_ok()) { 
        globalAcceptSock->get_error_text(buf, sizeof buf);
        dbTrace("Failed to create global socket: %s\n", buf);
        delete globalAcceptSock;
        globalAcceptSock = NULL;
    }
    localAcceptSock = 
        socket_t::create_local(serverURL, connectionQueueLen);
    if (!localAcceptSock->is_ok()) { 
        localAcceptSock->get_error_text(buf, sizeof buf);
        dbTrace("Failed to create local socket: %s\n", buf);
        delete localAcceptSock;
        localAcceptSock = NULL;
    }
    freeList = activeList = waitList = NULL;
    waitListLength = 0;
}
   
dbServer* dbServer::find(char const* URL)
{
    for (dbServer* server = chain; server != NULL; server = server->next) { 
        if (strcmp(URL, server->URL) == 0) { 
            return server;
        }
    }
    return NULL;
}

void dbServer::cleanup()
{
    dbServer *server, *next;
    for (server = chain; server != NULL; server = next) { 
        next = server->next;
        delete server;
    } 
}

void dbServer::start()
{
    nActiveThreads = nIdleThreads = 0;    
    cancelWait = cancelSession = cancelAccept = false;
    go.open();
    done.open();
    if (globalAcceptSock != NULL) { 
        globalAcceptThread.create(acceptGlobalThread, this);
    }
    if (localAcceptSock != NULL) { 
        localAcceptThread.create(acceptLocalThread, this);
    }
}

void dbServer::stop()
{
    cancelAccept = true;
    if (globalAcceptSock != NULL) {
        globalAcceptSock->cancel_accept();
        globalAcceptThread.join();
    }
    delete globalAcceptSock;
    globalAcceptSock = NULL;

    if (localAcceptSock != NULL) { 
        localAcceptSock->cancel_accept();
        localAcceptThread.join();
    }
    delete localAcceptSock;
    localAcceptSock = NULL;

    dbCriticalSection cs(mutex);
    cancelSession = true;
    while (activeList != NULL) { 
        activeList->sock->shutdown();
        done.wait(mutex);
    }

    cancelWait = true;
    while (nIdleThreads != 0) { 
        go.signal();
        done.wait(mutex);
    }

    while (waitList != NULL) {
        dbSession* next = waitList->next;
        delete waitList->sock;
        waitList->next = freeList;
        freeList = waitList;
        waitList = next;
    }
    waitListLength = 0;
    assert(nActiveThreads == 0);
    done.close();
    go.close();
}

bool dbServer::freeze(dbSession* session, int stmt_id)
{
    dbStatement* stmt = findStatement(session, stmt_id);
    int4 response = cli_ok;
    if (stmt == NULL || stmt->cursor == NULL) { 
        response = cli_bad_descriptor;
    } else { 
        stmt->cursor->freeze();
    }
    pack4(response);
    return session->sock->write(&response, sizeof response);
}
    
bool dbServer::unfreeze(dbSession* session, int stmt_id)
{
    dbStatement* stmt = findStatement(session, stmt_id);
    int4 response = cli_ok;
    if (stmt == NULL || stmt->cursor == NULL) { 
        response = cli_bad_descriptor;
    } else { 
        stmt->cursor->unfreeze();
    }
    pack4(response);
    return session->sock->write(&response, sizeof response);
}
    
bool dbServer::get_first(dbSession* session, int stmt_id)
{
    dbStatement* stmt = findStatement(session, stmt_id);
    int4 response;
    if (stmt == NULL || stmt->cursor == NULL) { 
        response = cli_bad_descriptor;
    } else if (!stmt->cursor->gotoFirst()) { 
        response = cli_not_found;
    } else { 
        return fetch(session, stmt);
    }
    pack4(response);
    return session->sock->write(&response, sizeof response);
}

bool dbServer::get_last(dbSession* session, int stmt_id)
{
    dbStatement* stmt = findStatement(session, stmt_id);
    int4 response;
    if (stmt == NULL || stmt->cursor == NULL) { 
        response = cli_bad_descriptor;
    } else if (!stmt->cursor->gotoLast()) { 
        response = cli_not_found;
    } else { 
        return fetch(session, stmt);
    }
    pack4(response);
    return session->sock->write(&response, sizeof response);
}

bool dbServer::get_next(dbSession* session, int stmt_id)
{
    dbStatement* stmt = findStatement(session, stmt_id);
    int4 response;
    if (stmt == NULL || stmt->cursor == NULL) { 
        response = cli_bad_descriptor;
    } 
    else if (!((stmt->firstFetch && stmt->cursor->gotoFirst()) ||
               (!stmt->firstFetch && stmt->cursor->moveNext()))) 
    { 
        response = cli_not_found;
    } else { 
        return fetch(session, stmt);
    }
    pack4(response);
    return session->sock->write(&response, sizeof response);
}

bool dbServer::get_prev(dbSession* session, int stmt_id)
{
    dbStatement* stmt = findStatement(session, stmt_id);
    int4 response;
    if (stmt == NULL || stmt->cursor == NULL) { 
        response = cli_bad_descriptor;
    } 
    else if (!((stmt->firstFetch && stmt->cursor->gotoLast()) ||
               (!stmt->firstFetch && stmt->cursor->movePrev()))) 
    { 
        response = cli_not_found;
    } else { 
        return fetch(session, stmt);
    }
    pack4(response);
    return session->sock->write(&response, sizeof response);
}


bool dbServer::skip(dbSession* session, int stmt_id, char* buf)
{
    dbStatement* stmt = findStatement(session, stmt_id);
    int4 response;
    if (stmt == NULL || stmt->cursor == NULL) {
        response = cli_bad_descriptor;
    } else { 
        int n = unpack4(buf);
        if ((n > 0 && !(((stmt->firstFetch && stmt->cursor->gotoFirst() && stmt->cursor->skip(n-1))
                         || (!stmt->firstFetch && stmt->cursor->skip(n)))))
            || (n < 0 && !(((stmt->firstFetch && stmt->cursor->gotoLast() && stmt->cursor->skip(n+1))
                            || (!stmt->firstFetch && stmt->cursor->skip(n))))))
        {
            response = cli_not_found;
        } else {
            return fetch(session, stmt);
        }
    }
    pack4(response);
    return session->sock->write(&response, sizeof response);
}

bool dbServer::seek(dbSession* session, int stmt_id, char* buf)
{
    dbStatement* stmt = findStatement(session, stmt_id);
    int4 response;
    if (stmt == NULL || stmt->cursor == NULL) {
        response = cli_bad_descriptor;
    } else { 
        oid_t oid = unpack_oid(buf);
        int pos = stmt->cursor->seek(oid); 
        if (pos < 0) { 
            response = cli_not_found;
        } else { 
            return fetch(session, stmt, pos);
        }
    }
    pack4(response);
    return session->sock->write(&response, sizeof response);
}

bool dbServer::fetch(dbSession* session, dbStatement* stmt, oid_t result)
{
    int4 response;
    char buf[64];
    dbColumnBinding* cb;
    
    stmt->firstFetch = false;
    if (stmt->cursor->isEmpty()) { 
        response = cli_not_found;
        pack4(response);
        return session->sock->write(&response, sizeof response);
    }
    size_t msg_size = sizeof(cli_oid_t) + 4;
    char* data = (char*)db->getRow(stmt->cursor->currId);
    char* src;
    for (cb = stmt->columns; cb != NULL; cb = cb->next) { 
        src = data + cb->fd->dbsOffs;
        if (cb->cliType == cli_array_of_string) {
            int len = ((dbVarying*)src)->size;
            dbVarying* p = (dbVarying*)(data + ((dbVarying*)src)->offs);
            msg_size += 4;
            while (--len >= 0) { 
                msg_size += p->size;
                p += 1;
            }
        } else if (cb->cliType == cli_array_of_wstring) {
            int len = ((dbVarying*)src)->size;
            dbVarying* p = (dbVarying*)(data + ((dbVarying*)src)->offs);
            msg_size += 4;
            while (--len >= 0) { 
                msg_size += p->size*sizeof(wchar_t);
                p += 1;
            }
        } else if (cb->cliType == cli_array_of_decimal) { 
            int len = ((dbVarying*)src)->size;
            msg_size += 4;
            char* p = data + ((dbVarying*)src)->offs;
            while (--len >= 0) { 
                switch (cb->fd->components->type) {
                  case dbField::tpInt1:
                    sprintf(buf, "%d", *(int1*)p);
                    p += sizeof(int1);
                    break;
                  case dbField::tpInt2:
                    sprintf(buf, "%d", *(int2*)p);
                    p += sizeof(int2);
                    break;
                  case dbField::tpInt4:
                    sprintf(buf, "%d", *(int4*)p);
                    p += sizeof(int4);
                    break;
                  case dbField::tpInt8:
                    sprintf(buf, INT8_FORMAT, *(db_int8*)p);
                    p += sizeof(db_int8);
                    break;
                  case dbField::tpReal4:
                    sprintf(buf, "%.14g", *(real4*)p);
                    p += sizeof(real4);
                    break;
                  case dbField::tpReal8:
                    sprintf(buf, "%.14g", *(real8*)p);
                    p += sizeof(real8);
                    break;
                }
                msg_size += strlen(buf) + 1;
            }
        } else if (cb->cliType == cli_datetime || cb->cliType == cli_autoincrement) {
            msg_size += 4;
        } else if (cb->cliType >= cli_array_of_oid && cb->cliType < cli_array_of_string) {
            msg_size += 4 + ((dbVarying*)(data + cb->fd->dbsOffs))->size
                            * sizeof_type[cb->cliType - cli_array_of_oid];
        } else if (cb->cliType >= cli_asciiz && cb->cliType <= cli_cstring) { 
            int size = ((dbVarying*)src)->size;
            if (cb->cliType == cli_cstring && size != 0) { 
                size -= 1; // omit '\0'
            }
            msg_size += 4 + size;
        } else if (cb->cliType == cli_wstring || cb->cliType == cli_pwstring) { 
            msg_size += 4 + ((dbVarying*)src)->size*sizeof(wchar_t);
        } else if (cb->cliType == cli_decimal) {
            switch (cb->fd->type) {
              case dbField::tpInt1:
                sprintf(buf, "%d", *(int1*)src);
                break;
              case dbField::tpInt2:
                sprintf(buf, "%d", *(int2*)src);
                break;
              case dbField::tpInt4:
                sprintf(buf, "%d", *(int4*)src);
                break;
              case dbField::tpInt8:
                sprintf(buf, INT8_FORMAT, *(db_int8*)src);
                break;
              case dbField::tpReal4:
                sprintf(buf, "%.14g", *(real4*)src);
                break;
              case dbField::tpReal8:
                sprintf(buf, "%.14g", *(real8*)src);
                break;
            }
            msg_size += strlen(buf) + 1;
        } else { 
            msg_size += sizeof_type[cb->cliType];
        }
        msg_size += 1; // column type
    }
    if ((size_t)stmt->buf_size < msg_size) { 
        delete[] stmt->buf;
        stmt->buf = new char[msg_size];
        stmt->buf_size = (int)msg_size;
    }
    char* p = stmt->buf;
    p = pack4(p, (int)msg_size);
    p = pack_oid(p, result);

    for (cb = stmt->columns; cb != NULL; cb = cb->next) { 
        char* src = data + cb->fd->dbsOffs;
        *p++ = cb->cliType;
        if (cb->cliType == cli_decimal) {
            switch (cb->fd->type) {
              case dbField::tpInt1:
                p += sprintf(p, "%d", *(int1*)src);
                break;
              case dbField::tpInt2:
                p += sprintf(p, "%d", *(int2*)src);
                break;
              case dbField::tpInt4:
                p += sprintf(p, "%d", *(int4*)src);
                break;
              case dbField::tpInt8:
                p += sprintf(p, INT8_FORMAT, *(db_int8*)src);
                break;
              case dbField::tpReal4:
                p += sprintf(p, "%.14g", *(real4*)src);
                break;
              case dbField::tpReal8:
                p += sprintf(p, "%.14g", *(real8*)src);
                break;
            }
            p += 1;
        } else { 
            switch (cb->fd->type) { 
              case dbField::tpBool:
              case dbField::tpInt1:
                switch (sizeof_type[cb->cliType]) { 
                  case 1:
                    *p++ = *src;
                    break;
                  case 2:
                    p = pack2(p, (int2)*(char*)src);
                    break;
                  case 4:
                    p = pack4(p, (int4)*(char*)src);
                    break;
                  case 8:
                    p = pack8(p, (db_int8)*(char*)src);
                    break;
                  default:
                    assert(false);
                }
                break;              
              case dbField::tpInt2:
                switch (sizeof_type[cb->cliType]) { 
                  case 1:
                    *p++ = (char)*(int2*)src;
                    break;
                  case 2:
                    p = pack2(p, src);
                    break;
                  case 4:
                    p = pack4(p, (int4)*(int2*)src);
                    break;
                  case 8:
                    p = pack8(p, (db_int8)*(int2*)src);
                    break;
                  default:
                    assert(false);
                }
                break;
              case dbField::tpInt4:
                switch (sizeof_type[cb->cliType]) { 
                  case 1:
                    *p++ = (char)*(int4*)src;
                    break;
                  case 2:
                    p = pack2(p, (int2)*(int4*)src);
                    break;
                  case 4:
                    p = pack4(p, src);
                    break;
                  case 8:
                    p = pack8(p, (db_int8)*(int4*)src);
                    break;
                  default:
                    assert(false);
                }
                break;
              case dbField::tpInt8:
                switch (sizeof_type[cb->cliType]) { 
                  case 1:
                    *p++ = (char)*(db_int8*)src;
                    break;
                  case 2:
                    p = pack2(p, (int2)*(db_int8*)src);
                    break;
                  case 4:
                    p = pack4(p, (int4)*(db_int8*)src);
                    break;
                  case 8:
                    p = pack8(p, src);
                    break;
                  default:
                    assert(false);
                }
                break;
              case dbField::tpReal4:
                switch (cb->cliType) { 
                  case cli_real4:
                    p = pack4(p, src);
                    break;
                  case cli_real8:
                  {
                      real8 temp = *(real4*)src;
                      p = pack8(p, (char*)&temp);
                      break;
                  }
                  default:
                    assert(false);
                }
                break;
              case dbField::tpReal8:
                switch (cb->cliType) { 
                  case cli_real4:
                  {
                      real4 temp = (real4)*(real8*)src;
                      p = pack4(p, (char*)&temp);
                  }
                  break;
                  case cli_real8:
                    p = pack8(p, src);
                    break;
                  default:
                    assert(false);
                }
                break;
              case dbField::tpString:
              {
                  dbVarying* v = (dbVarying*)src;
                  int size = v->size;
                  if (cb->cliType == cli_cstring && size != 0) { 
                      size -= 1;
                  }
                  p = pack4(p, size);
                  memcpy(p, data + v->offs, size);
                  p += v->size;
                  break;
              }
              case dbField::tpWString:
              {
                  dbVarying* v = (dbVarying*)src;
                  int size = v->size;
                  p = pack4(p, size);
                  memcpy(p, data + v->offs, size*sizeof(wchar_t));
                  p += size*sizeof(wchar_t);
                  break;
              }
              case dbField::tpReference:
                p = pack_oid(p, *(oid_t*)src);
                break;
              case dbField::tpRectangle:
                p = pack_rectangle(p, (cli_rectangle_t*)src);
                break;
              case dbField::tpArray:
              {
                  dbVarying* v = (dbVarying*)src;
                  int n = v->size;
                  p = pack4(p, n);
                  src = data + v->offs;
                  if (cb->cliType == cli_array_of_string) {
                      while (--n >= 0) {
                          strcpy(p, (char*)(src + ((dbVarying*)src)->offs));
                          p += strlen(p) + 1;
                          src += sizeof(dbVarying);
                      }
                  } else if (cb->cliType == cli_array_of_wstring) {
                      while (--n >= 0) {
                          wcscpy((wchar_t*)p, (wchar_t*)(src + ((dbVarying*)src)->offs));
                          p += (wcslen((wchar_t*)p) + 1)*sizeof(wchar_t);
                          src += sizeof(dbVarying);
                      }
                  } else if (cb->cliType == cli_array_of_decimal) { 
                      while (--n >= 0) {
                          switch (cb->fd->components->type) {
                            case dbField::tpInt1:
                              p += sprintf(p, "%d", *(int1*)src) + 1;
                              src += sizeof(int1);
                              break;
                            case dbField::tpInt2:
                              p += sprintf(p, "%d", *(int2*)src) + 1;
                              src += sizeof(int2);
                              break;
                            case dbField::tpInt4:
                              p += sprintf(p, "%d", *(int4*)src) + 1;
                              src += sizeof(int4);
                              break;
                            case dbField::tpInt8:
                              p += sprintf(p, INT8_FORMAT, *(db_int8*)src) + 1;
                              src += sizeof(db_int8);
                              break;
                            case dbField::tpReal4:
                              p += sprintf(buf, "%.14g", *(real4*)src) + 1;
                              src += sizeof(real4);
                              break;
                            case dbField::tpReal8:
                              p += sprintf(buf, "%.14g", *(real8*)src) + 1;
                              src += sizeof(real8);
                              break;
                          }
                      }   
                  } else { 
                      switch (sizeof_type[cb->cliType-cli_array_of_oid]) { 
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
                  break;
              }
              case dbField::tpStructure:
                assert(cb->cliType == cli_datetime);
                p = pack4(p, src);
                break;
              default:
                assert(false);
            }
        }
    }
    assert((size_t)(p - stmt->buf) == msg_size);
    return session->sock->write(stmt->buf, msg_size);
}
    
bool dbServer::remove(dbSession* session, int stmt_id)
{
    dbStatement* stmt = findStatement(session, stmt_id);
    int4 response;
    if (stmt == NULL) { 
        response = cli_bad_descriptor;
    } else { 
        if (stmt->cursor->isEmpty()) { 
            response = cli_not_found;
        } else { 
            stmt->cursor->removeAllSelected();
            response = cli_ok;
        }
    }
    pack4(response);
    return session->sock->write(&response, sizeof response);
}

bool dbServer::remove_current(dbSession* session, int stmt_id)
{
    dbStatement* stmt = findStatement(session, stmt_id);
    int4 response;
    if (stmt == NULL) { 
        response = cli_bad_descriptor;
    } else { 
        if (stmt->cursor->isEmpty()) { 
            response = cli_not_found;
        } else { 
            stmt->cursor->remove();
            response = cli_ok;
        }
    }
    pack4(response);
    return session->sock->write(&response, sizeof response);
}

bool dbServer::update(dbSession* session, int stmt_id, char* new_data)
{
    dbStatement* stmt = findStatement(session, stmt_id);
    dbColumnBinding* cb;
    int4 response;
    if (stmt == NULL) { 
        response = cli_bad_descriptor;
        pack4(response);
        return session->sock->write(&response, sizeof response);
    }
    if (stmt->cursor->isEmpty()) { 
        response = cli_not_found;
        pack4(response);
        return session->sock->write(&response, sizeof response);
    }
    char* old_data = stmt->buf + sizeof(cli_oid_t) + 4;
    for (cb = stmt->columns; cb != NULL; cb = cb->next) { 
        cb->ptr = new_data;
        old_data += 1; // skip column type
        if (cb->cliType == cli_decimal) {       
            if (cb->fd->indexType & (HASHED|INDEXED)
                && strcmp(new_data, old_data) != 0)
            {
                cb->fd->attr |= dbFieldDescriptor::Updated;
            }
            new_data += strlen(new_data) + 1;
            old_data += strlen(old_data) + 1;
        } else if (cb->cliType >= cli_asciiz && (cb->cliType <= cli_array_of_string || cb->cliType == cli_array_of_wstring)) {
            int new_len = unpack4(new_data);
            int old_len = unpack4(old_data);
            cb->len = new_len;
            if (cb->cliType == cli_cstring) { 
                cb->len += 1; // add '\0'
            }
            if (cb->fd->indexType & (HASHED|INDEXED) 
                && memcmp(new_data, old_data, (cb->cliType >= cli_wstring ? new_len*sizeof(wchar_t) : new_len) + 4) != 0)
            {
                cb->fd->attr |= dbFieldDescriptor::Updated;
            }
            new_data += 4;
            old_data += 4;
            if (cb->cliType == cli_array_of_string) {
                while (--new_len >= 0) { 
                    new_data += strlen(new_data) + 1;
                }
                while (--old_len >= 0) { 
                    old_data += strlen(old_data) + 1;
                }
            } else if (cb->cliType == cli_array_of_wstring) {
                while (--new_len >= 0) { 
                    new_data += (wcslen((wchar_t*)new_data) + 1)*sizeof(wchar_t);
                }
                while (--old_len >= 0) { 
                    old_data += (wcslen((wchar_t*)old_data) + 1)*sizeof(wchar_t);
                }
            } else if (cb->cliType == cli_array_of_decimal) {
                while (--new_len >= 0) { 
                    new_data += strlen(new_data) + 1;
                }
                while (--old_len >= 0) {
                    old_data += strlen(old_data) + 1;
                }
            } else if (cb->cliType == cli_wstring || cb->cliType == cli_pwstring) {
                new_data += new_len*sizeof(wchar_t);
                old_data += old_len*sizeof(wchar_t);                
            } else if (cb->cliType >= cli_array_of_oid) {
                new_data += new_len * sizeof_type[cb->cliType - cli_array_of_oid];
                old_data += old_len * sizeof_type[cb->cliType - cli_array_of_oid];
            } else {
                new_data += new_len;
                old_data += old_len;
            }
        } else { 
            int size = sizeof_type[cb->cliType];
            if (cb->fd->indexType & (HASHED|INDEXED) 
                && memcmp(new_data, old_data, size) != 0)
            {
                cb->fd->attr |= dbFieldDescriptor::Updated;
            }       
            new_data += size;
            old_data += size;
        }
    }
    db->beginTransaction(dbDatabase::dbExclusiveLock);

    dbRecord* rec = db->getRow(stmt->cursor->currId);
    dbTableDescriptor* table = stmt->query.table;
    dbFieldDescriptor *first = table->columns, *fd = first;
    size_t offs = table->fixedSize;
    do { 
        if (fd->type == dbField::tpArray || fd->type == dbField::tpString || fd->type == dbField::tpWString) 
        { 
            int len = ((dbVarying*)((char*)rec + fd->dbsOffs))->size;
            for (cb = stmt->columns; cb != NULL; cb = cb->next) { 
                if (cb->fd == fd) { 
                    len = cb->len;
                    break;
                }
            }
            offs = DOALIGN(offs, fd->components->alignment)
                + len*fd->components->dbsSize;
            if (fd->components->type == dbField::tpString) { 
                if (cb != NULL) { 
                    char* src = cb->ptr + 4;
                    while (--len >= 0) { 
                        int n = (int)strlen(src)+1;
                        offs += n;
                        src += n;
                    }
                } else { 
                    dbVarying* v = (dbVarying*)((char*)rec + ((dbVarying*)((char*)rec + fd->dbsOffs))->offs);
                    while (--len >= 0) { 
                        offs += v->size;
                        v += 1;
                    }
                }
            } else if (fd->components->type == dbField::tpWString) { 
                if (cb != NULL) { 
                    char* src = cb->ptr + 4;
                    while (--len >= 0) { 
                        int n = (int)((wcslen((wchar_t*)src)+1)*sizeof(wchar_t));
                        offs += n;
                        src += n;
                    }
                } else { 
                    dbVarying* v = (dbVarying*)((char*)rec + ((dbVarying*)((char*)rec + fd->dbsOffs))->offs);
                    while (--len >= 0) { 
                        offs += v->size*sizeof(wchar_t);
                        v += 1;
                    }
                }
            }
        }
    } while ((fd = fd->next) != first);

    old_data = new char[rec->size];
    memcpy(old_data, rec, rec->size);

    for (cb = stmt->columns; cb != NULL; cb = cb->next) { 
        if (cb->fd->attr & dbFieldDescriptor::Updated) {            
            if (cb->fd->indexType & HASHED) { 
                dbHashTable::remove(db, cb->fd, stmt->cursor->currId);
            }
            if (cb->fd->indexType & INDEXED) { 
                if (cb->fd->type == dbField::tpRectangle) { 
                    dbRtree::remove(db, cb->fd->tTree, stmt->cursor->currId,
                                    cb->fd->dbsOffs);
                } else { 
                    dbTtree::remove(db, cb->fd->tTree, stmt->cursor->currId, 
                                    cb->fd->type, (int)cb->fd->dbsSize, cb->fd->_comparator, cb->fd->dbsOffs);
                }
            }
        }
    }
    
    db->modified = true;
    new_data = (char*)db->putRow(stmt->cursor->currId, offs);    

    fd = first;
    offs = table->fixedSize;
    do { 
        if (fd->type == dbField::tpArray || fd->type == dbField::tpString || fd->type == dbField::tpWString) 
        { 
            int len = ((dbVarying*)(old_data + fd->dbsOffs))->size;
            offs = DOALIGN(offs, fd->components->alignment);
            ((dbVarying*)(new_data + fd->dbsOffs))->offs = (int)offs;
            for (cb = stmt->columns; cb != NULL; cb = cb->next) { 
                if (cb->fd == fd) { 
                    len = cb->unpackArray(new_data, offs);
                    break;
                }
            }
            ((dbVarying*)(new_data + fd->dbsOffs))->size = len;
            if (cb == NULL) { 
                memcpy(new_data + offs, 
                       old_data + ((dbVarying*)(old_data + fd->dbsOffs))->offs,
                       len*fd->components->dbsSize);
                if (fd->components->type == dbField::tpString) { 
                    dbVarying* new_str = (dbVarying*)(new_data + offs);
                    dbVarying* old_str = (dbVarying*)(old_data + ((dbVarying*)(old_data + fd->dbsOffs))->offs);
                    int relOffs = len*sizeof(dbVarying);
                    offs += relOffs;
                    while (--len >= 0) { 
                        int strlen = old_str->size;
                        new_str->offs = relOffs;
                        memcpy(new_data + offs, (char*)old_str + old_str->offs, strlen);
                        relOffs += strlen - sizeof(dbVarying);
                        offs += strlen;
                        old_str += 1;
                        new_str += 1;
                    }
                } else if (fd->components->type == dbField::tpWString) { 
                    dbVarying* new_str = (dbVarying*)(new_data + offs);
                    dbVarying* old_str = (dbVarying*)(old_data + ((dbVarying*)(old_data + fd->dbsOffs))->offs);
                    int relOffs = len*sizeof(dbVarying);
                    offs += relOffs;
                    while (--len >= 0) { 
                        int strlen = old_str->size*sizeof(wchar_t);
                        new_str->offs = relOffs;
                        memcpy(new_data + offs, (char*)old_str + old_str->offs, strlen);
                        relOffs += strlen - sizeof(dbVarying);
                        offs += strlen;
                        old_str += 1;
                        new_str += 1;
                    }
                } else {
                    offs += len*fd->components->dbsSize;
                }                   
            }
        } else {             
            for (cb = stmt->columns; cb != NULL; cb = cb->next) { 
                if (cb->fd == fd) { 
                    if (cb->cliType == cli_autoincrement) { // autoincrement column is ignored
                        cb = NULL;
                        break;
                    }
                    cb->unpackScalar(new_data, false);
                    break;
                }
            }
            if (cb == NULL) { 
                memcpy(new_data + fd->dbsOffs, old_data + fd->dbsOffs, 
                       fd->dbsSize);
            }
        }
    } while ((fd = fd->next) != first);

    delete[] old_data;

    for (cb = stmt->columns; cb != NULL; cb = cb->next) { 
        if (cb->fd->attr & dbFieldDescriptor::Updated) {            
            cb->fd->attr &= ~dbFieldDescriptor::Updated;
            if (cb->fd->indexType & HASHED) { 
                dbHashTable::insert(db, cb->fd, stmt->cursor->currId, 0);
            }
            if (cb->fd->indexType & INDEXED) { 
                if (cb->fd->type == dbField::tpRectangle) { 
                    dbRtree::insert(db, cb->fd->tTree, stmt->cursor->currId,
                                    cb->fd->dbsOffs);
                } else { 
                    dbTtree::insert(db, cb->fd->tTree, stmt->cursor->currId, 
                                    cb->fd->type, (int)cb->fd->dbsSize, cb->fd->_comparator, cb->fd->dbsOffs);
                }
            }
        }
    }
    response = cli_ok;
    pack4(response);
    return session->sock->write(&response, sizeof response);
}    


            
char* dbServer::checkColumns(dbStatement* stmt, int n_columns, 
                             dbTableDescriptor* desc, char* data, 
                             int4& response, bool select)
{
    dbColumnBinding** cpp = &stmt->columns;
    response = cli_ok;
    while (--n_columns >= 0) {
        int cliType = *data++;
        char* columnName = data;
        dbSymbolTable::add(columnName, tkn_ident, true);    
        dbFieldDescriptor* fd = desc->findSymbol(columnName);
        data += strlen(data) + 1;
        if (fd != NULL) { 
            if ((cliType == cli_any && select
                 && (fd->type <= dbField::tpReference 
                     || (fd->type == dbField::tpArray 
                         && fd->components->type <= dbField::tpReference)))
                || (cliType == cli_oid 
                 && fd->type == dbField::tpReference)
                || (cliType == cli_rectangle
                    && fd->type == dbField::tpRectangle)
                || (((cliType >= cli_bool && cliType <= cli_int8) || cliType == cli_autoincrement)
                    && fd->type >= dbField::tpBool 
                    && fd->type <= dbField::tpInt8)
                || (cliType >= cli_real4 && cliType <= cli_real8 
                    && fd->type >= dbField::tpReal4
                    && fd->type <= dbField::tpReal8)
                || ((cliType == cli_asciiz || cliType == cli_pasciiz || cliType == cli_cstring) 
                    && fd->type == dbField::tpString)
                || ((cliType == cli_wstring || cliType == cli_pwstring) && fd->type == dbField::tpWString)
                || (cliType == cli_array_of_oid && 
                    fd->type == dbField::tpArray && 
                    fd->components->type == dbField::tpReference)
                || (cliType == cli_decimal 
                    && fd->type >= dbField::tpInt1 
                    && fd->type <= dbField::tpReal8)
                || (cliType == cli_datetime
                    && ((fd->type == dbField::tpStructure
                         && fd->components->type == dbField::tpInt4) 
                        || fd->type ==  dbField::tpInt4))                   
                || (cliType >= cli_array_of_bool
                    && fd->type  == dbField::tpArray
                    && fd->components->type <= dbField::tpReference
                    && cliType-cli_array_of_oid == fd2cli_type_mapping[fd->components->type]))
            {
                dbColumnBinding* cb = new dbColumnBinding(fd, cliType);
                *cpp = cb;
                cpp = &cb->next;
            } else { 
                response = cli_incompatible_type;
                break;
            }
        } else { 
            TRACE_MSG(("Field '%s' not found\n", columnName));
            response = cli_column_not_found;
            break;
        }
    }
    return data;
}


bool dbServer::insert(dbSession* session, int stmt_id, char* data, bool prepare)
{
    dbStatement* stmt = findStatement(session, stmt_id);
    dbTableDescriptor* desc = NULL;
    dbColumnBinding* cb;
    int4   response;
    char   reply_buf[sizeof(cli_oid_t) + 8];
    char*  dst;
    oid_t  oid = 0;
    size_t offs;
    size_t size;
    int    i, n_columns;
    dbFieldDescriptor* fd;

    if (stmt == NULL) { 
        if (!prepare) { 
            response = cli_bad_statement;
            goto return_response;
        }
        stmt = new dbStatement(stmt_id);
        stmt->next = session->stmts;
        session->stmts = stmt;
    } else {
        if (prepare) { 
            stmt->reset();
        } else if ((desc = stmt->table) == NULL) {
            response = cli_bad_descriptor;
            goto return_response;
        }
    }
    if (prepare) { 
        session->scanner.reset(data);
        if (session->scanner.get() != tkn_insert 
            || session->scanner.get() != tkn_into
            || session->scanner.get() != tkn_ident) 
        {
            response = cli_bad_statement;
            goto return_response;
        }
        desc = db->findTable(session->scanner.ident);
        if (desc == NULL) {     
            response = cli_table_not_found;
            goto return_response;
        }
        data += strlen(data)+1;
        n_columns = *data++ & 0xFF;
        data = checkColumns(stmt, n_columns, desc, data, response, false);
        if (response != cli_ok) { 
            goto return_response;
        }
        stmt->table = desc;
    }
 
    offs = desc->fixedSize;
    for (cb = stmt->columns; cb != NULL; cb = cb->next) { 
        cb->ptr = data;
        if (cb->cliType == cli_decimal) {
            data += strlen(data) + 1;
        } else if (cb->cliType == cli_datetime) {
            data += 4;
        } else if (cb->cliType == cli_autoincrement) {
            ;
        } else if (cb->cliType >= cli_asciiz && (cb->cliType <= cli_array_of_string || cb->cliType == cli_array_of_wstring)) {
            int len = unpack4(data);
            cb->len = len;
            if (cb->cliType == cli_cstring) { 
                cb->len += 1; // add '\0'
            }
            offs = DOALIGN(offs, cb->fd->components->alignment)
                 + cb->len*cb->fd->components->dbsSize;
            data += 4;
            if (cb->cliType == cli_array_of_string) { 
                while (--len >= 0) { 
                    int n = (int)strlen(data) + 1; 
                    offs += n;
                    data += n;
                }
            } else if (cb->cliType == cli_array_of_wstring) { 
                while (--len >= 0) { 
                    int n = (int)((wcslen((wchar_t*)data) + 1)*sizeof(wchar_t)); 
                    offs += n;
                    data += n;
                }
            } else if (cb->cliType == cli_array_of_decimal) { 
                while (--len >= 0) { 
                    data += strlen(data) + 1; 
                }
            } else {
                data += len*cb->fd->components->dbsSize;
            }
        } else {
            data += sizeof_type[cb->cliType];
        }
    }
    size = DOALIGN(offs, sizeof(wchar_t)) + sizeof(wchar_t); // reserve one byte for not initialize strings
    db->beginTransaction(dbDatabase::dbExclusiveLock);
    db->modified = true;
    oid = db->allocateRow(desc->tableId, size);
#ifdef AUTOINCREMENT_SUPPORT
    desc->autoincrementCount = ((dbTable*)db->getRow(desc->tableId))->count;
#endif
    dst = (char*)db->getRow(oid);    
    memset(dst + sizeof(dbRecord), 0, size - sizeof(dbRecord));

    offs = desc->fixedSize;
    for (cb = stmt->columns; cb != NULL; cb = cb->next) { 
        fd = cb->fd;
        if (fd->type == dbField::tpArray || fd->type == dbField::tpString || fd->type == dbField::tpWString) {
            offs = DOALIGN(offs, fd->components->alignment);
            ((dbVarying*)(dst + fd->dbsOffs))->offs = (int)offs;
            ((dbVarying*)(dst + fd->dbsOffs))->size = cb->len;
            cb->unpackArray(dst, offs);
        } else { 
            cb->unpackScalar(dst, true);
        }
    }
    fd = desc->columns; 
    for (i = (int)desc->nColumns; --i >= 0; fd = fd->next) { 
        if ((fd->type == dbField::tpString || fd->type == dbField::tpWString) 
            && ((dbVarying*)(dst + fd->dbsOffs))->offs == 0)
        {
            ((dbVarying*)(dst + fd->dbsOffs))->size = 1;
            ((dbVarying*)(dst + fd->dbsOffs))->offs = (int)(size - sizeof(wchar_t));
        }
    }
    for (cb = stmt->columns; cb != NULL; cb = cb->next) { 
        if (cb->fd->indexType & HASHED) { 
            dbHashTable::insert(db, cb->fd, oid, 0);
        }
        if (cb->fd->indexType & INDEXED) { 
            if (cb->fd->type == dbField::tpRectangle) { 
                dbRtree::insert(db, cb->fd->tTree, oid, cb->fd->dbsOffs);
            } else { 
                dbTtree::insert(db, cb->fd->tTree, oid, 
                                cb->fd->type, (int)cb->fd->dbsSize, cb->fd->_comparator, cb->fd->dbsOffs);
            }
        }
    }
    response = cli_ok;
  return_response:
    pack4(reply_buf, response);
    if (desc == NULL) { 
        pack4(reply_buf+4, 0);
    } else { 
#ifdef AUTOINCREMENT_SUPPORT
        pack4(reply_buf+4, desc->autoincrementCount);
#else
        pack4(reply_buf+4, ((dbTable*)db->getRow(desc->tableId))->nRows);
#endif
    }
    pack_oid(reply_buf+8, oid);
    return session->sock->write(reply_buf, sizeof reply_buf);
}    


bool dbServer::describe_table(dbSession* session, char const* table)
{
    dbTableDescriptor* desc = db->findTableByName(table);
    if (desc == NULL) {
        char response[8];
        pack4(response, 0);
        pack4(response+4, -1);
        return session->sock->write(response, sizeof response);
    } else { 
        int i, length = 0;
        dbFieldDescriptor* fd = desc->columns; 
        for (i = (int)desc->nColumns; --i >= 0;) { 
            length += (int)strlen(fd->name)+2+3;
            if (fd->refTableName != NULL) {  
                length += (int)strlen(fd->refTableName);
            } else if (fd->type == dbField::tpArray && fd->components->refTableName != NULL) {
                length += (int)strlen(fd->components->refTableName);
            }
            if (fd->inverseRefName != NULL) { 
                length += (int)strlen(fd->inverseRefName);
            }
            fd = fd->next;
        }
        dbSmallBuffer response(length+8);
        char* p = (char*)response;
        pack4(p, length);
        pack4(p+4, (int)desc->nColumns);
        p += 8;
        for (i = (int)desc->nColumns, fd = desc->columns; --i >= 0;) { 
            int flags = 0;
            *p++ = map_type(fd);
            if (fd->tTree != 0) { 
                flags |= cli_indexed;
            }
            if (fd->hashTable != 0) { 
                flags |= cli_hashed;
            }
            *p++ = (char)flags;
            strcpy(p, fd->name);
            p += strlen(fd->name)+1;
            if (fd->refTableName != NULL) {
                strcpy(p, fd->refTableName);
                p += strlen(p) + 1;
            } else if (fd->type == dbField::tpArray && fd->components->refTableName != NULL) {
                strcpy(p, fd->components->refTableName);
                p += strlen(p) + 1;
            } else { 
                *p++ = '\0';
            }
            if (fd->inverseRefName != NULL) {
                strcpy(p, fd->inverseRefName);
                p += strlen(p) + 1;
            } else { 
                *p++ = '\0';
            }
            fd = fd->next;
        }
        return session->sock->write(response, length+8);
    }
}

bool dbServer::show_tables(dbSession* session)
{
    dbTableDescriptor* desc=db->tables;
    if (desc == NULL) {
        char response[8];
        pack4(response, 0);
        pack4(response+4, -1);
        return session->sock->write(response, sizeof response);
    } else {
        int length = 0, n = 0;
        for (desc=db->tables; desc != NULL; desc=desc->nextDbTable) {
            if (strcmp(desc->name, "Metatable")) {
                length += (int)strlen(desc->name)+1;
                n++;
            }
        }
        dbSmallBuffer response(length+8);
        char* p = (char*)response;
        pack4(p, length);
        pack4(p+4, n);
        p += 8;
        for (desc=db->tables; desc != NULL; desc=desc->nextDbTable) {
            if (strcmp(desc->name, "Metatable")) {
                strcpy(p, desc->name);
                p += strlen(desc->name)+1;
            }
        }
        return session->sock->write(response, length+8);
    }
}

bool dbServer::create_table(dbSession* session, char* data, bool create)
{
    db->beginTransaction(dbDatabase::dbExclusiveLock);
    db->modified = true;
    char* tableName = data;
    data += strlen(data) + 1;
    int nColumns = *data++ & 0xFF;
    cli_field_descriptor* columns = new cli_field_descriptor[nColumns];    
    for (int i = 0; i < nColumns; i++) {
        columns[i].type = (cli_var_type)*data++;
        columns[i].flags = *data++ & 0xFF;
        columns[i].name = data;
        data += strlen(data) + 1;
        if (*data != 0) { 
            columns[i].refTableName = data;
            data += strlen(data) + 1;
        } else { 
            columns[i].refTableName = NULL;
            data += 1;
        }
        if (*data != 0) { 
            columns[i].inverseRefFieldName = data;
            data += strlen(data) + 1;
        } else { 
            columns[i].inverseRefFieldName = NULL;
            data += 1;
        }
    }
    if (session->existed_tables == NULL) { 
        session->existed_tables = db->tables;
    }
    int4 response;
    if (create) {
        if (session->existed_tables == NULL) { 
            session->existed_tables = db->tables;
        }
        response = dbCLI::create_table(db, tableName, nColumns, columns);
    } else { 
        response = dbCLI::alter_table(db, tableName, nColumns, columns);
    }
    pack4(response);
    return session->sock->write(&response, sizeof response);
}

bool dbServer::drop_table(dbSession* session, char* tableName)
{
    db->beginTransaction(dbDatabase::dbExclusiveLock);
    dbTableDescriptor* desc = db->findTableByName(tableName);
    int4 response = cli_ok;
    if (desc != NULL) {
        db->dropTable(desc);
        if (desc == session->existed_tables) { 
            session->existed_tables = desc->nextDbTable;
        }
        db->unlinkTable(desc);
        desc->nextDbTable = session->dropped_tables;
        session->dropped_tables = desc;
    } else { 
        response = cli_table_not_found;
    }
    pack4(response);
    return session->sock->write(&response, sizeof response);
}

bool dbServer::alter_index(dbSession* session, char* data)
{
    char* tableName = data;
    data += strlen(data) + 1;
    char* fieldName = data;
    data += strlen(data) + 1;
    int newFlags = *data++ & 0xFF;
    int4 response = dbCLI::alter_index(db, tableName, fieldName, newFlags);
    pack4(response);
    return session->sock->write(&response, sizeof response);
}


bool dbServer::select(dbSession* session, int stmt_id, char* msg, bool prepare)
{
    int4 response;
    int i, n_params, tkn, n_columns;
    dbStatement* stmt = findStatement(session, stmt_id);
    dbCursorType cursorType;
    dbTableDescriptor* desc;

    if (prepare) { 
        if (stmt == NULL) { 
            stmt = new dbStatement(stmt_id);
            stmt->next = session->stmts;
            session->stmts = stmt;
        } else { 
            stmt->reset();
        }
        stmt->n_params = *msg++ & 0xFF;
        stmt->n_columns = n_columns = *msg++ & 0xFF;
        stmt->params = new dbParameterBinding[stmt->n_params];
        int len = unpack2(msg);
        msg += 2;
        session->scanner.reset(msg);
        char *p, *end = msg + len;
        if (session->scanner.get() != tkn_select) { 
            response = cli_bad_statement;
            goto return_response;
        }
        if ((tkn = session->scanner.get()) == tkn_all) { 
            tkn = session->scanner.get();
        }
        if (tkn == tkn_from && session->scanner.get() == tkn_ident) { 
            if ((desc = db->findTable(session->scanner.ident)) != NULL) { 
                msg = checkColumns(stmt, n_columns, desc, end, response, true);
                if (response != cli_ok) {
                    goto return_response;
                }
                stmt->cursor = new dbAnyCursor(*desc, dbCursorViewOnly, NULL);
                stmt->cursor->setPrefetchMode(false);
            } else { 
                response = cli_table_not_found;
                goto return_response;
            }           
        } else { 
            response = cli_bad_statement;
            goto return_response;
        }
        p = session->scanner.p;
        for (i = 0; p < end; i++) { 
            stmt->query.append(dbQueryElement::qExpression, p);
            p += strlen(p) + 1;
            if (p < end) { 
                int cliType = *p++;
                static const dbQueryElement::ElementType type_map[] = { 
                    dbQueryElement::qVarReference, // cli_oid
                    dbQueryElement::qVarBool,      // cli_bool
                    dbQueryElement::qVarInt1,      // cli_int1 
                    dbQueryElement::qVarInt2,      // cli_int2
                    dbQueryElement::qVarInt4,      // cli_int4
                    dbQueryElement::qVarInt8,      // cli_int8
                    dbQueryElement::qVarReal4,     // cli_real4
                    dbQueryElement::qVarReal8,     // cli_real8
                    dbQueryElement::qVarUnknown,   // cli_decimal
                    dbQueryElement::qVarStringPtr, // cli_asciiz
                    dbQueryElement::qVarStringPtr, // cli_pasciiz
                    dbQueryElement::qVarUnknown,   // cli_cstring
                    dbQueryElement::qVarUnknown,   // cli_array_of_oid,
                    dbQueryElement::qVarUnknown,   // cli_array_of_bool
                    dbQueryElement::qVarUnknown,   // cli_array_of_int1
                    dbQueryElement::qVarUnknown,   // cli_array_of_int2
                    dbQueryElement::qVarUnknown,   // cli_array_of_int4
                    dbQueryElement::qVarUnknown,   // cli_array_of_int8
                    dbQueryElement::qVarUnknown,   // cli_array_of_real4
                    dbQueryElement::qVarUnknown,   // cli_array_of_real8
                    dbQueryElement::qVarUnknown,   // cli_array_of_decimal
                    dbQueryElement::qVarUnknown,   // cli_array_of_string
                    dbQueryElement::qVarUnknown,   // cli_any
                    dbQueryElement::qVarInt4,      // cli_datetime
                    dbQueryElement::qVarUnknown,   // cli_autoincrement
                    dbQueryElement::qVarRectangle, // cli_rectanlge
                    dbQueryElement::qVarWStringPtr,// cli_wstring
                    dbQueryElement::qVarWStringPtr,// cli_pwstring
                    dbQueryElement::qVarUnknown,   // cli_array_of_wstring
                    dbQueryElement::qVarUnknown,   // cli_unknown
                };
                stmt->params[i].type = cliType;
                stmt->query.append(type_map[cliType], &stmt->params[i].u);
            }
        }
    } else { 
        if (stmt == NULL) { 
            response = cli_bad_descriptor;
            goto return_response;
        }
    }
    stmt->firstFetch = true;
    cursorType = *msg++ ? dbCursorForUpdate : dbCursorViewOnly;
    for (i = 0, n_params = stmt->n_params; i < n_params; i++) { 
        switch (stmt->params[i].type) { 
          case cli_oid:
            stmt->params[i].u.oid = unpack_oid(msg);
            msg += sizeof(cli_oid_t);
            break;
          case cli_int1:
            stmt->params[i].u.i1 = *msg++;
            break;
          case cli_int2:
            msg = unpack2((char*)&stmt->params[i].u.i2, msg);
            break;
          case cli_int4:
            msg = unpack4((char*)&stmt->params[i].u.i4, msg);
            break;
          case cli_int8:
            msg = unpack8((char*)&stmt->params[i].u.i8, msg);
            break;
          case cli_real4:
            msg = unpack4((char*)&stmt->params[i].u.r4, msg);
            break;
          case cli_real8:
            msg = unpack8((char*)&stmt->params[i].u.r8, msg);
            break;
          case cli_bool:
            stmt->params[i].u.b = *msg++;
            break;
          case cli_asciiz:
          case cli_pasciiz:
            stmt->params[i].u.str = msg;
            msg += strlen(msg) + 1;
            break;
          case cli_rectangle:
            assert(sizeof(cli_rectangle_t) == sizeof(rectangle));
            msg = unpack_rectangle((cli_rectangle_t*)&stmt->params[i].u.rect, msg);
            break;
          default:
            response = cli_bad_statement;
            goto return_response;           
        }
    } 
#ifdef THROW_EXCEPTION_ON_ERROR
    try { 
        response = (int)stmt->cursor->select(stmt->query, cursorType);
    } catch (dbException const& x) { 
        response = (x.getErrCode() == dbDatabase::QueryError)
            ? cli_bad_statement : cli_runtime_error;
    }
#else
    { 
        dbDatabaseThreadContext* ctx = db->threadContext.get();
        ctx->catched = true;
        int errorCode = setjmp(ctx->unwind);
        if (errorCode == 0) { 
            response = (int)stmt->cursor->select(stmt->query, cursorType);
        } else { 
            response = (errorCode == dbDatabase::QueryError)
                ? cli_bad_statement : cli_runtime_error;
        }
        ctx->catched = false;
    }
#endif  
  return_response:
    pack4(response);
    return session->sock->write(&response, sizeof response);
}


void dbServer::serveClient()
{
    dbStatement *sp, **spp;
    db->attach();
    while (true) {
        dbSession* session; 
        {   
            dbCriticalSection cs(mutex);
            do { 
                go.wait(mutex);
                if (cancelWait) { 
                    nIdleThreads -= 1;
                    done.signal();
                    db->detach();
                    return;
                }
            } while (waitList == NULL);

            session = waitList;
            waitList = waitList->next;
            session->next = activeList;
            activeList = session;
            nIdleThreads -= 1;
            nActiveThreads += 1;
            waitListLength -= 1;
        }
        cli_request req;
        int4 response = cli_ok;
        bool online = true;
        while (online && session->sock->read(&req, sizeof req)) { 
            req.unpack();
            int length = req.length - sizeof(req);
            dbSmallBuffer msg(length);
            if (length > 0) { 
                if (!session->sock->read(msg, length)) {
                    break;
                }
            }
            switch(req.cmd) { 
              case cli_cmd_close_session:
                while (session->dropped_tables != NULL) {
                    dbTableDescriptor* next = session->dropped_tables->nextDbTable;
                    delete session->dropped_tables;
                    session->dropped_tables = next;
                }
                db->commit();
                session->in_transaction = false;
                online = false;
                break;
              case cli_cmd_prepare_and_execute:
                online = select(session, req.stmt_id, msg, true); 
                session->in_transaction = true;
                break;
              case cli_cmd_execute:
                online = select(session, req.stmt_id, msg, false); 
                break;
              case cli_cmd_get_first:
                online = get_first(session, req.stmt_id);
                break;
              case cli_cmd_get_last:
                online = get_last(session, req.stmt_id);
                break;
              case cli_cmd_get_next:
                online = get_next(session, req.stmt_id);
                break;
              case cli_cmd_get_prev:
                online = get_prev(session, req.stmt_id);
                break;
              case cli_cmd_skip:
                online = skip(session, req.stmt_id, msg);
                break;
              case cli_cmd_seek:
                online = seek(session, req.stmt_id, msg);
                break;
              case cli_cmd_freeze:
                online = freeze(session, req.stmt_id);
                break;
              case cli_cmd_unfreeze:
                online = unfreeze(session, req.stmt_id);
                break;
              case cli_cmd_free_statement:
                for (spp = &session->stmts; (sp = *spp) != NULL; spp = &sp->next)
                {
                    if (sp->id == req.stmt_id) { 
                        *spp = sp->next;
                        delete sp;
                        break;
                    }
                }
                break;
              case cli_cmd_abort:
                while (session->dropped_tables != NULL) {
                    dbTableDescriptor* next = session->dropped_tables->nextDbTable;
                    db->linkTable(session->dropped_tables, session->dropped_tables->tableId);
                    session->dropped_tables = next;
                }
                if (session->existed_tables != NULL) { 
                    while (db->tables != session->existed_tables) { 
                        dbTableDescriptor* table = db->tables;
                        db->unlinkTable(table);
                        delete table;
                    }
                    session->existed_tables = NULL;
                }
                db->rollback();
                session->in_transaction = false;
                online = session->sock->write(&response, sizeof response);
                break;
              case cli_cmd_commit:              
                while (session->dropped_tables != NULL) {
                    dbTableDescriptor* next = session->dropped_tables->nextDbTable;
                    delete session->dropped_tables;
                    session->dropped_tables = next;
                }
                session->existed_tables = NULL;
                db->commit();
                session->in_transaction = false;
                online = session->sock->write(&response, sizeof response);
                break;
              case cli_cmd_precommit:
                db->precommit();
                online = session->sock->write(&response, sizeof response);
                break;
              case cli_cmd_lock:
                db->lock();
                break;
              case cli_cmd_update:
                online = update(session, req.stmt_id, msg);
                break;          
              case cli_cmd_remove:
                online = remove(session, req.stmt_id);
                break;          
              case cli_cmd_remove_current:
                online = remove_current(session, req.stmt_id);
                break;          
              case cli_cmd_prepare_and_insert:
                online = insert(session, req.stmt_id, msg, true);
                session->in_transaction = true;
                break;          
              case cli_cmd_insert:
                online = insert(session, req.stmt_id, msg, false);
                break;          
              case cli_cmd_describe_table:
                online = describe_table(session, (char*)msg);
                break;
              case cli_cmd_show_tables:
                online = show_tables(session);
                break;
              case cli_cmd_create_table:
                online = create_table(session, msg, true);
                break;
              case cli_cmd_alter_table:
                online = create_table(session, msg, false);
                break;
              case cli_cmd_drop_table:
                online = drop_table(session, msg);
                break;
              case cli_cmd_alter_index:
                online = alter_index(session, msg);
                break;
            }
        }       
        if (session->in_transaction) { 
            while (session->dropped_tables != NULL) {
                dbTableDescriptor* next = session->dropped_tables->nextDbTable;
                db->linkTable(session->dropped_tables, session->dropped_tables->tableId);
                session->dropped_tables = next;
            }
            if (session->existed_tables != NULL) { 
                while (db->tables != session->existed_tables) { 
                    dbTableDescriptor* table = db->tables;
                    db->unlinkTable(table);
                    delete table;
                }
                session->existed_tables = NULL;
            }
            db->rollback();
        }
        // Finish session
        {   
            dbCriticalSection cs(mutex);
            dbSession** spp;
            delete session->sock;
            for (spp = &activeList; *spp != session; spp = &(*spp)->next); 
            *spp = session->next;
            session->next = freeList;
            freeList = session;
            nActiveThreads -= 1;
            if (cancelSession) { 
                done.signal();
                break;
            }
            if (nActiveThreads + nIdleThreads >= optimalNumberOfThreads) {
                break;
            }
            nIdleThreads += 1;
        } 
    }
    db->detach();
}

void dbServer::acceptConnection(socket_t* acceptSock)
{
    while (true) { 
        socket_t* sock = acceptSock->accept();
        dbCriticalSection cs(mutex);
        if (cancelAccept) { 
            return;
        }
        if (sock != NULL) { 
            if (freeList == NULL) { 
                freeList = new dbSession;
                freeList->next = NULL;
            }
            dbSession* session = freeList;
            freeList = session->next;
            session->sock = sock;
            session->stmts = NULL;
            session->next = waitList;
            session->in_transaction = false;
            session->existed_tables = NULL;
            session->dropped_tables = NULL;
            waitList = session;
            waitListLength += 1;
            if (nIdleThreads < waitListLength) { 
                dbThread thread;
                nIdleThreads += 1;
                thread.create(serverThread, this);
                thread.detach();
            }
            go.signal();
        }
    }
}

dbServer::~dbServer()
{
    dbServer** spp;
    for (spp = &chain; *spp != this; spp = &(*spp)->next);
    *spp = next;
    delete globalAcceptSock;
    delete localAcceptSock;
    delete[] URL;
}

END_FASTDB_NAMESPACE




