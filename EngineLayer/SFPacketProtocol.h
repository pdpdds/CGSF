#pragma once
#include <EngineInterface/IPacketProtocol.h>
#include "SFConstant.h"
#include "SFChecksum.h"
#include "SFPacketIOBuffer.h"
#include "SFEncryption.h"
#include "SFEncryptionXOR.h"

#include "Macro.h"

class BasePacket;

void SendDummy(BasePacket* pPacket);

template <typename T>
class SFPacketProtocol : public IPacketProtocol
{
public:
	SFPacketProtocol(){}
	virtual ~SFPacketProtocol(void){}

	bool OnReceive(int serial, char* pBuffer, unsigned int dwTransferred) override;
	virtual bool DisposePacket(BasePacket* pPacket) override;
	virtual bool SendRequest(BasePacket* pPacket) override;	
	
	//virtual BasePacket* CreatePacket() override;
	virtual bool GetPacketData(BasePacket* pPacket, char* buffer, const int BufferSize, unsigned int& writtenSize) override;


	virtual IPacketProtocol* Clone(){return new SFPacketProtocol<T>();}

private:
	T m_Analyzer;
};
/*
template <typename T>
BasePacket* SFPacketProtocol<T>::CreatePacket()
{
	return m_Analyzer.CreatePacket();
}*/

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
	BOOL bRet = m_Analyzer.AddTransferredData(pBuffer, dwTransferred);

	if(FALSE == bRet)
	{
		SFASSERT(0);
		return FALSE;
	}

	int iErrorCode = PACKETIO_ERROR_NONE;
	
	do
	{
		BasePacket* pPacket = m_Analyzer.GetPacket(iErrorCode);
		
		if(NULL == pPacket)
			break;

		pPacket->SetPacketType(SFPACKET_DATA);
		pPacket->SetOwnerSerial(Serial);
		
		SendDummy(pPacket);
	}
	while(TRUE);
	
	if(iErrorCode != PACKETIO_ERROR_NONE)
	{
		return false;
	}

	return true;
}
/*
template <typename T>
bool SFPacketProtocol<T>::SendRequest(ISession* pSession, BasePacket* pPacket)
{
	BOOL bRet = m_Analyzer.SendRequest(pSession, pPacket);

	return true;
}*/
