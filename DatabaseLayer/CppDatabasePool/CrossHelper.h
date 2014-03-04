/*Righteous Ninja AKA P.S. Ching codediaries.blogspot.com*/
#ifndef CROSSHELPER_H
#define CROSSHELPER_H

#ifdef WIN32	/***WINDOWS***/
#include <windows.h>
#define XSEMAPHORE HANDLE
#define XMUTEX HANDLE
#define XTHREADID HANDLE
DWORD WINAPI XThreadProcedure(LPVOID);

#else			/***LINUX***/
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#define XSEMAPHORE sem_t
#define XMUTEX HANDLE pthread_mutex_t
#define XTHREADID threadid_t
void* XThreadProcedure(void*);
#endif


				/***Common***/
int XSemCreate(XSEMAPHORE *);
int XSemTimedWait(XSEMAPHORE* , int);
int XSemPost(XSEMAPHORE* );
int XMutCreate(XMUTEX*);
int XMutLock(XMUTEX*);
int XMutRelease(XMUTEX*);
unsigned int XGetLastError();
int XThreadCreate(XTHREADID* ,void*(*)(void*), void*);
int XThreadJoin(XTHREADID*);
struct XThreadDataWrapper{
	void*(*XPFunction)(void*);
	void* XPData;
};

#endif