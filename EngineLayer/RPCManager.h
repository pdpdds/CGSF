#pragma once
#include "stdafx.h"
#include "service_define_template.h"
#include "serviceDef.h"
#include "IRPCInterface.h"

template<class Service>
class RPCManager : public IRPCInterface

{
public:
	RPCManager(Service & service)
		: _service(service) {}

	virtual ~RPCManager(){}

	virtual void ProcessRPCService(SFPacket* pSFPacket)
	{
		CPstream oa;
		CPstream iar((char*)pSFPacket->GetData(), pSFPacket->GetDataSize());

		// De-Serialize service name
		std::string svrName;
		iar >> svrName;
		int ret;

		if (svrName != _serviceDef.getServiceName())
		{
			ret = RPC_OK_INVALID_SERVICE_NAME;
			oa << ret;
			return SendRPCServiceResult(pSFPacket, oa);
		}

		// De-Serialize method id
		int svrId;
		iar >> svrId;

		IServiceMethod<Service> * method = _serviceDef.getMethodById(svrId);
		if (method == NULL)
		{
			ret = RPC_OK_INVALID_METHOD_ID;
			oa << ret;
			return SendRPCServiceResult(pSFPacket, oa);
		}

		ret = RPC_OK;
		oa << ret;
		oa << svrId;
		method->dispatch(_service, iar, oa);
		return SendRPCServiceResult(pSFPacket, oa);
	}

	void SendRPCServiceResult(SFPacket* pSFPacket, CPstream& oa)
	{
		SFPacket packet(SFPACKET_RPC_PACKET);
		//packet.Write((const BYTE*)pSFPacket->GetHeader(), sizeof(SFPacketHeader));
		packet.SetOwnerSerial(pSFPacket->GetOwnerSerial());
		packet.Write((const BYTE*)oa.buf(), oa.bufSize());


		
		SFEngine::GetInstance()->SendRequest(&packet);
	}

private:
	ServiceDefinition<Service> _serviceDef;
	Service & _service;
};

