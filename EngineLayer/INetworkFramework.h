#pragma once
/*#include "ISFAgent.h"

enum eNetworkFrameworkType
{
	NetworkFramework_SERVER = 0,
	NetworkFramework_CLIENT,
	NetworkFramework_MAX,
};

class ILogicEntry;
class IProactorAgent;
class SFPacket;

class INetworkFramework
{
public:
	INetworkFramework(){m_pNetworkAgent = NULL;}
	virtual ~INetworkFramework(void){}

	virtual BOOL Initialize(eNetworkFrameworkType Type) = 0;
	virtual BOOL Start(ILogicEntry* pLogic) = 0;
	virtual BOOL End() = 0;
	virtual BOOL Finally() = 0;

	virtual BOOL Send(int Serial, USHORT PacketID, char* pMessage, int BufSize ) = 0;
	virtual BOOL Send(int Serial, SFPacket* pPacket) = 0;

	ISFAgent* GetNetworkAgent(){return m_pNetworkAgent;}
	void SetNetworkFrameworkType(eNetworkFrameworkType Type){m_Type = Type;}

protected:
	ISFAgent* m_pNetworkAgent;

private:
	eNetworkFrameworkType m_Type;
	
};