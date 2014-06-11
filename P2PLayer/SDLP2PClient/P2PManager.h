#pragma once
#include "../Common/P2PCommon.h"
#include "../Interface/P2PClient.h"
#include "../Interface/IP2PManager.h"
#include "SDL_net.h"
#include <map>

class IUDPNetworkCallback;

class P2PManager : public IP2PManager
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
	BOOL DataSend(const char* pszIP, USHORT uiPort, USHORT uiLen, const BYTE* pbyBuff); //특정 아이피에 데이터 보내기	

	void HandleRecv();
	bool UpdateConnectionState();
	bool SendUDPPingPongPacket(int channel);
	bool EncodePacket(void* pData, UDPpacket* pPacket, int len);

	P2PPeerInfo* FindPeer(int channel);

protected:
	void ProcessPublicIP(UDPpacket* pPacket);
	void ProcessPingPongReq(UDPpacket* pPacket);
	void ProcessPingPongRes(UDPpacket* pPacket);

private:
	struct lfds611_queue_state* m_pQueue;
	int m_peerIndex;
	std::map<int, P2PPeerInfo> m_mapPeer;

	///////////////////////////////////////////////////////////////////////////////////////
	BOOL PushPacket(BYTE* pData, int Length);
};

