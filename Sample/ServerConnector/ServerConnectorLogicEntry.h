#pragma once
#include "ILogicEntry.h"

class BasePacket;

class ServerConnectorLogicEntry : public ILogicEntry
{
public:
	ServerConnectorLogicEntry(void);
	virtual ~ServerConnectorLogicEntry(void);

	virtual bool Initialize() override;
	virtual bool ProcessPacket(BasePacket* pBasePacket) override;
};

