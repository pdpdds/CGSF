/*-< CLEANUPSEM.CPP >------------------------------------------------*--------*/
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     05-Aug-2004  K.A. Knizhnik  * / [] \ *
//                          Last update: 05-Aug-2004  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Unix utility to cleanup FastDB semaphores
//-------------------------------------------------------------------*--------*

#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#ifdef USE_POSIX_SEMAPHORES
#include <semaphore.h>  // For POSIX style semaphores
#endif

static char const* keyFileDir = "/tmp/";

int hashFunction(char const* s) 
{ 
    int ch, h = 0;    
    while ((ch = *s++) != '\0') { 
        h ^= ch;
    }
    h &= 0xFF;
    if (h == 0) { 
        h = 1;
    }
    return h;
}

int getKey(char const* databaseName, char const* suffix) { 
    char buf[1024];
    *buf = '\0';
    if (strchr(databaseName, '/') == NULL) { 
        strcpy(buf, keyFileDir);
    }
    strcat(buf, databaseName);
    strcat(buf, suffix);
#if defined(USE_STD_FTOK) 
    return ftok(buf, hashFunction(buf));
#else
    struct stat st;
    if (::stat(buf, &st) < 0) { 
        return (key_t)-1;
    }
    return (key_t)(((st.st_dev & 0x7f) << 24) ^ (st.st_ino & 0x7fffffff));
#endif
}

#ifdef USE_POSIX_SEMAPHORES
void removeSemaphore(char const* databaseName, char const* suffix) { 
    char buf[256];
    *buf = '\0';
    if (*databaseName != '/') { 
        strcpy(buf, "/");
    }
    strcat(buf, databaseName);
    strcat(buf, suffix);
    int rc = sem_unlink(buf);
    if (rc < 0) { 
        fprintf(stderr, "Failed to remove semaphore %s\n", buf);
    }
}
#else
void removeSemaphore(char const* databaseName, char const* suffix) { 
    int key = getKey(databaseName, suffix);
    if (key >= 0) { 
        int id = semget(key, 0, 0);
        if (id >= 0) { 
            int rc = semctl(id, 0, IPC_RMID, NULL);
            if (rc < 0) { 
                fprintf(stderr, "Failed to remove semaphore %s%s\n", databaseName, suffix);
            }
        } else { 
            fprintf(stderr, "Failed to get key for semaphore %s%s\n", databaseName, suffix);
        }
    }
}
#endif

void removeSharedMemory(char const* databaseName, char const* suffix) { 
    int key = getKey(databaseName, suffix);
    if (key >= 0) { 
        int id = shmget(key, 0, 0);
        if (id >= 0) { 
            int rc = shmctl(id, IPC_RMID, NULL);
            if (rc < 0) { 
                fprintf(stderr, "Failed to remove schared section %s%s\n", databaseName, suffix);
            }
        } else { 
            fprintf(stderr, "Failed to get key for semaphore %s%s\n", databaseName, suffix);
        }
    }
}
        

int main(int argc, char* argv[]) { 
    if (argc < 2) { 
        printf("Usage: cleanupsem DATABASE-NAME\n");
        return 1;
    }
    char* databaseName = argv[1];    
    removeSharedMemory(databaseName, ".dm");    
    removeSharedMemory(databaseName, ".fdb");    
    removeSemaphore(databaseName, ".in");    
    removeSemaphore(databaseName, ".ws");    
    removeSemaphore(databaseName, ".rs");    
    removeSemaphore(databaseName, ".us");    
    removeSemaphore(databaseName, ".bce");
    removeSemaphore(databaseName, ".dce");
    removeSemaphore(databaseName, ".cs");
    removeSemaphore(databaseName, ".mcs");
    printf("All semaphores are removed\n");
    return 0;
}
