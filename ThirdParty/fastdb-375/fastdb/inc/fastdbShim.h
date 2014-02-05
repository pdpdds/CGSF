/* This is a new file that has been added to fastdb sources as part of the porting effort. This file
 * captures stubbed implementation of functionality that is not available in the CFW VxWorks image
 * such that fastdb can be made to work. fastdbShim.h.changes file gives details of the stubbed calls
 */
#ifndef __fastdbShim__
#define __fastdbShim__

#define IPC_CREAT 0
#define IPC_RMID 0

#include "vkiWrap.h"
//#include "platform_specific_include.h"

#include "sockLib.h"
#include "selectLib.h"
#include <vxWorks.h>
#include <ioLib.h>
#include <sys/times.h>
#include <sys/times.h>

#include "socket.h"
#include "hostLib.h"
#include "sockLib.h"
#include "selectLib.h"

#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>

extern "C" {
#include <netdb.h>
}

#include <signal.h>

#define getpid taskIdSelf

// VxWorks puts the module id in the high order bits of errno
#if defined(VXWORKS) && defined(BUILDING_FASTDB)
#undef errno
#define errno   ((__errnoRef()) & 0x7fff)
#endif // VXWORKS && BUILDING_FASTDB


extern "C" {

#define UTSNAME_SHORT_NAME_SIZE 80
#define UTSNAME_LONG_NAME_SIZE  256
#define UTSNAME_NUMBER_SIZE 8

// We need to include proper header file
struct utsname
    {
    char sysname[UTSNAME_SHORT_NAME_SIZE];  /* operating system name */
    char nodename[UTSNAME_LONG_NAME_SIZE];  /* network node name */
    char release[UTSNAME_SHORT_NAME_SIZE];  /* OS release level */
    char version[UTSNAME_LONG_NAME_SIZE];   /* operating system version */
    char machine[UTSNAME_LONG_NAME_SIZE];   /* hardware type (BSP model) */
    char endian[UTSNAME_NUMBER_SIZE];       /* architecture endianness */
    char kernelversion[UTSNAME_SHORT_NAME_SIZE];/* VxWorks kernel version */
    char processor[UTSNAME_SHORT_NAME_SIZE];    /* CPU type */
    char bsprevision[UTSNAME_SHORT_NAME_SIZE];  /* VxWorks BSP revision */
    char builddate[UTSNAME_SHORT_NAME_SIZE];    /* VxWorks kernel build date */
    };

}

extern "C" int gettimeofday(struct timeval *tp, void *tzp);

extern "C" int getpagesize();

extern "C" double sqrt(double n);

int shmget(key_t key, size_t size, int shmflg);
void *shmat(int shmid, const void *shmaddr, int shmflg);
int shmdt(const void *shmaddr);
int shmctl(int shmid, int cmd, void *buf);

int uname (struct utsname *u);

#endif /* __fastdbShim__ */
