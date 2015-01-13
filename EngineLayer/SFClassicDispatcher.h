#pragma once
#include "SFLogicDispatcher.h"

class SFClassicDispatcher : public SFLogicDispatcher
{
public:
	SFClassicDispatcher();
	virtual ~SFClassicDispatcher();

	virtual void Dispatch(BasePacket* pPacket) override;
	
	virtual bool CreateLogicSystem(ILogicEntry* pLogicEntry) override;
	virtual bool ShutDownLogicSystem() override;
};

