#pragma once
#include <list>

class IPacketProtocol;

class SFPacketProtocolManager
{
	typedef std::map<int, IPacketProtocol*> mapPacketProtocol;
	typedef std::list<_ListenerInfo> listListenerInfo;
	typedef std::list<_ConnectorInfo> listConnectorInfo;

public:
	SFPacketProtocolManager(bool bMultiPacketProtocol);
	virtual ~SFPacketProtocolManager();
	
	bool AddPacketProtocol(int packetProtocolId, IPacketProtocol* pProtocol);
	bool AddListenerInfo(int listenerId, int packetProtocolId);
	bool AddConnectorInfo(_ConnectorInfo* pInfo);
	bool SetConnectorPacketProtocol(int connectorId, int packetProtocolId);
	bool SetConnectorState(int connectorId, bool connected);

	IPacketProtocol* GetPacketProtocol(int packetProtocolId)
	{ 
		const auto& iter = m_mapPacketProtocol.find(packetProtocolId);
		if (iter != m_mapPacketProtocol.end()) 
			return iter->second; 
		return NULL; 
	}

	IPacketProtocol* GetPacketProtocolWithListenerId(int listenerId)
	{
		for (auto& iter : m_listListenerInfo)
		{
			if (iter.listenerId == listenerId)
				return GetPacketProtocol(iter.packetProtocolId);
		}

		return NULL;
	}

	IPacketProtocol* GetPacketProtocolWithConnectorId(int connectorId)
	{
		for (auto& iter : m_listConnectorInfo)
		{
			if (iter.connectorId == connectorId)
				return GetPacketProtocol(iter.packetProtocolId);
		}

		return NULL;
	}

private:
	mapPacketProtocol m_mapPacketProtocol;
	listListenerInfo m_listListenerInfo;
	listConnectorInfo m_listConnectorInfo;
	bool m_bMultiPacketProtocol;
};