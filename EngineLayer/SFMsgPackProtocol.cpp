#include "stdafx.h"
#include "SFMsgPackProtocol.h"
#include <EngineInterface/ISession.h>
#include "SFProtocol.h"
#include "SFMsgPackPacket.h"
#include "SFEngine.h"
#include "glog/logging.h"

SFMsgPackProtocol::SFMsgPackProtocol()
{

}

SFMsgPackProtocol::~SFMsgPackProtocol()
{
	if (m_pPacketIOBuffer)
		delete m_pPacketIOBuffer;

	m_pPacketIOBuffer = NULL;
}

bool SFMsgPackProtocol::Initialize(int ioBufferSize, USHORT packetSize)
{
	m_pPacketIOBuffer = new SFPacketIOBuffer();
	m_pPacketIOBuffer->AllocIOBuf(ioBufferSize);

	SFPacket::SetMaxPacketSize(packetSize);

	return true;
}

bool SFMsgPackProtocol::Reset()
{
	m_pPacketIOBuffer->InitIOBuf();

	return true;
}

bool SFMsgPackProtocol::AddTransferredData(char* pBuffer, DWORD dwTransferred)
{
	m_pPacketIOBuffer->AppendData(pBuffer, dwTransferred);

	return true;
}

BasePacket* SFMsgPackProtocol::GetPacket(int& errorCode)
{
	errorCode = PACKETIO_ERROR_NONE;

	SFMsgPackPacket* pPacket = (SFMsgPackPacket*)CreatePacket();

	pPacket->GetData().reserve_buffer(pPacket->GetHeader()->dataSize);

	if (FALSE == m_pPacketIOBuffer->GetPacket(*pPacket->GetHeader(), (char*)pPacket->GetData().buffer(), errorCode))
	{
		DisposePacket(pPacket);
		return NULL;
	}

	pPacket->GetData().buffer_consumed(pPacket->GetHeader()->dataSize);


	if (false == pPacket->Decode(pPacket->GetData().buffer(), pPacket->GetHeader()->dataSize))
	{
		//ErrorCode = SFProtocol::eIncompletePacket;
		DisposePacket(pPacket);
		return NULL;
	}

	pPacket->SetPacketID(pPacket->GetHeader()->packetID);

	return pPacket;
}

bool SFMsgPackProtocol::DisposePacket(BasePacket* pPacket)
{
	delete pPacket;
	return true;
}

BasePacket* SFMsgPackProtocol::CreatePacket()
{
	return new SFMsgPackPacket();
}

bool SFMsgPackProtocol::SendRequest(BasePacket* pPacket)
{
	SFMsgPackPacket* pMsgPackPacket = (SFMsgPackPacket*)pPacket;

	const int bufferSize = 8192;
	char buffer[bufferSize] = { 0, };

	//////////////////////////////////////////////////
	//header copy
	//////////////////////////////////////////////////
	memcpy(buffer, pMsgPackPacket->GetHeader(), sizeof(SFPacketHeader));
	int nonParsedSize = pMsgPackPacket->GetData().nonparsed_size();

	if (bufferSize < nonParsedSize + sizeof(SFPacketHeader))
	{
		LOG(WARNING) << "packet date is bigger than buffersize";
		return false;
	}

	memcpy(buffer + sizeof(SFPacketHeader), pMsgPackPacket->GetData().nonparsed_buffer(), nonParsedSize);

	//데이터 사이즈를 헤더에 쓴다.
	//12바이트 헤더의 마지막 2바이트에 기록
	*((unsigned short*)buffer + 5) = nonParsedSize;

	SFEngine::GetInstance()->SendInternal(pMsgPackPacket->GetOwnerSerial(), buffer, sizeof(SFPacketHeader)+nonParsedSize);

	return true;
}