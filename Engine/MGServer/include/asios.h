#ifndef ASIOS_H
#define ASIOS_H

#pragma once

#include "variver.h"
#include "mytypedef.h"
#include "maths.h"
#include "Synchronized.h"

struct OverlappedOperation
{
	enum eOpType
	{
		eOpType_Write,
		eOpType_Read,
		eOpType_Connect,
	};

	OVERLAPPED		overlapped;	
	size_t			length; // Length of the actual data to send,	
	int			optype; //
	int				opcode;
	INT32			requesterID; // [요청자가 건네는 구분값] 현재 connectSocket을 위해 사용될 값. 그외 다양한 목적으로 사용할 수 있음
	size_t			maxLength;
	char*			datas; // Used to hold data to send, direct new/delete or refer from pool

	OverlappedOperation() : overlapped(), length(0), optype(eOpType_Write), opcode(0), requesterID(0), maxLength(0), datas(NULL)
	{
	}

	OverlappedOperation(size_t maxLength) : overlapped(), length(0), optype(eOpType_Write), opcode(0), requesterID(0), maxLength(maxLength), datas(NULL)
	{
		if(0 < maxLength)
			datas = new char[maxLength];
	}

	~OverlappedOperation()
	{
		if(NULL != datas)
			delete[] datas;
	}

	void initHeader()
	{
		ZeroMemory(&overlapped, sizeof(overlapped));	
		length = 0;
		optype = eOpType_Write;
		opcode = 0;
		requesterID = 0; 
	}
};

struct ASSOCKDESC
{
	ASSOCKUID		assockUid;
	LARGE_INTEGER	tick;

	ASSOCKDESC() : assockUid(), tick()
	{
	}

	ASSOCKDESC(ASSOCKUID assockUid, LARGE_INTEGER& tick) : assockUid(assockUid), tick(tick)
	{
	}

	ASSOCKDESC(ASSOCKUID assockUid, LARGE_INTEGER* atick) : assockUid(assockUid), tick()
	{
		if(atick)
		{
			tick = *atick;
		}
	}
};

class INetworkSender;
class INetworkReceiver;
struct ASSOCKDESCEX : public ASSOCKDESC
{
	INetworkSender*	psender;

	ASSOCKDESCEX() : psender()
	{
	}

	ASSOCKDESCEX(INetworkSender* psender) : psender(psender)
	{
	}
};


class AsynchSocket
{
private:
	static AsynchSocket*		sAsynchSocketPrototype;

protected:
	static ULONG_PTRGenerator	assockuidGen;
	static CriticalSectionLock	assockuidGenLock;

protected:
	SOCKET			sockID;
	ASSOCKDESCEX	assockDesc;
	LONG			referCount;
	int				closed;

public:

	static AsynchSocket* GetASPrototype()
	{
		Synchronized sync(&assockuidGenLock);
		if(NULL == sAsynchSocketPrototype)
		{
			sAsynchSocketPrototype = new AsynchSocket();
		}
		return sAsynchSocketPrototype;
	}
	
	static ASSOCKUID retrieveASSOCKUID()
	{
		Synchronized sync(&assockuidGenLock);
		ASSOCKUID id = assockuidGen.retrieve();
		return id;
	}

	static void releaseASSOCKUID(ASSOCKUID id)
	{
		Synchronized sync(&assockuidGenLock);
		assockuidGen.release(id);
	}

public:
	AsynchSocket() : sockID(INVALID_SOCKET), assockDesc(), referCount(0), closed(0)
	{
		//assockuid를 여기서 중앙관리한다.
		assockDesc.assockUid = retrieveASSOCKUID();
		QueryPerformanceCounter(&assockDesc.tick);
	}

	virtual ~AsynchSocket()
	{
		//assockuid반환은 asio를 통해서 한다.
	}

	virtual AsynchSocket* clone()
	{
		// 그냥 같은 타입의 인스턴스를 하나 생성시킨다.
		return new AsynchSocket();
	}

	virtual void handleConnected() {}
	virtual size_t makePacket(char* dest, size_t destmaxsize, char* src, size_t srcsize);
	virtual int handleCompletionOfReceive(INetworkReceiver* receiver, INetworkSender* sender, DWORD bytesTransfer, char* datas);

	int getClosed() 
	{ 
		return closed; 
	}

	ASSOCKUID getASSOCKUID()
	{
		return assockDesc.assockUid;
	}

	void setSender(INetworkSender* sender)
	{
		assockDesc.psender = sender;
	}

	void setSOCKET(SOCKET id)
	{
		sockID = id;
	}

	SOCKET getSOCKET()
	{
		return sockID;
	}

	void getASSOCKDESCEX(ASSOCKDESCEX& output)
	{
		output = assockDesc;
	}

	LONG enterIO()
	{			
		// refercount는 클래스멤버니깐 구지 4바이트 정렬을 안해도 될듯...
		return InterlockedIncrement(&referCount);
	}

	LONG exitIO()
	{		
		return InterlockedDecrement(&referCount);
	}

	bool disconnect();

	

};

class ASIOException
{	
	DWORD		errorcode;
	char		msg[128];
public:

	ASIOException( DWORD error, char* amsg ) : errorcode(error)
	{
		ZeroMemory(msg,sizeof(msg));
		if(amsg)
		{
			size_t len = strlen(amsg);
			if(sizeof(msg) > len)
			{
				strncpy_s(msg, amsg, len);
			}
		}

		// 발생자동로깅
		//	발생시 특정파일에 로그를 남기고,
		//	콜스택을 일정수준까지 출력해 준다.
	}

	virtual ~ASIOException()
	{
	}

	const char* toMsg()
	{
		return msg;
	}

	DWORD toError()
	{
		return errorcode;
	}
};

class INetworkSender
{
public:
	// 아래의 함수들은 필요에 의해 INetworkReceiver 객체에게 메소드를 던진다.
	// 따라서 INetworkReceiver 구현한 객체가 이 함수드을 호출하면 live lock에 걸린다.
	//송신 요청
	//socketUniqueId: 소켓관리번호, 연결통보시 넘겨준 관리번호 [SOCKET으로 주어지는 번호와는 다르다!], 자료구조키값임
	//length: 길이
	//data: 실제 내용
	//virtual DWORD postingSend(ASSOCKUID socketUniqueId, LARGE_INTEGER* tick, size_t length, char* data) = 0;
	virtual DWORD postingSend(ASSOCKDESC& sockdesc, size_t length, char* data) = 0;

	//연결해제 요청
	//uniqueId: 소켓관리번호, 연결통보시 넘겨준 관리번호
	virtual DWORD disconnectSocket(ASSOCKUID uniqueId, LARGE_INTEGER* tick) = 0;
	virtual DWORD disconnectSocket(ASSOCKDESC& sockdesc) = 0;

	//연결 요청
	// out_socketUniqueId : 연결요청에 사용될 소켓 관리 객체의 고유 값1이 반환될 인수
	// out_tick : 연결요청에 사용될 소켓 관리 객체의 고유 값2이 반환될 이수
	// prototype : 해당 소켓 관리 객체의 AsynchSocket을 상속 받은 타입의 원형 객체, 해당 객체를 복사하여 실제화 된 AsynchSocket 객체를 사용한다.
	//				AsynchSocket 클래스를 상속받아 재정의 하는 이유는 AsynchSocket 클래스 명세를 참고
	// ip : 연결할 호스트[서버]의 ip주소[dns는 지원하지 않음]
	// port : 연결할 호스트[서버]의 포트번호
	// return true: 연결 요청을 성공적으로 했음
	// return false: 연결 요청에 실패 하였음
	// 연결 성공여부: 이 인터페이스를 실제 구현한 객체를 생성시 넘겨준 INetworkReceiver 를 구현한 객체의 notifyConnectingResult 함수를 통해 통보 받음
	// 동기화 이슈: 이 함수를 호출하는 문맥/객체 는 INetworkReceiver::notifyConnectingResult 를 호출하는 문맥과 해당 함수를 구현한 객체와 전혀 다른것이어야 한다.
	// 이 함수는 windows xp, windows 2003 server 운영체제에서만 작동한다.
	// 20081215: ConnectEx()를 지원하지 않는 경우엔 ConnectThread로 돌아가게 처리하자.
	//virtual bool connectSocket( int& out_socketUniqueId, LARGE_INTEGER& out_tick, AsynchSocket* prototype, char* ip, int port ) = 0;
	virtual DWORD connectSocket(INT32 reqeusterID, AsynchSocket* prototype, char* ip, u_short port ) = 0;

	// 20070411
	//	socketUniqueId 반환
	//	응용계층에서 해당 값을 사용중인데, 네트워크 계층에서 이 값을 재사용해버리는 문제점을 없애기 위해서이다.
	virtual DWORD releaseSocketUniqueId( ASSOCKUID socketUniqueId ) = 0;

	virtual DWORD registerSocket(SOCKET sockid, AsynchSocket* prototype, SOCKADDR_IN& addrin) = 0;

	virtual size_t getIOMaxSize() = 0;

};



class INetworkReceiver
{
public:
	//아래의 함수들은 multi-thread들에 의해 호출되기 때문에 구현시 synchronization을 수행해야한다.
	//아래의 함수들은 AsynchIOService에 의해 호출된다. 아래의 함수 구현에서 또 다시 AsynchIOService의 메소드를
	//호출하게 되면 live lock[cross dead lock]에 걸리게 된다.
	//만약 아래의 함수로 인해 AsynchIOService 객체의 메소드를 호출할 필요가 있다면, 응용 프로그램 영역에서
	//따로 이벤트 큐에 집어넣어서 동기화 문맥이 다른곳에서 처리하게 한다.

	//소켓 연결 통보
	//socketUniqueId: 소켓관리번호, 소켓계층에서 관리하는 소켓번호가 아님
	//sender: 데이터 전송요청/접속 끊기 인터페이스 포인터
	virtual void notifyRegisterSocket(ASSOCKDESCEX& sockdesc, SOCKADDR_IN& ip) = 0;

	//소켓 연결해제 통보
	//socketUniqueId: 연결해제된 소켓의 관리번호, 연결통보시 넘겨준 관리번호
	virtual void notifyReleaseSocket(ASSOCKDESCEX& sockdesc) = 0;

	//수신된 메시지내용 통보
	//socketUniqueId: 수신된 소켓의 관리번호, 연결통보시 넘겨준 관리번호
	//length: 길이
	//data: 실제 내용
	virtual void notifyMessage(ASSOCKDESCEX& sockdesc, size_t length, char* data) = 0;

	//연결요청 결과 통보
	// INetworkSender를 통해 connectSocket을 수행시 이미 sender를 알고 있기 때문에, 부차적으로 sender를 알려줄 필요가 없다.
	//requestID:  INetworkSender.connectSocket 를 통해 건네주었던 식별값
	//socketUniqueId: 연결성공시 소켓식별값
	//tick : 연결성공시 틱, 해당 포인터의 내용을 복사해서 사용해야 한다.
	//isSuccess : 연결성공여부
	//error : 연결실패시 원인 GetLastError()참고
	//virtual void notifyConnectingResult( int socketUniqueId, LARGE_INTEGER* tick, bool isSuccess, DWORD error ) = 0;
	virtual void notifyConnectingResult(INT32 requestID, ASSOCKDESCEX& sockdesc, DWORD error) = 0;
};

#endif