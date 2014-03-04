/*-< INSPECTSEM.CPP >------------------------------------------------*--------*/
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     05-Aug-2004  K.A. Knizhnik  * / [] \ *
//                          Last update: 05-Aug-2004  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Unix utility to inspect values FastDB semaphores
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
void inspectSemaphore(char const* databaseName, char const* suffix) { 
    char buf[256];
    *buf = '\0';
    if (*databaseName != '/') { 
        strcpy(buf, "/");
    }
    strcat(buf, databaseName);
    strcat(buf, suffix);
    sem_t* sem = sem_open(buf, 0);
    if (sem == SEM_FAILED) { 
        fprintf(stderr, "Failed to open semaphore %s\n", buf);
    } else { 
        int value;
        int rc = sem_value(sem, &value);
        if (rc < 0) { 
            fprintf(stderr, "Failed to get semaphore value\n", buf);
        } else {
            printf("Semaphore %s: value=%d\n", buf, value);
        }
        sem_close(sem);
    }
}
#else
void inspectSemaphore(char const* databaseName, char const* suffix) { 
    int key = getKey(databaseName, suffix);
    if (key >= 0) { 
        int id = semget(key, 0, 0);
        if (id >= 0) { 
            struct semid_ds ds;
            int rc = semctl(id, 0, IPC_STAT, &ds);
            if (rc < 0) { 
                fprintf(stderr, "Failed to get state of semaphore %s%s\n", databaseName, suffix);
            }
            printf("Semaphore %s%s: id=%d {\n",  databaseName, suffix, id);
            for (int i = 0, n_sems = ds.sem_nsems; i < n_sems; i++) { 
                printf("\tVAL=%d, PID=%d, NCNT=%d, ZCNT=%d\n", 
                       semctl(id, i, GETVAL, 0), semctl(id, i, GETPID, 0), semctl(id, i, GETNCNT, 0), semctl(id, i, GETZCNT, 0));
            }
            printf("}\n");
        } else { 
            fprintf(stderr, "Failed to get semaphore %s%s\n", databaseName, suffix);
        }
    } else { 
        fprintf(stderr, "Failed to get key for semaphore %s%s\n", databaseName, suffix);
    }
}
#endif
        

int main(int argc, char* argv[]) { 
    if (argc < 2) { 
        printf("Usage: cleanupsem DATABASE-NAME\n");
        return 1;
    }
    char* databaseName = argv[1];    
    inspectSemaphore(databaseName, ".in");    
    inspectSemaphore(databaseName, ".ws");    
    inspectSemaphore(databaseName, ".rs");    
    inspectSemaphore(databaseName, ".us");    
    inspectSemaphore(databaseName, ".bce");
    inspectSemaphore(databaseName, ".dce");
    inspectSemaphore(databaseName, ".cs");
    inspectSemaphore(databaseName, ".mcs");
    return 0;
}
