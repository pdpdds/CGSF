#pragma once
#include "ErrorCode.h"
#include <EngineInterface/IEngine.h>
#include <EngineInterface/INetworkEngine.h>
#include <EngineInterface/IPacketProtocol.h>
#include <EngineInterface/EngineConstant.h>
#include "SFConfigure.h"
#include "ILogicDispatcher.h"
#include "ILogicEntry.h"
#include <vector>
#include "Macro.h"
#include "SFPacket.h"
#include "SFPacketProtocol.h"

class IRPCInterface;
class SFServerConnectionManager;
class SFPacketProtocolManager;


class SFEngine : public IEngine
{
	friend class SFTCPNetwork;

	friend class SFMsgPackProtocol;
	friend class SFProtobufProtocol;
	friend class SFAvroProtocol;
	friend class SFCGSFPacketProtocol;
	friend class SFJsonProtocol;

	typedef std::map<int, long> mapTimer;

public:
	virtual ~SFEngine(void);

	static SFEngine* GetInstance();
	void SetLogFolder(TCHAR* szPath = NULL);

	bool Start(int protocolId);	
	
	NET_ERROR_CODE Intialize(ILogicEntry* pLogicEntry, ILogicDispatcher* pDispatcher = NULL);
	bool ShutDown();

	virtual ISessionService* CreateSessionService(_SessionDesc& desc) override;
	
	virtual bool OnConnect(int serial, _SessionDesc& desc) override;
	virtual bool OnDisconnect(int serial, _SessionDesc& desc) override;
	virtual bool OnTimer(const void *arg) override;

	bool AddTimer(int timerID, DWORD period, DWORD delay);
	bool CancelTimer(int timerID);

	bool SendRequest(BasePacket* pPacket);
	bool SendRequest(BasePacket* pPacket, std::vector<int>& ownerList);

	bool SendDelayedRequest(BasePacket* pPacket);
	bool SendDelayedRequest(BasePacket* pPacket, std::vector<int>* pOwnerList = NULL);

	bool ReleasePacket(BasePacket* pPacket);

	bool Disconnect(int serial);

	SFServerConnectionManager* GetServerConnectionManager(){ return m_pServerConnectionManager; }
	bool SetupServerReconnectSys();
	bool LoadConnectorList(WCHAR* szFileName);
	int  AddListener(char* szIP, unsigned short port, int packetProtocolId);
	int  AddConnector(int connectorId, char* szIP, unsigned short port);
	void AddRPCService(IRPCService* pService);
	bool AddPacketProtocol(int packetProtocolId, IPacketProtocol* pProtocol);	

	SFConfigure* GetConfig(){return &m_Config;}
	void SetConfig(SFConfigure& Config){m_Config = Config;}

	ILogicDispatcher* GetLogicDispatcher(){return m_pLogicDispatcher;}

	bool IsServer(){ return m_isServer; }

	SFPacketProtocolManager* GetPacketProtocolManager(){ return m_pPacketProtocolManager; }

	INetworkEngine* GetNetworkEngine(){ return m_pNetworkEngine; }

	void SendToLogic(BasePacket* pMessage);

protected:
	bool CreatePacketSendThread();
	NET_ERROR_CODE CreateEngine(char* szModuleName, bool Server = false);
	
private:
	SFEngine();
	bool Start(char* szIP, unsigned short port); //클라이언트 전용, 이후 deprecated 될 것임	

	SFConfigure m_Config;
	int m_packetSendThreadId;

	HINSTANCE m_engineHandle;
	INetworkEngine* m_pNetworkEngine;

	SFPacketProtocolManager* m_pPacketProtocolManager;
	ILogicDispatcher* m_pLogicDispatcher;
	SFServerConnectionManager* m_pServerConnectionManager;

	void SetLogicDispathcer(ILogicDispatcher* pDispatcher){m_pLogicDispatcher = pDispatcher;}

	static SFEngine* m_pEngine;

	bool m_isServer;

	mapTimer m_mapTimer;
};