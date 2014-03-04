#define INSIDE_FASTDB

#include "stdtp.h"
#include <stdio.h>

BEGIN_FASTDB_NAMESPACE

static void CLI_CALLBACK_CC stderrTrace(char* msg)
{
    fputs(msg, stderr);
    fflush(stderr);
}

dbTraceFunctionPtr dbTraceFunction = stderrTrace;
bool dbTraceEnable = true;

FASTDB_DLL_ENTRY void dbTrace(char* message, ...) 
{
    if (dbTraceEnable) { 
        va_list args;
        va_start (args, message);
        char buffer[1024];
        vsprintf(buffer, message, args);
        (*dbTraceFunction)(buffer);
        va_end(args);
    }
}

#ifdef USE_DLMALLOC
extern "C" {
	void* dlmalloc(size_t);
	void  dlfree(void*);
}
#endif

FASTDB_DLL_ENTRY byte* dbMalloc(size_t size)
{
#ifdef USE_DLMALLOC
    return (byte*)dlmalloc(size);
#else
    return (byte*)malloc(size);
#endif
}

FASTDB_DLL_ENTRY void  dbFree(void* p)
{
#ifdef USE_DLMALLOC
    dlfree(p);
#else
    free(p);
#endif
}

END_FASTDB_NAMESPACE
