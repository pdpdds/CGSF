#pragma once
#include <map>

class BasePacket;

class ILogicEntry
{
public:
	ILogicEntry(void){m_bP2PService = false;}
	virtual ~ILogicEntry(void){}

	virtual bool Initialize() = 0;
	virtual bool ProcessPacket(BasePacket* pPacket) = 0;
	virtual bool ProcessServerPacket(int acceptorId, BasePacket* pPacket) { UNREFERENCED_PARAMETER(acceptorId); UNREFERENCED_PARAMETER(pPacket);  return false; }

	void SetP2PService(bool bP2PService){ m_bP2PService = bP2PService; }
	bool GetP2PService(){ return m_bP2PService; }

private:
	bool m_bP2PService;
};
