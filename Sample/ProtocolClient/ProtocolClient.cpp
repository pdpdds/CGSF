// ProtocolClient.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "ProtocolCallback.h"
#include "SFNetworkEntry.h"
#include "SFPacketProtocol.h"
#include "SFCGSFPacketProtocol.h"
#include "SFProtobufPacket.h"
#include "ProtocolClientProtocol.h"
#include "SFMsgPackPacket.h"
#include "SFMsgPackProtocol.h"
#include "SFPacket.h"
#include "SFMessage.h"
#include <string>
#include <iostream>
#include "../../Common/SFPacketStore/ProtocolPacket.pb.h"
#include "../../Common/SFPacketStore/ProtocolPacketID.h"
#include <msgpack.hpp>

#pragma comment(lib, "EngineLayer.lib")

using namespace google;

void ProtoclInputThread(void* Args)
{
	std::string input;

	while (SFNetworkEntry::GetInstance()->IsConnected())
	{
		std::cin >> input;

		if (input.compare("exit") == 0)
			break;

		if (input.compare("1") == 0)
		{
			SFPacket packet(Protocol::Sample1);

			float speed = 10.0f;
			float move = 100.0f;
			packet << speed << move;

			SFNetworkEntry::GetInstance()->TCPSend(&packet);
		}
		if (input.compare("2") == 0)
		{
			SFPacket packet(Protocol::Sample2);

			SFNetworkEntry::GetInstance()->TCPSend(&packet);
		}
		if (input.compare("3") == 0)
		{
			SFPacket packet(Protocol::Sample3);

			SFNetworkEntry::GetInstance()->TCPSend(&packet);
		}
		if (input.compare("4") == 0)
		{
			SFPacket packet(Protocol::Sample4);

			SFNetworkEntry::GetInstance()->TCPSend(&packet);
		}
	}
}

void ProtoclInputThread2(void* Args)
{
	std::string input;

	while (SFNetworkEntry::GetInstance()->IsConnected())
	{
		std::cin >> input;

		if (input.compare("exit") == 0)
			break;

		if (input.compare("1") == 0)
		{
			SFProtobufPacket<ProtocolPacket::Sample1> packet(Protocol::Sample1);

			float speed = 10.0f;
			float move = 100.0f;
			
			packet.GetData().set_speed(speed);
			packet.GetData().set_move(move);

			SFNetworkEntry::GetInstance()->TCPSend(&packet);
		}
		if (input.compare("2") == 0)
		{
			SFProtobufPacket<ProtocolPacket::Sample2> packet(Protocol::Sample2);

			SFNetworkEntry::GetInstance()->TCPSend(&packet);
		}
		if (input.compare("3") == 0)
		{
			SFProtobufPacket<ProtocolPacket::Sample3> packet(Protocol::Sample3);

			SFNetworkEntry::GetInstance()->TCPSend(&packet);
		}
		if (input.compare("4") == 0)
		{
			SFProtobufPacket<ProtocolPacket::Sample4> packet(Protocol::Sample4);

			SFNetworkEntry::GetInstance()->TCPSend(&packet);
		}
	}
}

void ProtoclInputThread3(void* Args)
{
	std::string input;

	while (SFNetworkEntry::GetInstance()->IsConnected())
	{
		std::cin >> input;

		if (input.compare("exit") == 0)
			break;

		if (input.compare("1") == 0)
		{
			msgpack::sbuffer buffer;

			msgpack::packer<msgpack::sbuffer> pk(&buffer);
			pk.pack(std::string("Log message ... 1"));
			pk.pack(std::string("Log message ... 2"));
			pk.pack(std::string("Log message ... 3"));


			SFMsgPackPacket packet(Protocol::Sample1);
			msgpack::unpacker& pac = packet.GetData();
			pac.reserve_buffer(buffer.size());
			memcpy(pac.buffer(), buffer.data(), buffer.size());
			pac.buffer_consumed(buffer.size());
			
			SFNetworkEntry::GetInstance()->TCPSend(&packet);
		}
		if (input.compare("2") == 0)
		{
			SFMsgPackPacket packet(Protocol::Sample2);

			SFNetworkEntry::GetInstance()->TCPSend(&packet);
		}
		if (input.compare("3") == 0)
		{
			SFMsgPackPacket packet(Protocol::Sample3);

			SFNetworkEntry::GetInstance()->TCPSend(&packet);
		}
		if (input.compare("4") == 0)
		{
			SFMsgPackPacket packet(Protocol::Sample4);

			SFNetworkEntry::GetInstance()->TCPSend(&packet);
		}
	}
}


void ProcessInput()
{

	int inputThreadID = ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)ProtoclInputThread3, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY);

	SFASSERT(inputThreadID != -1);

	while (SFNetworkEntry::GetInstance()->IsConnected())
	{
		SFNetworkEntry::GetInstance()->Update();

		Sleep(1);
	}

	ACE_Thread_Manager::instance()->wait_grp(inputThreadID);
}

int _tmain(int argc, _TCHAR* argv[])
{
	ProtocolCallback* pCallback = new ProtocolCallback();

	SFNetworkEntry::GetInstance()->Initialize(pCallback, new SFPacketProtocol<SFMsgPackProtocol>);
	SFNetworkEntry::GetInstance()->Run();

	ProcessInput();

	SFNetworkEntry::GetInstance()->ShutDown();

	return 0;
}