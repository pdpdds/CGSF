#include "stdafx.h"
#include "SFEchoProtocol.h"
#include <EngineInterface/ISession.h>
#include "SFEchoPacket.h"
#include "SFProtocol.h"
#include "SFEngine.h"

#define nReceiveBufferSize 1024 * 8/*8192*/
#define nSendBufferSize 1024 * 8/*8192*/
SFEchoProtocol::SFEchoProtocol(void)
	: m_Buffer(nReceiveBufferSize)
{
}


SFEchoProtocol::~SFEchoProtocol(void)
{
	
}

bool SFEchoProtocol::Initialize(int ioBufferSize, unsigned short packetSize, int packetOption)
{	
	m_ioSize = ioBufferSize;
	m_packetSize = packetSize;
	m_packetOption = packetOption;

	return true;
}

bool SFEchoProtocol::AddTransferredData(char* pBuffer, DWORD dwTransferred)
{
	return m_Buffer.Append(pBuffer, dwTransferred);
}

bool SFEchoProtocol::Reset()
{
	return true;
}

bool SFEchoProtocol::Encode(BasePacket* pPacket, char** ppBuffer, int& bufferSize)
{	
	SFEchoPacket* pEchoPacket = (SFEchoPacket*)pPacket;
		
	*ppBuffer = pEchoPacket->m_Buffer.GetBuffer();
	bufferSize = pEchoPacket->m_Buffer.GetDataSize();

	return true;
}

BasePacket* SFEchoProtocol::GetPacket(int& ErrorCode)
{
	ErrorCode = PACKETIO_ERROR_NONE;

	SFEchoPacket* pPacket = (SFEchoPacket*)CreatePacket();

	if (false == GetPacket(pPacket))
	{
		//ErrorCode = SFProtocol::eIncompletePacket;
		DisposePacket(pPacket);
		return NULL;
	}

	return pPacket;
}

bool SFEchoProtocol::GetPacket(SFEchoPacket* pPacket)
{
	if (m_Buffer.GetDataSize() == 0)
		return false;
	
	pPacket->m_Buffer.Append(m_Buffer.GetBuffer(), m_Buffer.GetDataSize());
	m_Buffer.Pop(m_Buffer.GetDataSize());

	return true;
}

BasePacket* SFEchoProtocol::CreatePacket()
{
	return new SFEchoPacket();
}

bool SFEchoProtocol::DisposePacket(BasePacket* pPacket)
{
	delete pPacket;
	return true;
}