#pragma once
#include "ILogicEntry.h"

class BasePacket;

class RPCLogicEntry : public ILogicEntry
{
public:
	RPCLogicEntry(void);
	virtual ~RPCLogicEntry(void);

	virtual bool Initialize() override;
	virtual bool ProcessPacket(BasePacket* pBasePacket) override;
};

