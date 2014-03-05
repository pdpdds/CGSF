#ifndef __P2PMANAGER_H__
#define __P2PMANAGER_H__
#include "P2PClient.h"
#include <EngineInterface/IP2PManager.h>
#include "liblfds611.h"

class IUDPNetworkCallback;
class CPuPeers;

class P2PManager : public IP2PManager, public CPuPeers
{
public:
	P2PManager(void);
	virtual ~P2PManager(void);

	BOOL Initialize(IUDPNetworkCallback* pCallback) override;
	BOOL Finally() override;

	BOOL Update() override;

	BOOL RunP2P(char* szIP, unsigned short Port) override;
	BOOL EndP2P() override;

	BOOL AddPeer(PeerAddressInfo* pAddressInfo, BYTE& byIndex);//피어 추가 추가하면서 연결과 릴레이 가능한지 모두 체크한다.
	BOOL BroadCast(BYTE* pData, USHORT Size, BOOL ExceptMe = FALSE); //피어에게 데이터 전송. 피어에게 데이터 직접 전송이 불가능하면 릴레이 가능한 피어에게 데이터 전송. 그것도 안되면 릴레이 서버 이용
	BOOL BroadCastWithRelayServer(BYTE* pData, USHORT Size); //릴레이 서버를 이용해서 데이터 브로드 캐스트
	BOOL Disconnect(); //피어들과의 연결을 끊는다.
	BOOL BroadcastEcho(); //피어들에게 에코 신호를 보낸다.
	BOOL RemovePeer(BYTE byIndex); //특정피어 제거..
	BOOL DataSend( const char* pszIP, USHORT uiPort, USHORT uiLen, const BYTE* pbyBuff); //특정 아이피에 데이터 보내기	

protected:
	void ProcessDataPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) override;
	void OnResCheckExternalIp( const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulRemoteE, USHORT uiRemotePortE ) override;

private:
	SOCKADDR_IN m_sinLocal;
	struct lfds611_queue_state* m_pQueue;

///////////////////////////////////////////////////////////////////////////////////////
	BOOL PushPacket(BYTE* pData, int Length);
};

#endif