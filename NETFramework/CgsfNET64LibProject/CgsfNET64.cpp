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
		
		if (config->MaxBufferSize <= 0) {
			config->MaxBufferSize = MAX_IO_SIZE;
		}

		if (config->MaxPacketSize <= 0) {
			config->MaxPacketSize = MAX_PACKET_SIZE;
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
		
	NET_ERROR_CODE_N CgsfNET64::Init(NetworkConfig^ config,
								Generic::List<RemoteServerConnectInfo^>^ connectInfoList,
								Generic::List<MultiListenNetworkInfo^>^ listneInfoList)
	{
		SetNetworkConfig(config);

		m_packetQueue = gcnew ConcurrencyPacketQueue();

		m_pLogicEntry = new ServerLogicEntry();
		m_pLogicEntry->m_refPacketQueue = m_packetQueue;

		m_pDispatcher = new SFNETDispatcher();
		m_pDispatcher->Init(m_networkConfig->ThreadCount);


		m_pServerConnectCallback = new ServerConnectCallback;
		m_pServerConnectCallback->m_refPacketQueue = m_packetQueue;
				

		NET_ERROR_CODE errorCode = SFEngine::GetInstance()->Intialize(m_pLogicEntry,
														m_pDispatcher);
		if (errorCode != NET_ERROR_CODE::SUCCESS)
		{
			return (NET_ERROR_CODE_N)errorCode;
		}

		if (config->Port > 0)
		{
			if (AddPacketProtocol(config->ProtocolID, m_networkConfig->MaxBufferSize, m_networkConfig->MaxPacketSize, config->ProtocolOption) == false)
			{
				LOG(ERROR) << "CgsfNET64::Init. Basic AddPacketProtocol. ProtocolID: " << config->ProtocolID;
				google::FlushLogFiles(google::GLOG_ERROR);
				return NET_ERROR_CODE_N::ENGINE_INIT_ADD_PACKET_PROTOCOL_FAIL;
			}
		}


		// Connector 등록
		if (connectInfoList != nullptr && connectInfoList->Count > 0)
		{
			LOG(INFO) << "CgsfNET64::Init. Regist Connector";
			google::FlushLogFiles(google::GLOG_INFO);

			for (int i = 0; i < connectInfoList->Count; ++i)
			{
				auto result = RegistConnectInfo(connectInfoList[i]);

				if (result != NET_ERROR_CODE_N::SUCCESS)
				{
					return result;
				}
			}

			m_RemoteServerConnectInfoList = connectInfoList;
		}


		// Listener 등록
		if (listneInfoList != nullptr && listneInfoList->Count > 0)
		{
			LOG(INFO) << "CgsfNET64::Init. Regist Listener";
			google::FlushLogFiles(google::GLOG_INFO);

			for (int i = 0; i < listneInfoList->Count; ++i)
			{
				if (CheckingUniqueProtocolID(listneInfoList[i]->ProtocolID) == false)
				{
					return NET_ERROR_CODE_N::ENGINE_INIT_DUPLICATION_PROTOCOL_ID;
				}

				if (AddPacketProtocol(listneInfoList[i]->ProtocolID, listneInfoList[i]->MaxBufferSize, listneInfoList[i]->MaxPacketSize, listneInfoList[i]->ProtocolOption) == false)
				{
					LOG(ERROR) << "CgsfNET64::Init. Multi Listener AddPacketProtocol. ProtocolID: " << listneInfoList[i]->ProtocolID;
					google::FlushLogFiles(google::GLOG_ERROR);
					return NET_ERROR_CODE_N::ENGINE_INIT_ADD_PACKET_PROTOCOL_FAIL;
				}
								
				listneInfoList[i]->ListenID = SFEngine::GetInstance()->AddListener(nullptr, listneInfoList[i]->Port, listneInfoList[i]->ProtocolID);
				LOG(INFO) << "CgsfNET64::Init. listenerId: " << listneInfoList[i]->ListenID;
				google::FlushLogFiles(google::GLOG_INFO);
			}

			m_ListneInfoList = listneInfoList;
		}
		
		return NET_ERROR_CODE_N::SUCCESS;
	}


	bool CgsfNET64::Start(int protocolID)
	{
		LOG(INFO) << "CgsfNET64::Start";
		bool isResult = SFEngine::GetInstance()->Start(protocolID);
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
		if (MIN_SERVER_CONNECTOR_ID > connectInfo->ConnectID)
		{
			return NET_ERROR_CODE_N::SERVER_CONNECTOR_INVALID_CONNECT_ID;
		}

		if (CheckingUniqueProtocolID(connectInfo->ProtocolID) == false)
		{
			return NET_ERROR_CODE_N::ENGINE_INIT_DUPLICATION_PROTOCOL_ID;
		}


		System::String^ serverIP = connectInfo->IP;
		System::String^ description = connectInfo->Description;
		
		_ConnectorInfo info;
		info.szIP = msclr::interop::marshal_as<std::wstring>(serverIP);
		info.port = connectInfo->Port;
		info.connectorId = connectInfo->ConnectID;
		info.szDesc = msclr::interop::marshal_as<std::wstring>(description);
		
		
		auto packetProtocol = new SFPacketProtocol<SFCGSFPacketProtocol>(connectInfo->MaxBufferSize,
																		connectInfo->MaxPacketSize,
																		connectInfo->ProtocolOption);
		
		SFEngine::GetInstance()->AddPacketProtocol(connectInfo->ProtocolID, packetProtocol);
		
		SFEngine::GetInstance()->GetServerConnectionManager()->AddConnectorInfo(info);

		auto result = m_pLogicEntry->AddConnectorCallback(info.connectorId, m_pServerConnectCallback, connectInfo->ProtocolID);
		if (result == false)
		{
			LOG(ERROR) << "CgsfNET64::RegistConnectInfo. ConnectorID: " << info.connectorId;
			google::FlushLogFiles(google::GLOG_ERROR);
			NET_ERROR_CODE_N::SERVER_CONNECTOR_REGIST_DUPLICATION_CONNECT_ID;
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
			LOG(ERROR) << "CgsfNET64::CheckingUniqueProtocolID. Duplication ProtocolID: " << protocolID;
			google::FlushLogFiles(google::GLOG_ERROR);
			return false;
		}

		m_UseProtocolIDList->Add(protocolID);
		return true;
	}

	bool CgsfNET64::AddPacketProtocol(int protocolID, int maxBufferSize, int maxPacketSize, int option)
	{
		auto packetProtocol = new SFPacketProtocol<SFCGSFPacketProtocol>(maxPacketSize, maxPacketSize, option);

		if (SFEngine::GetInstance()->AddPacketProtocol(protocolID, packetProtocol) == false)
		{
			return false;
		}

		return true;
	}
}