#include "stdafx.h"
#include "TCPCallback.h"
#include "SFNetworkEntry.h"
#include "BasePacket.h"
#include <string>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "SFProtobufPacket.h"
#include "CasualGameManager.h"
#include "CasualGameGUI.h"
#include "NetworkSystem.h"
#include <SFPacketStore/PacketID.h>
#include <SFPacketStore/SFPacketStore.pb.h>

extern CasualGameManager* g_pCasualGameManager;

TCPCallback::TCPCallback(CasualGameManager* pOwner)
	: m_pOwner(pOwner)
	//: m_Serial(-1)
{
}

TCPCallback::~TCPCallback(void)
{
}

bool TCPCallback::HandleNetworkMessage(BasePacket* pPacket)
{
	switch(pPacket->GetPacketID())
	{
	case CGSF::Auth:
		{
			m_pOwner->GetGUI()->ChangeState(GUI_STATE_LOGIN);
		}
		break;
	case CGSF::EnterLobby:
		{
			m_pOwner->GetGUI()->ChangeState(GUI_STATE_LOBBY);
		}
		break;
	case CGSF::CreateRoom:
	case CGSF::EnterRoom:
		{
			m_pOwner->GetGUI()->ChangeState(GUI_STATE_ROOM);
		}
		break;
	case CGSF::LeaveRoom:
		{
			m_pOwner->GetGUI()->ChangeState(GUI_STATE_LOBBY);
		}
		break;
	case CGSF::LoadingStart:
		{
			m_pOwner->GetGUI()->ChangeState(GUI_STATE_LOADING);
		}
		break;
	case CGSF::PlayStart:
		{
			m_pOwner->GetGUI()->ChangeState(GUI_STATE_GAME);
		}
		break;

	case CGSF::PlayEnd:
		{
			if(m_pOwner->GetGUI()->GetGUIState() == GUI_STATE_GAME)
			{
				SFProtobufPacket<SFPacketStore::LeaveRoom> pktLeaveRoom(CGSF::LeaveRoom);
				g_pCasualGameManager->GetNetwork()->TCPSend(&pktLeaveRoom);		
			}
		}
		break;

	case CGSF::DeletePeer:
		{
			if(m_pOwner->GetGUI()->GetGUIState() == GUI_STATE_GAME)
			{
				SFProtobufPacket<SFPacketStore::LeaveRoom> pktLeaveRoom(CGSF::LeaveRoom);
				g_pCasualGameManager->GetNetwork()->TCPSend(&pktLeaveRoom);		
			}
		}
		break;

	default:
		{
			m_pOwner->GetGUI()->Notify(pPacket);
		}
	}

	return true;
}

void TCPCallback::HandleConnect(int Serial)
{
	SetSerial(Serial);
	printf("Connected\n");

}

void TCPCallback::HandleDisconnect(int Serial)
{
	m_pOwner->GetGUI()->QuitMessage("Network Connection Loss!!");
	printf("Disconnected\n");
}