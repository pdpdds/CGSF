#include "stdafx.h"
#include "TCPCallback.h"
#include "SFNetworkEntry.h"
#include "BasePacket.h"
#include "PacketID.h"
#include <string>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "SFPacketStore.pb.h"
#include "SFProtobufPacket.h"

extern SFNetworkEntry* g_pNetworkEntry;

TCPCallback::TCPCallback(void)
	: m_Serial(-1)
{
}

TCPCallback::~TCPCallback(void)
{
}

bool TCPCallback::HandleNetworkMessage(BasePacket* pPacket)
{
	int PacketID = pPacket->GetPacketID();

	if (PacketID == CGSF::Auth)
	{
		int i = 1;
		std::string name = "cgsf";
		std::string password = "1234";

		SFProtobufPacket<SFPacketStore::Login> request(CGSF::Login);
		request.SetOwnerSerial(m_Serial);
		request.GetData().set_username(name);
		request.GetData().set_password(password);

		g_pNetworkEntry->TCPSend(&request);

		//LOG(ERROR) << "This should work";
		//LOG(ERROR) << "This should work";
	}
	else if (PacketID == CGSF::LoginSuccess)
	{
		printf("Login Success\n");
		SFProtobufPacket<SFPacketStore::EnterLobby> request(CGSF::EnterLobby);
		request.SetOwnerSerial(m_Serial);
		
		g_pNetworkEntry->TCPSend(&request);
	}
	else if (PacketID == CGSF::EnterLobby)
	{
		printf("Enter Lobby\n");

		SFProtobufPacket<SFPacketStore::CreateRoom> request(CGSF::CreateRoom);
		request.GetData().set_gamemode(3);
		request.SetOwnerSerial(m_Serial);

		g_pNetworkEntry->TCPSend(&request);
	}

	return true;
}

void TCPCallback::HandleConnect(int Serial)
{
	m_Serial = Serial;
	printf("Connected\n");
	
}

void TCPCallback::HandleDisconnect(int Serial)
{
	printf("Disconnected\n");
}