#pragma once
#include "ILogicEntry.h"
#include "SFDispatch.h"

class BasePacket;

class ProtocolLogicEntry : public ILogicEntry
{
public:
	ProtocolLogicEntry(void);
	virtual ~ProtocolLogicEntry(void);

	virtual bool Initialize() override;
	virtual bool ProcessPacket(BasePacket* pBasePacket) override;

protected:
	bool OnPacketSample1(BasePacket* pPacket);
	bool OnPacketSample2(BasePacket* pPacket);
	bool OnPacketSample3(BasePacket* pPacket);
	bool OnPacketSample4(BasePacket* pPacket);

private:
	SFDispatch<USHORT, std::tr1::function<BOOL(BasePacket*)>, BasePacket*> m_Dispatch;
};