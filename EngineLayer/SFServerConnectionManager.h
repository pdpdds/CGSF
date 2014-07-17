#pragma once
#include "SFServerBridge.h"
#include <list>

class IServerConnector;

class SFServerConnectionManager
{
	typedef std::map<int, SFServerBridge*> mapServerInfo;

public:
	SFServerConnectionManager();
	virtual ~SFServerConnectionManager();

	bool LoadConnectionServerList(WCHAR* szFileName);
	mapServerInfo& GetServerList(){ return m_mapServerInfo; }
	mapServerInfo& GetConnectedServer(){ return m_mapConnectedServer; }

protected:
	bool InitServerList(WCHAR* szFileName);

private:
	mapServerInfo m_mapServerInfo;
	mapServerInfo m_mapConnectedServer;
	std::list<SFServerBridge> m_listDisonnectedServer;
};
