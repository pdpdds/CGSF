#include "StdAfx.h"
#include "SFBridgeThread.h"

SFIOCPQueue<int> IOCPQueue;

BOOL gServerEnd = FALSE;

void BusinessThread(void* Args)
{
	while(gServerEnd == FALSE)
	{
		SFCommand* pCommand = LogicGatewaySingleton::instance()->PopPacket();
		LogicEntrySingleton::instance()->ProcessPacket(pCommand);

		switch(pCommand->GetPacketType())
		{
		case SFCommand_Connect:
		case SFCommand_Data:
		case SFCommand_Timer:
		case SFCommand_Shouter:
		case SFCommand_Disconnect:
			{
				PacketPoolSingleton::instance()->Release((SFPacket*)pCommand);
			}
			break;
		case SFCommand_DB:
			{
				LogicEntrySingleton::instance()->RecallDBMessage((SFMessage*)pCommand);
			}
			break;
		default:
			{
				SFASSERT(0);
			}
			break;
		}
	}
}