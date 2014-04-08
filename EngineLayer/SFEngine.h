#pragma once
#include <EngineInterface/IEngine.h>
#include <EngineInterface/INetworkEngine.h>
#include <EngineInterface/IPacketProtocol.h>
#include "SFConfigure.h"
#include "ILogicDispatcher.h"
#include "ILogicEntry.h"

class SFEngine : public IEngine
{
	friend class SFTCPNetwork;

public:
	virtual ~SFEngine(void);

	static SFEngine* GetInstance();

	bool Start();
	bool Start(char* szIP, unsigned short Port);
	bool Intialize(ILogicEntry* pLogicEntry, IPacketProtocol* pProtocol, ILogicDispatcher* pDispatcher = NULL);
	bool ShutDown();

	virtual ISessionService* CreateSessionService() override;
	
	virtual bool OnConnect(int Serial) override;
	virtual bool OnDisconnect(int Serial) override;
	virtual bool OnTimer(const void *arg) override;

	bool AddTimer(int timerID, DWORD period, DWORD delay);
	bool SendDelayedRequest(BasePacket* pPacket);
	bool SendRequest(BasePacket* pPacket);
	bool SendInternal(int ownerSerial, char* buffer, unsigned int bufferSize);

	INetworkEngine* GetNetworkEngine(){return m_pNetworkEngine;}

	SFConfigure* GetConfig(){return &m_Config;}
	void SetConfig(SFConfigure& Config){m_Config = Config;}

	IPacketProtocol* GetPacketProtocol(){return m_pPacketProtocol;}
	ILogicDispatcher* GetLogicDispatcher(){return m_pLogicDispatcher;}

	bool ReleasePacket(BasePacket* pPacket);

	bool ServerTerminated(){ return m_bServerTerminated; }
	bool ExecuteServiceController(TCHAR* szArg);

	void SetLogFolder();

protected:

	bool CreateLogicThread(ILogicEntry* pLogic);
	bool CreatePacketSendThread();
	bool CreateEngine(char* szModuleName, bool Server = false);
	
private:
	SFEngine();

	SFConfigure m_Config;
	int m_PacketSendThreadId;
	int m_LogicThreadId;
	bool m_bServerTerminated;

	HINSTANCE m_EngineHandle;
	INetworkEngine* m_pNetworkEngine;
	IPacketProtocol* m_pPacketProtocol;
	ILogicDispatcher* m_pLogicDispatcher;

	void SetPacketProtocol(IPacketProtocol* pProtocol){m_pPacketProtocol = pProtocol;}
	void SetLogicDispathcer(ILogicDispatcher* pDispatcher){m_pLogicDispatcher = pDispatcher;}

	static SFEngine* m_pEngine;
};