#pragma once
#include "ILogicDispatcher.h"

class SFClassicDispatcher : public ILogicDispatcher
{
public:
	SFClassicDispatcher();
	virtual ~SFClassicDispatcher();

	virtual void Dispatch(BasePacket* pPacket) override;
	
	virtual bool CreateLogicSystem(ILogicEntry* pLogicEntry) override;
	virtual bool ShutDownLogicSystem() override;
};

