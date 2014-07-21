#include "stdafx.h"
#include "SFAvroProtocol.h"
#include "stdafx.h"
#include <EngineInterface/ISession.h>
#include "SFProtocol.h"
#include "SFAvroPacketImpl.h"
#include "SFEngine.h"
#include "glog/logging.h"

SFAvroProtocol::SFAvroProtocol()
{

}

SFAvroProtocol::~SFAvroProtocol()
{
	if (m_pPacketIOBuffer)
		delete m_pPacketIOBuffer;

	m_pPacketIOBuffer = NULL;

	if (m_pBuffer)
		delete m_pBuffer;
}

bool SFAvroProtocol::Initialize(int ioBufferSize, unsigned short packetSize, int packetOption)
{
	m_pPacketIOBuffer = new SFPacketIOBuffer();
	m_pPacketIOBuffer->AllocIOBuf(ioBufferSize);

	m_ioSize = ioBufferSize;
	m_packetSize = packetSize;
	m_packetOption = packetOption;

	m_pBuffer = new char[m_packetSize];
	memset(m_pBuffer, 0, m_packetSize);

	return true;
}

bool SFAvroProtocol::Reset()
{
	m_pPacketIOBuffer->InitIOBuf();

	return true;
}

bool SFAvroProtocol::AddTransferredData(char* pBuffer, DWORD dwTransferred)
{
	m_pPacketIOBuffer->AppendData(pBuffer, dwTransferred);

	return true;
}

BasePacket* SFAvroProtocol::GetPacket(int& errorCode)
{
	errorCode = PACKETIO_ERROR_NONE;

	SFPacketHeader header;

	if (false == m_pPacketIOBuffer->GetHeader(header, errorCode))
		return NULL;

	BasePacket* pPacket = CreateIncomingPacketFromPacketId(header.packetID);

	if (pPacket == NULL)
		return NULL;

	SFAvroPacketImpl* pAvroPacket = (SFAvroPacketImpl*)pPacket;

	memcpy(pAvroPacket->GetHeader(), &header, sizeof(SFPacketHeader));

	char dataBuffer[8096] = { 0, };

	if (false == m_pPacketIOBuffer->GetPacket(header, dataBuffer, m_packetSize, errorCode))
	{
		DisposePacket(pPacket);
		return NULL;
	}

	if (false == pPacket->Decode(dataBuffer, header.dataSize))
	{
		errorCode = PACKETIO_ERROR_DATA;
		DisposePacket(pPacket);
		return NULL;
	}

	pPacket->SetPacketID(header.packetID);

	return pPacket;
}

bool SFAvroProtocol::DisposePacket(BasePacket* pPacket)
{
	delete pPacket;
	return true;
}

/*
BasePacket* SFAvroProtocol::CreatePacket()
{
	return new SFAvroPacket();
}*/

bool SFAvroProtocol::Encode(BasePacket* pPacket, char** ppBuffer, int& bufferSize)
{
	SFAvroPacketImpl* pAvroPacket = (SFAvroPacketImpl*)pPacket;

	//////////////////////////////////////////////////
	//header copy
	//////////////////////////////////////////////////
	memcpy(m_pBuffer, pAvroPacket->GetHeader(), sizeof(SFPacketHeader));
	
	/*std::auto_ptr<avro::OutputStream> out = avro::memoryOutputStream();
	avro::EncoderPtr e = avro::binaryEncoder();
	e->init(*out);

	bool bResult = pAvroPacket->Encode(e);*/
	
	unsigned int writtenSize = pAvroPacket->GetBufferSize();
	memcpy(m_pBuffer + sizeof(SFPacketHeader), pAvroPacket->GetBuffer(), writtenSize);

	//데이터 사이즈를 헤더에 쓴다.
	//12바이트 헤더의 마지막 2바이트에 기록
	*((unsigned short*)m_pBuffer + 5) = writtenSize;

	*ppBuffer = m_pBuffer;
	bufferSize = sizeof(SFPacketHeader)+writtenSize;

	return true;
}