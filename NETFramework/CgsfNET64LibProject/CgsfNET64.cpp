// 기본 DLL 파일입니다.

#include "stdafx.h"
#include "CgsfNET64.h"
#include "ServerLogicEntry.h"
#include "SFNETDispatcher.h"
#include "SFCGSFPacketProtocol.h"
#include "SFNETPacket.h"
#include "ConcurrencyPacketQueue.h"

#include <msclr\marshal.h>
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
	
	void CgsfNET64::SetNetworkConfig(NetworkConfig^ config)
	{
		if (config->MaxAcceptCount <= 0)
		{
			config->MaxAcceptCount = 5000;
		}

		auto pConfig = SFEngine::GetInstance()->GetConfig()->GetConfigureInfo();
		m_networkConfig = config;

		System::String^ serverIP = config->IP;
		System::String^ engineName = config->EngineDllName;
		
		pConfig->serverIP = msclr::interop::marshal_as<std::wstring>(serverIP);
		pConfig->serverPort = config->Port;
		pConfig->engineName = msclr::interop::marshal_as<std::wstring>(engineName);
		pConfig->maxAccept = config->MaxAcceptCount;
	}

	ERROR_CODE_N CgsfNET64::Init(NetworkConfig^ config)
	{
		SetNetworkConfig(config);

		m_packetQueue = gcnew ConcurrencyPacketQueue();

		m_pLogicEntry = new ServerLogicEntry();
		m_pLogicEntry->m_refPacketQueue = m_packetQueue;

		m_pDispatcher = new SFNETDispatcher();
		m_pDispatcher->Init(m_networkConfig->ThreadCount);

		
		const int CGSF_PACKET_OPTION_NONE = 0;
		if (m_networkConfig->MaxBufferSize <= 0) {
			m_networkConfig->MaxBufferSize = MAX_IO_SIZE;
		}

		if (m_networkConfig->MaxPacketSize <= 0) {
			m_networkConfig->MaxPacketSize = MAX_PACKET_SIZE;
		}

		auto errorCode = SFEngine::GetInstance()->Intialize(m_pLogicEntry, 
						new SFPacketProtocol<SFCGSFPacketProtocol>(m_networkConfig->MaxBufferSize, 
																m_networkConfig->MaxPacketSize, CGSF_PACKET_OPTION_NONE), 
														m_pDispatcher);
		if (errorCode != ERROR_CODE::SUCCESS)
		{
			return (ERROR_CODE_N)errorCode;
		}

		return ERROR_CODE_N::SUCCESS;
	}

	

	bool CgsfNET64::Start()
	{
		auto isResult = SFEngine::GetInstance()->Start();
		LogFlush();
		return isResult;
	}

	void CgsfNET64::Stop()
	{
		SFEngine::GetInstance()->ShutDown();
	}
	
	SFNETPacket^ CgsfNET64::GetPacket()
	{
		return m_packetQueue->Dequeue();
	}

	bool CgsfNET64::SendPacket(int sessionID, UINT16 packetID, array<Byte>^ data)
	{
		pin_ptr<Byte> pNativeData = &data[0];
		
		SFPacket sendPacket(packetID);
		sendPacket.SetSerial(sessionID);
		sendPacket.Write(pNativeData, data->Length);
		
		auto result = SFEngine::GetInstance()->SendRequest(&sendPacket);

		pNativeData = nullptr;

		return result;
	}

	void CgsfNET64::ForceDisConnect(int sessionID)
	{
		SFEngine::GetInstance()->GetNetworkEngine()->Disconnect(sessionID);
	}

	void CgsfNET64::LogFlush()
	{
		google::FlushLogFiles(google::GLOG_INFO);
		google::FlushLogFiles(google::GLOG_ERROR);
	}
}