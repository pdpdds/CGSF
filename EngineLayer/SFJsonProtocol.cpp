#include "stdafx.h"
#include "SFJsonProtocol.h"
#include <Json/JsonNode.h>
#include <EngineInterface/ISession.h>
#include "SFJsonPacket.h"
#include "SFProtocol.h"
#include "SFEngine.h"

SFJsonProtocol::SFJsonProtocol(void)
{
}


SFJsonProtocol::~SFJsonProtocol(void)
{
}

bool SFJsonProtocol::Initialize(int ioBufferSize, USHORT packetSize)
{
	SFPacket::SetMaxPacketSize(packetSize);
	m_builder.PrepareBuffer(ioBufferSize);
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

bool SFJsonProtocol::SendRequest(BasePacket* pPacket)
{
	SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;
	JsonObjectNode& ObjectNode = pJsonPacket->GetData();

	const int BufferSize = SFPacket::GetMaxPacketSize();
	char* buffer = new char[BufferSize];
	memset(buffer, 0, BufferSize);

//////////////////////////////////////////////////
//header copy
//////////////////////////////////////////////////
	memcpy(buffer, pJsonPacket->GetHeader(), sizeof(SFPacketHeader));
	unsigned int writtenSize = JsonBuilder::MakeBuffer(ObjectNode, buffer + sizeof(SFPacketHeader), BufferSize - sizeof(SFPacketHeader));
	*((unsigned short*)buffer + 5) = writtenSize;
	
	SFEngine::GetInstance()->SendInternal(pJsonPacket->GetOwnerSerial(), buffer, sizeof(SFPacketHeader) + writtenSize);

	delete buffer;

	return true;
}

bool SFJsonProtocol::GetCompleteNode(SFJsonPacket* pPacket)
{
	if (m_builder.GetUsedBufferSize() < sizeof(SFPacketHeader))
		return false;

	memcpy(pPacket->GetHeader(), m_builder.GetBuffer(), sizeof(SFPacketHeader));

	SFPacketHeader* pHeader = pPacket->GetHeader();
	pPacket->SetPacketID(pHeader->packetID);

	if (pHeader->dataSize > SFPacket::GetMaxPacketSize() - sizeof(SFPacketHeader))
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

bool SFJsonProtocol::GetPacketData(BasePacket* pPacket, char* buffer, const int BufferSize, unsigned int& writtenSize)
{
	SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;
	JsonObjectNode& ObjectNode = pJsonPacket->GetData();

	memcpy(buffer, pJsonPacket->GetHeader(), sizeof(SFPacketHeader));
	
	writtenSize = JsonBuilder::MakeBuffer(ObjectNode, buffer + sizeof(SFPacketHeader), BufferSize);

	if(writtenSize == 0)
		return false;

	writtenSize += sizeof(SFPacketHeader);

	return true;
}
