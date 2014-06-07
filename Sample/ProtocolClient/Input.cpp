#include "stdafx.h"
#include "Input.h"
#include "SFNetworkEntry.h"
#include "SFMsgPackPacket.h"
#include "SFMsgPackProtocol.h"
#include "SFProtobufPacket.h"
#include "SFPacket.h"
#include "SFMessage.h"
#include <string>
#include <iostream>
#include "../../Common/SFPacketStore/ProtocolPacket.pb.h"
#include "../../Common/SFPacketStore/ProtocolPacketID.h"
#include <msgpack.hpp>
#include "AvroClientProtocol.h"
#include "SFPacketStore/AvroProtocolPacket.hh"
#include "SFAvroPacket.h"
#include "avro/buffer/BufferStream.hh"
#include "avro/buffer/BufferReader.hh"
#include "avro/Avroserialize.hh"

void ProcessCGSFInput(void* Args)
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

void ProcessProtobufInput(void* Args)
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

void ProcessMsgPackInput(void* Args)
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

void ProcessAvroInput(void* Args)
{
	std::string input;

	while (SFNetworkEntry::GetInstance()->IsConnected())
	{
		std::cin >> input;

		if (input.compare("exit") == 0)
			break;

		if (input.compare("1") == 0)
		{
			SFAvroPacket<c::Sample1> packet(Protocol::Sample1);
			packet.GetData().move = 1115.0f;
			packet.GetData().speed = 1116.0f;

			SFNetworkEntry::GetInstance()->TCPSend(&packet);
		}
		if (input.compare("2") == 0)
		{

		}
		if (input.compare("3") == 0)
		{

		}
		if (input.compare("4") == 0)
		{

		}
	}
}

void ProcessInput(void* func)
{

	int inputThreadID = ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)func, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY);

	SFASSERT(inputThreadID != -1);

	while (SFNetworkEntry::GetInstance()->IsConnected())
	{
		SFNetworkEntry::GetInstance()->Update();

		Sleep(1);
	}

	ACE_Thread_Manager::instance()->wait_grp(inputThreadID);
}