#pragma once
#include <list>
#include "SFLock.h"

class IServerConnector;

class SFServerConnectionManager
{

public:
	SFServerConnectionManager();
	virtual ~SFServerConnectionManager();

	bool SetupServerReconnectSys();
	bool LoadConnectorList(WCHAR* szFileName);
	HANDLE m_hTimerEvent;
	bool SetConnectorState(int connectorId, bool connected);

	void AddConnectorInfo(_ConnectorInfo& connectorInfo);

protected:
	UINT static __stdcall ServerReconnectProc(LPVOID arg);

private:
	std::list<_ConnectorInfo> m_listConnectorInfo;
	HANDLE m_hThread;
	DWORD  m_dwThreadID;

	SFLock m_Lock;
};
