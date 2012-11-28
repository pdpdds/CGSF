#include "StdAfx.h"
#include "SFBridgeThread.h"
#include "SFPacket.h"

SFIOCPQueue<int> IOCPQueue;

BOOL gServerEnd = FALSE;

void BusinessThread(void* Args)
{
	while(gServerEnd == FALSE)
	{
		BasePacket* pPacket = LogicGatewaySingleton::instance()->PopPacket();
		LogicEntrySingleton::instance()->ProcessPacket(pPacket);

		switch(pPacket->GetPacketType())
		{
		case SFPacket_Connect:
		case SFPacket_Disconnect:
		case SFPacket_Timer:
		case SFPacket_Data:
		case SFPacket_Shouter:
			{
				delete pPacket;
			}
			break;
		
		//프로토콜 버퍼 완성한 다음에 디폴트 프로토콜 처리 고려하자...	
		//PacketPoolSingleton::instance()->Release((SFPacket*)pPacket);
			//break;
		case SFPacket_DB:
			{
				LogicEntrySingleton::instance()->ReleaseDBMessage((SFMessage*)pPacket);
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