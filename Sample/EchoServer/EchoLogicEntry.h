#pragma once
#include "ILogicEntry.h"

class BasePacket;

class EchoLogicEntry : public ILogicEntry
{
public:
	EchoLogicEntry(void);
	virtual ~EchoLogicEntry(void);

	virtual BOOL Initialize() override;
	virtual BOOL ProcessPacket(BasePacket* pBasePacket) override;
};

