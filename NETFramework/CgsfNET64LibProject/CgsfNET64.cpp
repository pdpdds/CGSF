// 기본 DLL 파일입니다.

#include "stdafx.h"
#include "SFServerConnectionManager.h"
#include "CgsfNET64.h"
#include "ServerLogicEntry.h"
#include "SFNETDispatcher.h"
#include "ServerConnectReceiveCallback.h"
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
		delete m_pDispatcher;
		delete m_pLogicEntry;
		delete m_pServerConnectCallback;
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

	NET_ERROR_CODE_N CgsfNET64::Init(NetworkConfig^ config)
	{
		SetNetworkConfig(config);
		
		m_packetQueue = gcnew ConcurrencyPacketQueue();

		m_pLogicEntry = new ServerLogicEntry();
		m_pLogicEntry->m_refPacketQueue = m_packetQueue;

		m_pServerConnectCallback = new ServerConnectCallback;
		m_pServerConnectCallback->m_refPacketQueue = m_packetQueue;

		m_pDispatcher = new SFNETDispatcher();
		m_pDispatcher->Init(m_networkConfig->ThreadCount);

		
		//const int CGSF_PACKET_OPTION_NONE = 0;
		if (m_networkConfig->MaxBufferSize <= 0) {
			m_networkConfig->MaxBufferSize = MAX_IO_SIZE;
		}

		if (m_networkConfig->MaxPacketSize <= 0) {
			m_networkConfig->MaxPacketSize = MAX_PACKET_SIZE;
		}


		NET_ERROR_CODE errorCode;

		if (m_networkConfig->IsListenerAndConnector == false)
		{
			errorCode = SFEngine::GetInstance()->Intialize(m_pLogicEntry,
				new SFPacketProtocol<SFCGSFPacketProtocol>(m_networkConfig->MaxBufferSize,
				m_networkConfig->MaxPacketSize, CGSF_PACKET_OPTION_NONE),
				m_pDispatcher);
		}
		else
		{
			if (CheckingUniqueProtocolID(m_networkConfig->ProtocolID) == false)
			{
				return NET_ERROR_CODE_N::ENGINE_INIT_DUPLICATION_PROTOCOL_ID;
			}

			errorCode = SFEngine::GetInstance()->Intialize(m_pLogicEntry,
							nullptr,
							m_pDispatcher);

			auto packetProtocol = new SFPacketProtocol<SFCGSFPacketProtocol>(m_networkConfig->MaxBufferSize,
														m_networkConfig->MaxPacketSize,
														CGSF_PACKET_OPTION_NONE);

			if (SFEngine::GetInstance()->AddPacketProtocol(m_networkConfig->ProtocolID, packetProtocol) == false)
			{
				return NET_ERROR_CODE_N::ENGINE_INIT_ADD_PACKET_PROTOCOL_FAIL;
			}

			auto listenerId = SFEngine::GetInstance()->AddListener(nullptr, m_networkConfig->Port, m_networkConfig->ProtocolID);
			LOG(INFO) << "CgsfNET64::Init. listenerId: " << listenerId;
			google::FlushLogFiles(google::GLOG_INFO);
		}
		

		if (errorCode != NET_ERROR_CODE::SUCCESS)
		{
			return (NET_ERROR_CODE_N)errorCode;
		}

		return NET_ERROR_CODE_N::SUCCESS;
	}

	

	bool CgsfNET64::Start()
	{
		bool isResult = false;

		if (m_networkConfig->IsListenerAndConnector == false)
		{
			isResult = SFEngine::GetInstance()->Start();
		}
		else
		{
			isResult = SFEngine::GetInstance()->Activate();
		}

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

	void CgsfNET64::InnerPacket(SFNETPacket^ packet)
	{
		m_packetQueue->Enqueue(packet);
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


	NET_ERROR_CODE_N CgsfNET64::RegistConnectInfo(RemoteServerConnectInfo^ connectInfo)
	{		
		System::String^ serverIP = connectInfo->IP;
		System::String^ description = connectInfo->Description;
		
		_ConnectorInfo info;
		info.szIP = msclr::interop::marshal_as<std::wstring>(serverIP);
		info.port = connectInfo->Port;
		info.connectorId = connectInfo->ConnectID;
		info.szDesc = msclr::interop::marshal_as<std::wstring>(description);
		
		
		if (CheckingUniqueProtocolID(connectInfo->ProtocolID) == false)
		{
			return NET_ERROR_CODE_N::ENGINE_INIT_DUPLICATION_PROTOCOL_ID;
		}


		auto packetProtocol = new SFPacketProtocol<SFCGSFPacketProtocol>(connectInfo->MaxBufferSize,
																		connectInfo->MaxPacketSize,
																		CGSF_PACKET_OPTION_NONE);
		
		SFEngine::GetInstance()->AddPacketProtocol(connectInfo->ProtocolID, packetProtocol);
		
		SFEngine::GetInstance()->GetServerConnectionManager()->AddConnectInfo(info);

		auto result = m_pLogicEntry->AddConnectorCallback(info.connectorId, m_pServerConnectCallback, connectInfo->ProtocolID);
		if (result == false)
		{
			NET_ERROR_CODE_N::SERVER_CONNECT_REGIST_DUPLICATION_CONNECT_ID;
		}

		m_RemoteServerConnectInfoList->Add(connectInfo);

		return NET_ERROR_CODE_N::SUCCESS;
	}

	bool CgsfNET64::SetupServerReconnectSys()
	{
		if (SFEngine::GetInstance()->SetupServerReconnectSys() == false)
		{
			return false;
		}

		return true;
	}
	
	bool CgsfNET64::CheckingUniqueProtocolID(int protocolID)
	{
		if (m_UseProtocolIDList->Contains(protocolID))
		{
			return false;
		}

		m_UseProtocolIDList->Add(protocolID);
		return true;
	}
}