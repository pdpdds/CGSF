#pragma once
#include <map>

class BasePacket;
class INetworkCallback;

class ILogicEntry
{
	typedef std::map<int, INetworkCallback*> mapConnectorCallback;
public:
	ILogicEntry(void){}
	virtual ~ILogicEntry(void){}

	virtual bool Initialize() = 0;
	virtual bool ProcessPacket(BasePacket* pPacket) = 0;

	bool AddConnectorCallback(int identifier, INetworkCallback* pCallback){ m_mapConnectorCallback.insert(std::make_pair(identifier, pCallback)); return true; }
	virtual bool ProcessConnectorPacket(BasePacket* pPacket);

protected:
	mapConnectorCallback m_mapConnectorCallback;
};
