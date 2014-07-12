#pragma once
#include "IPacketTask.h"

class SFPacketDelaySendTask : public IPacketTask
{
public:
	SFPacketDelaySendTask();
	virtual ~SFPacketDelaySendTask();

	virtual bool Execute() override;

protected:

private:
};