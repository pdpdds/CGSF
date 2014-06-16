#pragma once
#include <EngineInterface/IEngine.h>
#include <EngineInterface/INetworkEngine.h>
#include <EngineInterface/IPacketProtocol.h>
#include "SFConfigure.h"
#include "ILogicDispatcher.h"
#include "ILogicEntry.h"

class IRPCInterface;

class SFEngine : public IEngine
{
	friend class SFTCPNetwork;

public:
	virtual ~SFEngine(void);

	static SFEngine* GetInstance();
	void SetLogFolder(TCHAR* szPath = NULL);

	bool Start(char* szIP = NULL, unsigned short port = 0);
	bool Intialize(ILogicEntry* pLogicEntry, IPacketProtocol* pProtocol, ILogicDispatcher* pDispatcher = NULL);
	bool ShutDown();

	virtual ISessionService* CreateSessionService() override;
	
	virtual bool OnConnect(int Serial, bool bServerObject = false) override;
	virtual bool OnDisconnect(int Serial, bool bServerObject = false) override;
	virtual bool OnTimer(const void *arg) override;

	bool AddTimer(int timerID, DWORD period, DWORD delay);
	bool SendRequest(BasePacket* pPacket, bool bDirectSend = true);
	bool SendRequest(BasePacket* pPacket, std::vector<int>& ownerList);
	bool SendInternal(int ownerSerial, char* buffer, unsigned int bufferSize);
	bool ReleasePacket(BasePacket* pPacket);

	int  AddListener(char* szIP, unsigned short port);
	int  AddConnector(char* szIP, unsigned short port);
	void AddRPCService(IRPCService* pService);
	
	INetworkEngine* GetNetworkEngine(){return m_pNetworkEngine;}

	SFConfigure* GetConfig(){return &m_Config;}
	void SetConfig(SFConfigure& Config){m_Config = Config;}

	IPacketProtocol* GetPacketProtocol(){return m_pPacketProtocol;}
	ILogicDispatcher* GetLogicDispatcher(){return m_pLogicDispatcher;}

	bool IsServer(){ return m_isServer; }

protected:
	bool CreatePacketSendThread();
	bool CreateEngine(char* szModuleName, bool Server = false);
	
private:
	SFEngine();

	SFConfigure m_Config;
	int m_PacketSendThreadId;	

	HINSTANCE m_EngineHandle;
	INetworkEngine* m_pNetworkEngine;
	IPacketProtocol* m_pPacketProtocol;
	ILogicDispatcher* m_pLogicDispatcher;

	void SetPacketProtocol(IPacketProtocol* pProtocol){m_pPacketProtocol = pProtocol;}
	void SetLogicDispathcer(ILogicDispatcher* pDispatcher){m_pLogicDispatcher = pDispatcher;}

	static SFEngine* m_pEngine;

	bool m_isServer;
};