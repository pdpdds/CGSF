#pragma once
#include "ILogicEntry.h"

class BasePacket;

class ServerListenerLogicEntry : public ILogicEntry
{
public:
	ServerListenerLogicEntry(void);
	virtual ~ServerListenerLogicEntry(void);

	virtual bool Initialize() override;
	virtual bool ProcessPacket(BasePacket* pBasePacket) override;
};

