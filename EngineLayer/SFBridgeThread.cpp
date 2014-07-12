#include "StdAfx.h"
#include "SFBridgeThread.h"
#include "SFPacketDelaySendTask.h"

void PacketSendThread(void* Args)
{
	while (1)
	{
		SFPacketDelaySendTask* pPacketTask = (SFPacketDelaySendTask*)PacketSendSingleton::instance()->PopTask();

		if (pPacketTask == NULL)
			break;

		pPacketTask->Execute();

		PacketDelayedSendTask::instance()->Release((SFPacketDelaySendTask*)pPacketTask);
	}
}