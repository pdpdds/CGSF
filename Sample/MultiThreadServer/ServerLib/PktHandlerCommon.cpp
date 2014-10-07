#include "stdafx.h"
#include "PktHandlerCommon.h"
#include "ServerErrorCode.h"
#include "WorkerPacket.h"

namespace ServerLib {
	
	PktHandlerCommon::PktHandlerCommon()
	{

	}

	PktHandlerCommon::~PktHandlerCommon()
	{
	}


	short PktHandlerCommon::SystemOnConnect(const WorkerPacket& requestPacket)
	{
		printf("SystemOnConnect. SessionID:%d \n", requestPacket.SessionID());
		return SERVER_ERROR_CODE::NONE;
	}

	short PktHandlerCommon::SystemOnDisconnect(const WorkerPacket& requestPacket)
	{
		printf("SystemOnDisConnect. SessionID:%d \n", requestPacket.SessionID());
		return SERVER_ERROR_CODE::NONE;
	}

	short PktHandlerCommon::RequestLoging(const WorkerPacket& requestPacket)
	{
		UNREFERENCED_PARAMETER(requestPacket);

		return SERVER_ERROR_CODE::NONE;
	}
}