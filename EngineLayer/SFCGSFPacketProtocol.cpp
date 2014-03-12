#include "stdafx.h"
#include "SFCGSFPacketProtocol.h"
#include "SFCompressor.h"
#include <EngineInterface/ISession.h>
#include "SFEngine.h"

SFCGSFPacketProtocol::SFCGSFPacketProtocol(void)
{
	Initialize();
}

SFCGSFPacketProtocol::~SFCGSFPacketProtocol(void)
{
	if(m_pPacketIOBuffer)
		delete m_pPacketIOBuffer;

	m_pPacketIOBuffer = NULL;
}

BOOL SFCGSFPacketProtocol::Initialize()
{
	m_pPacketIOBuffer = new SFPacketIOBuffer();
	m_pPacketIOBuffer->AllocIOBuf(PACKETIO_SIZE);

	return TRUE;
}

BasePacket* SFCGSFPacketProtocol::GetPacket(int& ErrorCode)
{
	SFPacket* pPacket = PacketPoolSingleton::instance()->Alloc();
	pPacket->Initialize();

	if(FALSE == m_pPacketIOBuffer->GetPacket(pPacket, ErrorCode))
	{
		PacketPoolSingleton::instance()->Release(pPacket);
		return NULL;
	}

	if(FALSE == pPacket->Decode(ErrorCode))
	{
		PacketPoolSingleton::instance()->Release(pPacket);
		return NULL;
	}

	return pPacket;
}

BOOL SFCGSFPacketProtocol::AddTransferredData(char* pBuffer, DWORD dwTransferred)
{
	m_pPacketIOBuffer->AppendData(pBuffer, dwTransferred);

	return TRUE;
}

BOOL SFCGSFPacketProtocol::Reset()
{
	m_pPacketIOBuffer->InitIOBuf();

	return TRUE;
}

bool SFCGSFPacketProtocol::SendRequest(BasePacket* pPacket)
{
	SFPacket* pSFPacket = (SFPacket*)pPacket;
	pSFPacket->Encode();

	SFEngine::GetInstance()->SendInternal(pSFPacket->GetOwnerSerial(), (char*)pSFPacket->GetHeader(), pSFPacket->GetHeaderSize() + pSFPacket->GetDataSize());
	
	return TRUE;
}

bool SFCGSFPacketProtocol::DisposePacket(BasePacket* pPacket)
{
	SFPacket* pSFPacket = static_cast<SFPacket*>(pPacket);

	SFASSERT(pSFPacket != NULL);
	return PacketPoolSingleton::instance()->Release(pSFPacket);
}

bool SFCGSFPacketProtocol::GetPacketData(BasePacket* pPacket, char* buffer, const int BufferSize, unsigned int& writtenSize)
{
	writtenSize = 0;

	SFPacket* pSFPacket = (SFPacket*)pPacket;

	if (pSFPacket->GetDataSize() == 0)
	{
		return true;
	}

	if (pSFPacket->GetDataSize() > BufferSize)
	{
		SFASSERT(0);
		return false;
	}

	pSFPacket->Encode();

	memcpy(buffer, pSFPacket->GetDataBuffer(), pSFPacket->GetDataSize());
	writtenSize = pSFPacket->GetDataSize();

	return true;
}