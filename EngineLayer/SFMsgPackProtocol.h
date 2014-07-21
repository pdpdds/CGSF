#pragma once
// ============================================================================
// SFMsgPackProtocol 클래스
// author : pdpdds
// desc : MessagePack 프로토콜을 처리하는 클래스
// SFMsgPackPacket 패킷 객체의 생성을 담당하며 패킹된 MessagePack 데이터를 네트워크로 전송한다.
// 프로토콜 클래스는 연결된 소켓에 1:1 대응을 한다.
// ============================================================================
#include "SFBasePacketProtocol.h"

#pragma warning( push )
#pragma warning (disable : 4244) 
#pragma warning (disable : 4267)
#pragma warning (disable : 4100)
#include <msgpack.hpp>
#pragma warning( pop )

class SFMsgPackPacket;
class SFPacketIOBuffer;

class SFMsgPackProtocol : public SFBasePacketProtocol
{
public:
	SFMsgPackProtocol();
	virtual ~SFMsgPackProtocol();

	bool Initialize(int ioBufferSize, unsigned short packetSize, int packetOption);

	// ----------------------------------------------------------------
	//  Name:           Reset
	//  Description:    프로토콜 객체를 재 활용하기 위해 패킷 IO 버퍼를 초기화한다.	
	// ----------------------------------------------------------------
	bool Reset();

	// ----------------------------------------------------------------
	//  Name:           AddTransferredData
	//  Description:    유저가 전송한 데이터를 패킷 IO 버퍼에 저장한다.	
	// ----------------------------------------------------------------
	bool AddTransferredData(char* pBuffer, DWORD dwTransferred);

	// ----------------------------------------------------------------
	//  Name:           GetPacket
	//  Description:    유저가 전송한 데이터를 이용해서 SFMsgPackPacket 패킷 객체를 생성한다.
	//					해당 패킷 객체는 로직 층으로 전송된다.
	// ----------------------------------------------------------------
	BasePacket* GetPacket(int& errorCode);

	// ----------------------------------------------------------------
	//  Name:           Encode
	//  Description:    
	// ----------------------------------------------------------------
	bool Encode(BasePacket* pPacket, char** ppBuffer, int& bufferSize);

	// ----------------------------------------------------------------
	//  Name:           DisposePacket
	//  Description:    패킷을 수거한다.
	// ----------------------------------------------------------------
	static bool DisposePacket(BasePacket* pPacket);

	// ----------------------------------------------------------------
	//  Name:           CreatePacket
	//  Description:    깨끗한 SFMsgPackPacket 객체를 얻어온다.
	// ----------------------------------------------------------------
	BasePacket* CreatePacket();

protected:

private:
	SFPacketIOBuffer* m_pPacketIOBuffer;
};

