#ifndef AsynchIOService_H
#define AsynchIOService_H

#pragma once

#include "variver.h"
#include "asios.h"
#include "Thread.h"
#include "Synchronized.h"
#include <deque>
#include <map>


// assocuid는 누가 관리할것인가?
//	기존에는 AsynchSocket클래스 계층에서 중앙관리햇엇다.
//		그래서 그 특징은 세부유형에 상관없이 번호가 중앙집중관리되어지는 형태로 되어 있다.
//		즉 AsynchIOService가 AsynchSocket에게 assockid를 물어보는 형태로 관리되어 있다.
//		생성과 삭제시에 글로벌하게 동기화가 일어난다.
//		똑같은 리시버가 받으면 동일한 번호가 생기는 문제점은 asio 객체로 구분할것인가?

class AsynchIOService : public INetworkSender, public Runnable
{
protected:
	INetworkReceiver*		receiver;
	DWORD					conThread;
	std::deque< Thread* >	workers;
	size_t					ioMaxSize;
	HANDLE					iocpHandle;
	
	typedef std::map<ASSOCKUID, AsynchSocket*> tASSOCKMAP;
	typedef std::map<ASSOCKUID, AsynchSocket*>::iterator tASSOCKMAPITER;
	tASSOCKMAP theASSocks;

	CriticalSectionLock		entireSynch;
	CriticalSectionLock		ioFrameSynch;
	CriticalSectionLock		logSynch;

	std::deque< OverlappedOperation* >	ioFramePools;

	char						name[ 128 ];

private:
	static size_t			cIOMINSIZE;

protected:
	OverlappedOperation* retrieveIOFrame();
	void releaseIOFrame(OverlappedOperation* op);

	void releaseSocket(AsynchSocket* socket, DWORD why);


public:
	static size_t GetIOMinSize()
	{
		return cIOMINSIZE;
	}

public:
	// receiver, 
	AsynchIOService(INetworkReceiver* receiver, size_t ioMaxSize = cIOMINSIZE, DWORD conThread=1, char* aname = NULL);
	virtual ~AsynchIOService();

	DWORD start();

	DWORD stop();

protected:
	void dispatchError(DWORD error, DWORD bytesTransfer, LPOVERLAPPED lpoverlapped, ULONG_PTR keyValue);
	void dispatchCompletion(DWORD bytesTransfer, LPOVERLAPPED lpoverlapped, ULONG_PTR keyValue);
	DWORD postingRead(AsynchSocket* socket, OverlappedOperation* op);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// implementation for INetworkSender
public:
	//virtual DWORD postingSend(ASSOCKUID socketUniqueId, LARGE_INTEGER* tick, size_t length, char* data);
	virtual DWORD postingSend(ASSOCKDESC& sockdesc, size_t length, char* data);
	virtual DWORD disconnectSocket(ASSOCKUID uniqueId, LARGE_INTEGER* tick);
	virtual DWORD disconnectSocket(ASSOCKDESC& sockdesc);
	virtual DWORD connectSocket(INT32 reqeusterID, AsynchSocket* prototype, char* ip, u_short port);
	virtual DWORD releaseSocketUniqueId( ASSOCKUID socketUniqueId );
	virtual DWORD registerSocket(SOCKET sockid, AsynchSocket* prototype, SOCKADDR_IN& addrin);
	virtual size_t getIOMaxSize() { return ioMaxSize; }

	void logmsg(char * format,...);

//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// implementation for Runnable
public:
	void run(Thread* info);

//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////



};

#endif
