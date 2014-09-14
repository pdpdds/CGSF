#include "stdafx.h"
#include "SFJsonProtocol.h"
#include <Json/JsonNode.h>
#include <EngineInterface/ISession.h>
#include "SFJsonPacket.h"
#include "SFProtocol.h"
#include "SFEngine.h"

SFJsonProtocol::SFJsonProtocol(void)
: m_pBuffer(NULL)
{
}


SFJsonProtocol::~SFJsonProtocol(void)
{
	if (m_pBuffer)
		delete m_pBuffer;
}

bool SFJsonProtocol::Initialize(int ioBufferSize, unsigned short packetSize, int packetOption)
{
	m_builder.PrepareBuffer(ioBufferSize);

	m_ioSize = ioBufferSize;
	m_packetSize = packetSize;
	m_packetOption = packetOption;

	m_pBuffer = new char[m_packetSize];
	memset(m_pBuffer, 0, m_packetSize);

	return true;
}

bool SFJsonProtocol::AddTransferredData(char* pBuffer, DWORD dwTransferred)
{
	m_builder.PushBuffer(pBuffer, dwTransferred);

	return true;
}

bool SFJsonProtocol::Reset()
{
	return true;
}

bool SFJsonProtocol::Encode(BasePacket* pPacket, char** ppBuffer, int& bufferSize)
{
	SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;
	JsonObjectNode& ObjectNode = pJsonPacket->GetData();

	memset(m_pBuffer, 0, m_packetSize);

//////////////////////////////////////////////////
//header copy
//////////////////////////////////////////////////
	memcpy(m_pBuffer, pJsonPacket->GetHeader(), sizeof(SFPacketHeader));
	unsigned int writtenSize = JsonBuilder::MakeBuffer(ObjectNode, m_pBuffer + sizeof(SFPacketHeader), m_packetSize - sizeof(SFPacketHeader));
	*((unsigned short*)m_pBuffer + 5) = writtenSize;

	*ppBuffer = m_pBuffer;
	bufferSize = sizeof(SFPacketHeader)+writtenSize;

	pJsonPacket->Encode(m_packetSize, m_packetOption);
	
	return true;
}

bool SFJsonProtocol::GetCompleteNode(SFJsonPacket* pPacket)
{
	if (m_builder.GetUsedBufferSize() < sizeof(SFPacketHeader))
		return false;

	memcpy(pPacket->GetHeader(), m_builder.GetBuffer(), sizeof(SFPacketHeader));

	SFPacketHeader* pHeader = pPacket->GetHeader();
	pPacket->SetPacketID(pHeader->packetID);

	if (pHeader->dataSize > m_packetSize - sizeof(SFPacketHeader))
		return false;

	if (m_builder.GetUsedBufferSize() < pHeader->dataSize + sizeof(SFPacketHeader))
		return false;

	m_builder.IncReadOffset(sizeof(SFPacketHeader));
	
	return m_builder.PopCompleteNode(pPacket->GetData(), pHeader->dataSize);
}

BasePacket* SFJsonProtocol::GetPacket(int& ErrorCode)
{
	ErrorCode = PACKETIO_ERROR_NONE;

	SFJsonPacket* pPacket = (SFJsonPacket*)CreatePacket();

	if (false == GetCompleteNode(pPacket))
	{
		//ErrorCode = SFProtocol::eIncompletePacket;
		DisposePacket(pPacket);
		return NULL;
	}
	
	return pPacket;
}

BasePacket* SFJsonProtocol::CreatePacket()
{
	return new SFJsonPacket();
}

bool SFJsonProtocol::DisposePacket(BasePacket* pPacket)
{
	delete pPacket;
	return true;
}