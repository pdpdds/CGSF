#include "stdafx.h"
#include "SFCGSFPacketProtocol.h"
#include "SFCompressor.h"
#include <EngineInterface/ISession.h>

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

bool SFCGSFPacketProtocol::SendRequest(ISession* pSession, BasePacket* pPacket)
{
	SFPacket* pSFPacket = (SFPacket*)pPacket;
	pSFPacket->Encode();

	pSession->SendInternal((char*)pSFPacket->GetHeader(), pSFPacket->GetHeaderSize() + pSFPacket->GetDataSize());
	
	return TRUE;
}