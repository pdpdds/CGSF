/*Righteous Ninja AKA P.S. Ching codediaries.blogspot.com*/
#include <iostream>
#include "CrossHelper.h"


#ifdef WIN32 /***Windows implementations***/
unsigned int XGetLastError(){
	return GetLastError();
}
DWORD WINAPI XThreadProcedure(LPVOID lp){
	XThreadDataWrapper* pxdw = (XThreadDataWrapper*)lp;
	pxdw->XPFunction(pxdw->XPData);
	delete pxdw;
	return 0;
}
int XThreadCreate(XTHREADID* threadid, void*(*ptf)(void*), void* lp){
	XThreadDataWrapper* pxdw = new XThreadDataWrapper();
	pxdw->XPFunction = ptf;
	pxdw->XPData = lp;
	if((*threadid = CreateThread(0,0,&XThreadProcedure, pxdw, 0,0))==NULL)
		return -1;
	else
		return 0;
}
int XThreadJoin(XTHREADID * threadid){
	if(WAIT_FAILED==WaitForSingleObject(*threadid, 0))
		return -1;
	else
		return 0;
}
int XSemCreate(XSEMAPHORE *sem){
	if((*sem = CreateSemaphore(NULL,0,2,NULL))==NULL)
		return -1;
	else
		return 0;
}
int XSemTimedWait(XSEMAPHORE* sem, int milliseconds){
	if(WAIT_FAILED==WaitForSingleObject(*sem, milliseconds))
		return -1;
	else
		return 0;
}
int XSemPost(XSEMAPHORE* sem){
	if(ReleaseSemaphore(*sem,1,NULL))
		return 0;
	else
		return -1;
}
int XMutCreate(XMUTEX* mut){
	if((*mut=CreateMutex(NULL, false, NULL))==NULL)
		return -1;
	else
		return 0;
}
int XMutLock(XMUTEX* mut){
	if(WAIT_FAILED==WaitForSingleObject(*mut, 0))
		return -1;
	else
		return 0;
}
int XMutRelease(XMUTEX* mut){
	if(ReleaseMutex(*mut))
		return 0;
	else
		return -1;
}

#else /***Linux implementations***/
/*
TO DO
*/
#endif




