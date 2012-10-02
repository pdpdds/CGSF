#include "stdafx.h"
#include "TCPCallback.h"
#include "SFNetworkEntry.h"

#include "PacketID.h"
#include <string>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "SFPacketStore.pb.h"

extern SFNetworkEntry* g_pNetworkEntry;

TCPCallback::TCPCallback(void)
{
}

TCPCallback::~TCPCallback(void)
{
}

bool TCPCallback::HandleNetworkMessage(int PacketID, BYTE* pBuffer, USHORT Length)
{
	if (PacketID == CGSF::Auth)
	{
		int i = 1;
		std::string name = "cgsf";
		std::string password = "1234";

		SFPacketStore::Login PktLogin;
		PktLogin.set_username(name);
		PktLogin.set_password(password);

		int BufSize = PktLogin.ByteSize();

		char Buffer[2048] = {0,};

		if(BufSize != 0)
		{
			::google::protobuf::io::ArrayOutputStream os(Buffer, BufSize);
			PktLogin.SerializeToZeroCopyStream(&os);
		}

		g_pNetworkEntry->TCPSend(CGSF::Login, (char*)&Buffer, BufSize);

		//LOG(ERROR) << "This should work";
		//LOG(ERROR) << "This should work";
	}
	else if (PacketID == CGSF::LoginSuccess)
	{
		SFPacketStore::EnterLobby PktEnterLobby;
		int BufSize = PktEnterLobby.ByteSize();

		char Buffer[2048] = {0,};

		if(BufSize != 0)
		{
			::google::protobuf::io::ArrayOutputStream os(Buffer, BufSize);
			PktEnterLobby.SerializeToZeroCopyStream(&os);
		}
		
		g_pNetworkEntry->TCPSend(CGSF::EnterLobby, (char*)&Buffer, BufSize);
	}
	else if (PacketID == CGSF::EnterLobby)
	{
		SFPacketStore::CreateRoom PktCreateRoom;
		int BufSize = PktCreateRoom.ByteSize();

		char Buffer[2048] = {0,};

		if(BufSize != 0)
		{
			::google::protobuf::io::ArrayOutputStream os(Buffer, BufSize);
			PktCreateRoom.SerializeToZeroCopyStream(&os);
		}

		g_pNetworkEntry->TCPSend(CGSF::CreateRoom, (char*)&Buffer, BufSize);
	}

	return true;
}