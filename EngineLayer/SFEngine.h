#pragma once
#include "IEngine.h"
#include "SFConfig.h"
#include "INetworkEngine.h"
#include "IPacketProtocol.h"
#include "ILogicDispatcher.h"
#include "ILogicEntry.h"

class SFEngine : public IEngine
{
public:
	SFEngine(void);
	virtual ~SFEngine(void);

	BOOL Start(char* szIP, unsigned short Port);
	BOOL ShutDown();

	virtual ISessionService* CreateSessionService() override;
	BOOL CreateLogicThread(ILogicEntry* pLogic);
	BOOL CreateEngine(char* szModuleName, bool Server = false);
	
	virtual bool OnConnect(int Serial) override;
	virtual bool OnDisconnect(int Serial) override;
	virtual bool OnTimer(const void *arg) override;

	BOOL SendRequest(BasePacket* pPacket);

	INetworkEngine* GetNetworkEngine(){return m_pNetworkEngine;}

	SFConfig* GetConfig(){return &m_Config;}
	void SetConfig(SFConfig& Config){m_Config = Config;}

	void SetPacketProtocol(IPacketProtocol* pProtocol){m_pPacketProtocol = pProtocol;}
	IPacketProtocol* GetPacketProtocol(){return m_pPacketProtocol;}
	void SetLogicDispathcer(ILogicDispatcher* pDispatcher){m_pLogicDispatcher = pDispatcher;}

	ILogicDispatcher* GetLogicDispatcher(){return m_pLogicDispatcher;}
protected:
	
private:
	
	SFConfig m_Config;
	HINSTANCE m_EngineHandle;
	INetworkEngine* m_pNetworkEngine;
	IPacketProtocol* m_pPacketProtocol;
	ILogicDispatcher* m_pLogicDispatcher;
};
/*
	BOOL Send( int Serial, int PacketID, char* pBuffer, int BufferSize )
	{
		int HeaderSize = sizeof(SFPacketHeader);

		SFPacket PacketSend;

		PacketSend.SetPacketID(PacketID);

		PacketSend.MakePacket((BYTE*)pBuffer, BufferSize, CGSF_PACKET_OPTION);

		return GetNetworkEngine()->Send(Serial, (char*)PacketSend.GetHeader(), PacketSend.GetHeaderSize() + PacketSend.GetDataSize());
	}
*/