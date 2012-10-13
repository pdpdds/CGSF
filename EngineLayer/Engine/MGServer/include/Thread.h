#ifndef Thread_H
#define Thread_H

#pragma once

#include "variver.h"
#include <windows.h>

class Thread;
class Runnable
{
public:
	virtual void run(Thread* info) = 0;
};

class Thread
{
protected:

	Runnable*				runner;
	volatile unsigned int	isStarted;
	unsigned		identifier;
	HANDLE			handle;

protected:

	//static DWORD WINAPI		handleRunner(LPVOID parameter);
	static unsigned __stdcall handleRunner(void* parameter);

public:
	
	Thread();
	Thread(Runnable* r);
	virtual ~Thread();

	virtual	unsigned int isStart() { return isStarted;}

	virtual	DWORD resume();

	virtual void run();

	virtual	DWORD start();

	virtual	void sleep(long millis);	

	virtual	void stop();

	virtual	DWORD suspend();	

	virtual	void terminated();

	virtual	bool waitFor();

};

#endif