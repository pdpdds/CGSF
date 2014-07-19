#include "stdafx.h"
#include "SFCGSFPacketProtocol.h"
#include "SFCompressor.h"
#include <EngineInterface/ISession.h>
#include "SFEngine.h"

SFCGSFPacketProtocol::SFCGSFPacketProtocol(void)
{
	
}

SFCGSFPacketProtocol::~SFCGSFPacketProtocol(void)
{
	if(m_pPacketIOBuffer)
		delete m_pPacketIOBuffer;

	m_pPacketIOBuffer = NULL;
}

bool SFCGSFPacketProtocol::Initialize(int ioBufferSize, unsigned short packetSize, int packetOption)
{
	m_pPacketIOBuffer = new SFPacketIOBuffer();
	m_pPacketIOBuffer->AllocIOBuf(ioBufferSize);

	m_ioSize = ioBufferSize;
	m_packetSize = packetSize;
	m_packetOption = packetOption;

	return true;
}

BasePacket* SFCGSFPacketProtocol::GetPacket(int& errorCode)
{
	SFPacket* pPacket = PacketPoolSingleton::instance()->Alloc();
	pPacket->Initialize();

	if (FALSE == m_pPacketIOBuffer->GetPacket(*pPacket->GetHeader(), (char*)pPacket->GetData(), m_packetSize, errorCode))
	{
		PacketPoolSingleton::instance()->Release(pPacket);
		return NULL;
	}

	if (FALSE == pPacket->Decode(m_packetSize, errorCode))
	{
		PacketPoolSingleton::instance()->Release(pPacket);
		return NULL;
	}

	return pPacket;
}

bool SFCGSFPacketProtocol::AddTransferredData(char* pBuffer, DWORD dwTransferred)
{
	m_pPacketIOBuffer->AppendData(pBuffer, dwTransferred);

	return true;
}

bool SFCGSFPacketProtocol::Reset()
{
	m_pPacketIOBuffer->InitIOBuf();

	return true;
}

bool SFCGSFPacketProtocol::SendRequest(BasePacket* pPacket)
{
	SFPacket* pSFPacket = (SFPacket*)pPacket;
	pSFPacket->Encode(m_packetSize, m_packetOption);

	SFEngine::GetInstance()->SendInternal(pSFPacket->GetSerial(), (char*)pSFPacket->GetHeader(), pSFPacket->GetPacketSize());
	
	return TRUE;
}

bool SFCGSFPacketProtocol::DisposePacket(BasePacket* pPacket)
{
	SFPacket* pSFPacket = static_cast<SFPacket*>(pPacket);

	SFASSERT(pSFPacket != NULL);
	return PacketPoolSingleton::instance()->Release(pSFPacket);
}