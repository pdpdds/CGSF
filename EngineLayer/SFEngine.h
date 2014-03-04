#pragma once
#include "IEngine.h"
#include "SFConfigure.h"
#include "INetworkEngine.h"
#include "IPacketProtocol.h"
#include "ILogicDispatcher.h"
#include "ILogicEntry.h"

class SFEngine : public IEngine
{
	friend class SFTCPNetwork;
public:
	SFEngine(_TCHAR* pArg);
	virtual ~SFEngine(void);

	BOOL Start();
	BOOL Start(char* szIP, unsigned short Port);
	BOOL Intialize(ILogicEntry* pLogicEntry, IPacketProtocol* pProtocol, ILogicDispatcher* pDispatcher);
	BOOL ShutDown();

	virtual ISessionService* CreateSessionService() override;
	
	virtual bool OnConnect(int Serial) override;
	virtual bool OnDisconnect(int Serial) override;
	virtual bool OnTimer(const void *arg) override;

	BOOL AddTimer(int timerID, DWORD period, DWORD delay);
	BOOL SendRequest(BasePacket* pPacket);

	INetworkEngine* GetNetworkEngine(){return m_pNetworkEngine;}

	SFConfigure* GetConfig(){return &m_Config;}
	void SetConfig(SFConfigure& Config){m_Config = Config;}

	
	IPacketProtocol* GetPacketProtocol(){return m_pPacketProtocol;}
	ILogicDispatcher* GetLogicDispatcher(){return m_pLogicDispatcher;}

protected:

	BOOL CreateLogicThread(ILogicEntry* pLogic);
	BOOL CreatePacketSendThread();
	BOOL CreateEngine(char* szModuleName, bool Server = false);
	
private:
	SFConfigure m_Config;
	int m_PacketSendThreadId;
	int m_LogicThreadId;

	HINSTANCE m_EngineHandle;
	INetworkEngine* m_pNetworkEngine;
	IPacketProtocol* m_pPacketProtocol;
	ILogicDispatcher* m_pLogicDispatcher;

	void SetPacketProtocol(IPacketProtocol* pProtocol){m_pPacketProtocol = pProtocol;}
	void SetLogicDispathcer(ILogicDispatcher* pDispatcher){m_pLogicDispatcher = pDispatcher;}
};