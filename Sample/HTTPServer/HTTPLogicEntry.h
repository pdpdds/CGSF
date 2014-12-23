#pragma once
#include "ILogicEntry.h"

class BasePacket;

class HTTPLogicEntry : public ILogicEntry
{
public:
	HTTPLogicEntry(void);
	virtual ~HTTPLogicEntry(void);

	virtual bool Initialize() override;
	virtual bool ProcessPacket(BasePacket* pBasePacket) override;
};
