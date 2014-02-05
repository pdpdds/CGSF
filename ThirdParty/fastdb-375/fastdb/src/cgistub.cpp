//-< CGISTUB.CXX >---------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     27-Mar-99    K.A. Knizhnik  * / [] \ *
//                          Last update: 30-Jul-99    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Stub for CGI scripts 
//-------------------------------------------------------------------*--------*

#include "stdtp.h"
#include "sockio.h"

USE_FASTDB_NAMESPACE

void message(char const* msg)
{
    printf("Content-type: text/html\r\n\r\n"
           "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">"
           "<HTML><HEAD><TITLE>CGI STUB message</TITLE></HEAD><BODY>"
           "<H1><FONT COLOR=\"#FF0000\">%s</FONT></H1></BODY></HTML>", msg);
    exit(0);
}


int main() 
{ 
    char*  request = getenv("QUERY_STRING");
    char*  buf;
    size_t length;
    size_t buf_size;
    char* host = getenv("HTTP_HOST");
    char* script = getenv("SCRIPT_NAME");
    char* peer = getenv("REMOTE_ADDR");
    assert(host != NULL);
    assert(script != NULL);

    if (request == NULL || *request == '\0') { 
        char* length_str = getenv("CONTENT_LENGTH");
        if (length_str == NULL || sscanf(length_str, "%d", &length) != 1) { 
            message("Failed to receive data using POST method");
        }
        buf_size = length + 14 + 6 + strlen(host) + strlen(script) + strlen(peer);
        buf = new char[buf_size+1];
        if (fread(buf, 1, length, stdin) != length) { 
            message("Failed to read data using POST method");
        }
    } else {
        length = strlen(request);
        buf_size = length + 14 + 6 + strlen(host) + strlen(script) + strlen(peer);
        buf = new char[buf_size+1];
        memcpy(buf, request, length);
    }
    if (length <= 7 || strncmp(buf, "socket=", 7) != 0) {
        message("No socket field specified");
    }
    sprintf(buf+length, "&stub=http://%s%s&peer=%s&", host, script, peer);
    char* addr = buf + 7;
    char *src = addr, *dst = addr;
    while (*src != '&') { 
        if (*src == '+') {
            *dst++ = ' ';
            src += 1;
        } else if (*src == '%') { 
            *dst++ = ((src[1] >= 'A' ? src[1] - 'A'+ 10 : src[1] - '0') << 4) |
                     (src[2] >= 'A' ? src[2] - 'A'+ 10 : src[2] - '0');
            src += 3;
        } else { 
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    socket_t* s = socket_t::connect(addr);
    if (s == NULL) { 
        message("Failed to connect to application");
    }
    *(int*)buf = (int)buf_size;
    buf[4] = (char)(src - buf - 3);
    if (!s->write(buf, (int)buf_size)) { 
        message("Failed to send query to application");
    }
    if (s->read(buf, 4, 4) != 4) { 
        message("Failed to read reply length");
    }
    nat4 reply_length = *(int4*)buf;
    if (reply_length > buf_size) { 
        delete[] buf;
        buf = new char[reply_length];
    } 
    int size = reply_length - sizeof(reply_length);
    if (s->read(buf, size, size) != size) { 
        message("Failed to read reply");
    }
    fwrite(buf, 1, reply_length - sizeof(reply_length), stdout);
    delete[] buf;
    s->close();
    return 0;
}



