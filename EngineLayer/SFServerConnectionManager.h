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

	bool SetupServerReconnectSys(WCHAR* szFileName);
	mapServerInfo& GetServerList(){ return m_mapServerInfo; }
	mapServerInfo& GetConnectedServer(){ return m_mapConnectedServer; }

	HANDLE m_hTimerEvent;
	std::list<SFServerBridge> m_listDisonnectedServer;

protected:
	bool InitServerList(WCHAR* szFileName);
	UINT static __stdcall ServerReconnectProc(LPVOID arg);

private:
	mapServerInfo m_mapServerInfo;
	mapServerInfo m_mapConnectedServer;
	HANDLE m_hThread;
	DWORD  m_dwThreadID;
	

	SFLock m_Lock;
};
