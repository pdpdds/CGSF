// 기본 DLL 파일입니다.

#include "stdafx.h"
#include "CgsfNET64.h"
#include "ServerLogicEntry.h"
#include "SFNETDispatcher.h"
#include "SFCGSFPacketProtocol.h"
#include "SFNETPacket.h"
#include "ConcurrencyPacketQueue.h"

#include <msclr\marshal_cppstd.h>


namespace CgsfNET64Lib {

	CgsfNET64::CgsfNET64()
	{
	}

	CgsfNET64::~CgsfNET64()
	{
		delete m_pLogicEntry;
		delete m_pDispatcher;
	}

	bool CgsfNET64::Init(int threadCount, int maxBufferSize, int maxPacketSize)
	{
		m_packetQueue = gcnew ConcurrencyPacketQueue();

		m_pLogicEntry = new ServerLogicEntry();
		m_pLogicEntry->m_refPacketQueue = m_packetQueue;

		m_pDispatcher = new SFNETDispatcher();
		m_pDispatcher->Init(threadCount);

		
		const int CGSF_PACKET_OPTION_NONE = 0;
		if (maxBufferSize <= 0) {
			maxBufferSize = MAX_IO_SIZE;
		}

		if (maxPacketSize <= 0) {
			maxPacketSize = MAX_PACKET_SIZE;
		}

		auto result = SFEngine::GetInstance()->Intialize(m_pLogicEntry, new SFPacketProtocol<SFCGSFPacketProtocol>(maxBufferSize, maxPacketSize, CGSF_PACKET_OPTION_NONE), m_pDispatcher);
		if (result == false)
		{
			return false;
		}

		SetNetworkConfigInfo();

		return true;
	}

	void CgsfNET64::SetNetworkConfigInfo()
	{
		m_networkConfig->IP = msclr::interop::marshal_as<System::String^>(SFEngine::GetInstance()->GetConfig()->GetConfigureInfo()->ServerIP);
		m_networkConfig->Port = SFEngine::GetInstance()->GetConfig()->GetConfigureInfo()->ServerPort;
		m_networkConfig->EngineDllName = msclr::interop::marshal_as<System::String^>(SFEngine::GetInstance()->GetConfig()->GetConfigureInfo()->EngineName);
	}

	void CgsfNET64::Start()
	{
		SFEngine::GetInstance()->Start();
		google::FlushLogFiles(google::GLOG_INFO);
	}

	void CgsfNET64::Stop()
	{
		SFEngine::GetInstance()->ShutDown();
	}
	
	SFNETPacket^ CgsfNET64::GetPacket()
	{
		return m_packetQueue->Dequeue();
	}

	bool CgsfNET64::SendPacket(int serial, UINT16 packetID, array<Byte>^ data)
	{
		pin_ptr<Byte> pNativeData = &data[0];
		
		SFPacket sendPacket(packetID);
		sendPacket.SetSerial(serial);
		sendPacket.Write(pNativeData, data->Length);
		
		auto result = SFEngine::GetInstance()->SendRequest(&sendPacket);

		pNativeData = nullptr;

		return result;
	}
}