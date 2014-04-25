#pragma once
// ============================================================================
// SFPacketProtocol 클래스
// author : pdpdds
// desc : 유저가 전송한 네트워크 데이터를 저장하고 패킷의 생성 및 처분, 그리고 유저에게 패킷을 전송하는 역할을 담당하는 템플릿 클래스입니다.
// 실제 데이터의 처리는 템플릿으로 제공된 클래스가 담당합니다.
// ============================================================================

#include <EngineInterface/IPacketProtocol.h>
#include "SFConstant.h"
#include "SFChecksum.h"
#include "SFPacketIOBuffer.h"
#include "SFEncryption.h"
#include "SFEncryptionXOR.h"
#include "SFBaseProtocol.h"

#include "Macro.h"

class BasePacket;

void SendLogicLayer(BasePacket* pPacket);

template <typename T>
class SFPacketProtocol : public IPacketProtocol, public SFBaseProtocol 
{
public:
	SFPacketProtocol();
	SFPacketProtocol(int bufferIOSize, USHORT packetDataSize);
	virtual ~SFPacketProtocol(void){}

	// ----------------------------------------------------------------
	//  Name:           OnReceive
	//  Description:    유저가 전송한 데이터를 처리한다. 	
	// ----------------------------------------------------------------
	bool OnReceive(int serial, char* pBuffer, unsigned int dwTransferred) override;
	
	// ----------------------------------------------------------------
	//  Name:           SendRequest
	//  Description:    타겟에게 패킷을 전송한다.
	// ----------------------------------------------------------------
	virtual bool SendRequest(BasePacket* pPacket) override;	

	// ----------------------------------------------------------------
	//  Name:           DisposePacket
	//  Description:    쓸모가 없어진 패킷을 수거한다.
	// ----------------------------------------------------------------
	virtual bool DisposePacket(BasePacket* pPacket) override;

	// ----------------------------------------------------------------
	//  Name:           Clone
	//  Description:    패킷프로토콜 클래스 객체를 생성한다.
	// ----------------------------------------------------------------
	virtual IPacketProtocol* Clone(){ return new SFPacketProtocol<T>(); }
	
	// ----------------------------------------------------------------
	//  Name:           GetPacketData
	//  Description:    로직쓰레드가 패킷 인코딩을 하지 않도록 패킷의 데이터만 뽑아내는 메소드이다.
	//					현재 개발중인 메소드임
	// ----------------------------------------------------------------
	virtual bool GetPacketData(BasePacket* pPacket, char* buffer, const int BufferSize, unsigned int& writtenSize) override;

	//virtual BasePacket* CreatePacket() override;

private:

	// ----------------------------------------------------------------
	//  Name:           템플릿 클래스
	//  Description:    실제 패킷 프로토콜을 처리하는 클래스
	//					AddTransferredData, DisposePacket, SendRequest GetPacket 메소드를 반드시 구현해야 된다.
	//					GetPacketData 메소드는 빈 메소드로 만들어 두도록 한다.
	// ----------------------------------------------------------------
	T m_Analyzer;	
};

template <typename T>
SFPacketProtocol<T>::SFPacketProtocol()
{
	m_Analyzer.Initialize(m_ioSize, m_packetSize);
}

template <typename T>
SFPacketProtocol<T>::SFPacketProtocol(int bufferIOSize, USHORT packetDataSize)
{
	if (bufferIOSize > MAX_IO_SIZE)
		bufferIOSize = MAX_IO_SIZE;

	if (packetDataSize > MAX_PACKET_SIZE)
		packetDataSize = MAX_PACKET_SIZE;

	if (packetDataSize > bufferIOSize)
		packetDataSize = bufferIOSize;

	m_ioSize = bufferIOSize;
	m_packetSize = packetDataSize;

	m_Analyzer.Initialize(m_ioSize, m_packetSize);
}

template <typename T>
bool SFPacketProtocol<T>::DisposePacket(BasePacket* pPacket)
{
	return m_Analyzer.DisposePacket(pPacket);
}

template <typename T>
bool SFPacketProtocol<T>::GetPacketData(BasePacket* pPacket, char* buffer, const int BufferSize, unsigned int& writtenSize)
{
	return m_Analyzer.GetPacketData(pPacket, buffer, BufferSize, writtenSize);
}

template <typename T>
bool SFPacketProtocol<T>::SendRequest(BasePacket* pPacket)
{
	return m_Analyzer.SendRequest(pPacket);
}

template <typename T>
bool SFPacketProtocol<T>::OnReceive(int Serial, char* pBuffer, unsigned int dwTransferred)
{
	bool bRet = m_Analyzer.AddTransferredData(pBuffer, dwTransferred);

	if(false == bRet)
	{
		SFASSERT(0);
		return false;
	}

	int iErrorCode = PACKETIO_ERROR_NONE;
	
	do
	{
		BasePacket* pPacket = m_Analyzer.GetPacket(iErrorCode);
		
		if(NULL == pPacket)
			break;

		pPacket->SetPacketType(SFPACKET_DATA);
		pPacket->SetOwnerSerial(Serial);
		
		SendLogicLayer(pPacket);
	}
	while(true);
	
	if(iErrorCode != PACKETIO_ERROR_NONE)
	{
		return false;
	}

	return true;
}

/*
template <typename T>
BasePacket* SFPacketProtocol<T>::CreatePacket()
{
return m_Analyzer.CreatePacket();
}*/