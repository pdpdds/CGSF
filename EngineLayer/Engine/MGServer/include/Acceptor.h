#ifndef Acceptor_H
#define Acceptor_H

#pragma once

#include "Thread.h"
#include "asios.h"

class Acceptor : public Thread
{
	AsynchSocket*		prototype; //socket creator : prototype patterns
	SOCKET				socketValue; //listen socket
	INetworkSender*		theAsynchEventManager; 
	char				filestr[1024];
	//int				contype;

protected:

	void logmsg(char * format,...);

public:
	Acceptor(INetworkSender* theAsynchEventManager, AsynchSocket* prototype, const char* address, u_short port);
	//Acceptor(INetworkSender* theAsynchEventManager, AsynchSocket* prototype, const char* address, u_short port, int contype);
	virtual ~Acceptor();
	void run(); //acceptor loop

};


#endif
