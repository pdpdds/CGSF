#include "stdafx.h"
#include "SFHTTPProtocol.h"
#include "SFHTTPPacket.h"

SFHTTPProtocol::SFHTTPProtocol()
{

}


SFHTTPProtocol::~SFHTTPProtocol()
{
	if (m_pIOBuffer)
		delete m_pIOBuffer;

	m_pIOBuffer = NULL;
}


bool SFHTTPProtocol::Initialize(int ioBufferSize, unsigned short packetSize, int packetOption)
{
	m_pIOBuffer = new SFIOBuffer();
	m_pIOBuffer->AllocIOBuf(ioBufferSize);

	m_ioSize = ioBufferSize;
	m_packetSize = packetSize;
	m_packetOption = packetOption;

	return true;
}

bool SFHTTPProtocol::Reset()
{
	m_pIOBuffer->InitIOBuf();

	return true;
}

bool SFHTTPProtocol::AddTransferredData(char* pBuffer, DWORD dwTransferred)
{
	m_pIOBuffer->AppendData(pBuffer, dwTransferred);

	return true;
}

BasePacket* SFHTTPProtocol::CreatePacket()
{
	return new SFHTTPPacket();
}

bool SFHTTPProtocol::Encode(BasePacket* pPacket, char** ppBuffer, int& bufferSize)
{
	UNREFERENCED_PARAMETER(pPacket);
	UNREFERENCED_PARAMETER(ppBuffer);
	UNREFERENCED_PARAMETER(bufferSize);

	SFHTTPPacket* pHttpPacket = (SFHTTPPacket*)pPacket;

	http::BufferedResponse response;
	std::size_t size = pHttpPacket->m_szRawResponse.length();
	std::size_t used = 0;
	std::size_t pass = 0;

	try
	{
		while ((used < size) && !response.complete()) {
			used += response.feed(pHttpPacket->m_szRawResponse.c_str() + used, size - used);
		}
	}
	catch (...)
	{
		LOG(ERROR) << "HTTP PROTOCOL RESPONSE RAW DATA PROBLEM!! ";
		return false;
	}
	

	*ppBuffer = (char*)pHttpPacket->m_szRawResponse.c_str();
	bufferSize = pHttpPacket->m_szRawResponse.length();

	return true;
}

bool SFHTTPProtocol::DisposePacket(BasePacket* pPacket)
{
	delete pPacket;
	return true;
}

BasePacket* SFHTTPProtocol::GetPacket(int& errorCode)
{
	errorCode = PACKETIO_ERROR_NONE;

	SFHTTPPacket* pPacket = (SFHTTPPacket*)CreatePacket();
	
	if (m_pIOBuffer->GetUsedBufferSize() == 0)
	{		
		return false;
	}

	char recvData[16384] = { 0, };

	int getDataSize = m_pIOBuffer->GetData((char*)recvData, 16384);

	if (getDataSize == 0)
	{
		errorCode = PACKETIO_ERROR_DATA;
		return false;
	}
	
	int usedSize = 0;
	int size = getDataSize;
	char* data = recvData;

	try
	{
		while (size > 0 && pPacket->m_request.complete() == false)
		{
			std::size_t pass = pPacket->m_request.feed(recvData, size);
			usedSize += pass, data += pass, size -= pass;
		}
	}
	catch (...)
	{
//리퀘스트 요청 스트링 문제. 유저의 연결을 종료시킨다.
		errorCode = PACKETIO_ERROR_DATA;
		return false;
	}
	
	if (pPacket->m_request.complete() == false)
	{
//데이터가 완전히 도착하지 않았음
		m_pIOBuffer->AppendData(recvData, getDataSize);
		DisposePacket(pPacket);
		return NULL;
	}

//완전한 요청 패킷 다음에 불완전한 데이터가 추가로 붙어 있음
	if (size > 0)
	{
		m_pIOBuffer->AppendData(recvData + usedSize, size);
	}

	return pPacket;
}


	
