#pragma once
#include "ILogicDispatcher.h"

class SFCasualGameDispatcher : public ILogicDispatcher
{
public:
	SFCasualGameDispatcher(void);
	virtual ~SFCasualGameDispatcher(void);

	virtual void Dispatch(BasePacket* pPacket) override;
	virtual void Finally() override;

	static void BusinessThread(void* Args);
	static void RPCThread(void* Args);
};

