////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2003.01.02
// 설  명 :
// 수정일 : 2007.11.13
//

/////////////////////////////////////////////////////////////////////////////////////////
//
#include "AllOcfClientSys.h"
#include "AllOcfClient.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
CUdpCommClient::CUdpCommClient():
	CThreadClient()
{
	WSADATA wsaData;
	if (!WSAStartup(MAKEWORD(2,0), &wsaData))
	{
		m_bSockLibFlag = TRUE;
	}
	else
	{
		m_bSockLibFlag = FALSE;
	}

	//
	m_sinLocal.sin_family = AF_INET;
	m_sinLocal.sin_port = 0;
	m_sinLocal.sin_addr.s_addr = ADDR_ANY;

	//
	m_dSock = INVALID_SOCKET;
	m_hSockEvent = WSA_INVALID_EVENT;
	m_hWrEvent = NULL;
	m_bWouldBlock = FALSE;
	m_nErrorCode = 0;

	//
// 	m_ulMaxSndPktLen = 0xFFFF;
// 	m_ulMaxRcvPktLen = 0xFFFF;

 	m_ulMaxSndPktLen = 1024*1024*10;
 	m_ulMaxRcvPktLen = 1024*1024*10;

// 	m_ulSndBuff = 0xFFFF;
// 	m_ulRcvBuff = 0xFFFF;

 	m_ulSndBuff = 1024*1024*10;
 	m_ulRcvBuff = 1024*1024*10;

	//
	m_dwIdleTime = INFINITE;
	m_tTimeout = 0;
	m_tSessionTimeout = 0;

	//
	m_bConnectFlag = FALSE;

	//
	m_hQuit = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hWrEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

CUdpCommClient::~CUdpCommClient()
{
	//
	if (NULL != m_hWrEvent)
	{
		CloseHandle(m_hWrEvent);
		m_hWrEvent = NULL;
	}

	//
	if (NULL != m_hQuit)
	{
		CloseHandle(m_hQuit);
		m_hQuit = NULL;
	}

	//
	if (TRUE == m_bSockLibFlag)
	{
		WSACleanup();
	}
}

BOOL 
CUdpCommClient::Init()
{
	//
	if (FALSE == CThreadClient::Init())
	{
		return FALSE;
	}

	//
	if (FALSE == m_bSockLibFlag)
	{
		return FALSE;
	}

	//
	m_tTimeout = time(NULL);

	//
	if (NULL == m_hQuit)
	{
		m_nErrorCode = GetLastError();
		HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
		return FALSE;
	}

	//
	if (NULL == m_hWrEvent)
	{
		m_nErrorCode = GetLastError();
		HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
		return FALSE;
	}

	//
	// 일단 UDP로 
	//
	m_dSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == m_dSock)
	{
		m_nErrorCode = WSAGetLastError();
		HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
		return FALSE;
	}

	//
	if (FALSE == UdpFixBind())
	{
		return FALSE;
	}


	//... SOO SOCKET BUFF SIZING : 
//  	m_ulRcvBuff = 1024 * 1024 * 1000;
//  	m_ulSndBuff = 1024 * 1024 * 1000;

	//
	if (SOCKET_ERROR == setsockopt(m_dSock, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&m_ulRcvBuff), sizeof(m_ulRcvBuff)))
	{
		m_nErrorCode = WSAGetLastError();
		HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
	}
	
	//
	if (SOCKET_ERROR == setsockopt(m_dSock, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(&m_ulSndBuff), sizeof(m_ulSndBuff)))
	{
		m_nErrorCode = WSAGetLastError();
		HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
	}
	
	//
	int nLenVariable = sizeof(m_ulMaxSndPktLen);
	if (SOCKET_ERROR == getsockopt(m_dSock, SOL_SOCKET, SO_MAX_MSG_SIZE, reinterpret_cast<char*>(&m_ulMaxSndPktLen), &nLenVariable))
	{
		m_nErrorCode = WSAGetLastError();
		HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
		return FALSE;
	}

	//
	if (FALSE == m_cRecvChunk.Ext(m_ulMaxRcvPktLen + 1))
	{
		m_nErrorCode = WSAGetLastError();
		HandleError(OCFCLIENT_ERROR_USERDEF, OCFCLIENT_ERROR_USERDEF_CATEGORY_MEMORY, 0xFF, 0, NULL);
		return FALSE;
	}

	//
	m_hSockEvent = WSACreateEvent();
	if (WSA_INVALID_EVENT == m_hSockEvent)
	{
		m_nErrorCode = WSAGetLastError();
		HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
		return FALSE;
	}

	//
	// Only For UDP
	//
	if (WSAEventSelect(m_dSock, m_hSockEvent, FD_READ | FD_WRITE) == SOCKET_ERROR)
	{
		m_nErrorCode = WSAGetLastError();
		HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
		return FALSE;
	}

	//
	m_bConnectFlag = TRUE;
	m_bWouldBlock = FALSE;

	return TRUE;
}

void 
CUdpCommClient::Uninit()
{
	if (INVALID_SOCKET != m_dSock)
	{
		WSAEventSelect(m_dSock, m_hSockEvent, 0);
		closesocket(m_dSock);
		m_dSock = INVALID_SOCKET;
	}

	//
	if (WSA_INVALID_EVENT != m_hSockEvent)
	{
		WSACloseEvent(m_hSockEvent);
		m_hSockEvent = WSA_INVALID_EVENT;
	}

	//
	m_bWouldBlock = FALSE;
	m_bConnectFlag = FALSE;

	//
	CThreadClient::Uninit();
}

BOOL 
CUdpCommClient::SetBindInfo(const char* pszAddr, UINT uiPort, ULONG ulMaxSndMsgPkLen, ULONG ulMaxRcvMsgPkLen, ULONG ulSndBuff, ULONG ulRcvBuff)
{
	LPHOSTENT pHent;

	//
	m_sinLocal.sin_family = AF_INET;
	m_sinLocal.sin_port = htons(uiPort);
	m_sinLocal.sin_addr.s_addr = inet_addr(pszAddr);

	//
	if (m_sinLocal.sin_addr.s_addr == INADDR_NONE)
	{
		pHent = gethostbyaddr(pszAddr,strlen(pszAddr) ,AF_INET);
		//pHent = gethostbyname(pszAddr);
		if (pHent == NULL)
		{
			m_nErrorCode = WSAGetLastError();
			HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
			return FALSE;
		}

		m_sinLocal.sin_addr.s_addr = *(u_long *)pHent->h_addr;
	}

	//
	m_ulMaxSndPktLen = ulMaxSndMsgPkLen;
	m_ulMaxRcvPktLen = ulMaxRcvMsgPkLen;
	m_ulSndBuff = ulSndBuff;
	m_ulRcvBuff = ulRcvBuff;

	return TRUE;
}

BOOL
CUdpCommClient::GetLocalAddrInfo(SOCKADDR_IN& sinLocal)
{
	int nLenAddr = sizeof(SOCKADDR_IN);

	//
	if (SOCKET_ERROR == getsockname(m_dSock, reinterpret_cast<struct sockaddr*>(&sinLocal), &nLenAddr))
	{
		return FALSE;
	}

	//
	if (ADDR_ANY != sinLocal.sin_addr.S_un.S_addr)
	{
		return TRUE;
	}

	//
	char achHostName[1024];

	if (SOCKET_ERROR == gethostname(achHostName, sizeof(achHostName)))
	{
		return FALSE;
	}

	struct hostent *pstHostEnt = gethostbyname(achHostName);
	if (NULL == pstHostEnt)
	{
		return FALSE;
	}

	if(NULL == pstHostEnt->h_addr_list)
	{
		return FALSE;
	}

    memcpy(&sinLocal.sin_addr, *(pstHostEnt->h_addr_list), sizeof(sinLocal.sin_addr));
	
// 	for ( int i = 0; pstHostEnt->h_addr_list[i]; i++)
// 	{
// 
// //		addr_fprint(pstHostEnt->h_addr_list[i], dst);
// 
// 		if (pstHostEnt->h_addr_list[i])
// 		{
// 
// 				BYTE split[4];
// 				ULONG ip;
// 				ULONG *x = (ULONG *) pstHostEnt->h_addr_list[i];
// 
// 				ip = ntohl(*x);
// 				split[0] = (ip & 0xff000000) >> 24;
// 				split[1] = (ip & 0x00ff0000) >> 16;
// 				split[2] = (ip & 0x0000ff00) >> 8;
// 				split[3] = (ip & 0x000000ff);
// 
// 		}
// 
// 	}

	return TRUE;
}

BOOL 
CUdpCommClient::Do()
{
	DWORD dwIdx;
	HANDLE awsaEvent[3];

	//
	awsaEvent[0] = m_hSockEvent;
	awsaEvent[1] = m_hWrEvent;
	awsaEvent[2] = m_hQuit;

	//
	while (FALSE == CThreadClient::IsEnd())
	{
		dwIdx = ::WaitForMultipleObjects(3, awsaEvent, FALSE, m_dwIdleTime);

		switch (dwIdx)
		{
			case WAIT_OBJECT_0:
				OnWinSockEvent();
				break;

			case (WAIT_OBJECT_0 + 1):
				HandleWrite();
				break;

			case (WAIT_OBJECT_0 + 2):
				break;

			case WAIT_TIMEOUT:
				IdleWrite();
				break;
		}

		//
		if (0 < m_tSessionTimeout && difftime(time(NULL), m_tTimeout) > m_tSessionTimeout)
		{
			m_tTimeout = time(NULL);
			HandleIdle();
		}
	}

	return TRUE;
}

BOOL
CUdpCommClient::DoEnd()
{
	CThreadClient::DoEnd();
	SetEvent(m_hQuit);

	return TRUE;
}

BOOL 
CUdpCommClient::Push(const SOCKADDR_IN& stRemote, ULONG ulLen, const BYTE* pbyBuff)
{
	if (ulLen >= m_ulMaxSndPktLen)
	{
		HandleError(OCFCLIENT_ERROR_USERDEF, OCFCLIENT_ERROR_USERDEF_CATEGORY_PKTMAXSIZE, 0, ulLen, pbyBuff);
		return FALSE;
	}

	//
	CUdpPacketNode* pcPktNode = m_cSndPktPool.Allocate();
	if (NULL == pcPktNode)
	{
		HandleError(OCFCLIENT_ERROR_USERDEF, OCFCLIENT_ERROR_USERDEF_CATEGORY_MEMORY, 0, ulLen, pbyBuff);
		return FALSE;
	}

	//
	if (FALSE == pcPktNode->Init(stRemote, ulLen, pbyBuff))
	{
		m_cSndPktPool.Deallocate(pcPktNode);

		HandleError(OCFCLIENT_ERROR_USERDEF, OCFCLIENT_ERROR_USERDEF_CATEGORY_MEMORY, 0, ulLen, pbyBuff);
		return FALSE;
	}

	//
	Lock();

	m_cSndPktList.Add(pcPktNode);

	if (TRUE == m_bConnectFlag && FALSE == m_bWouldBlock)
	{
		SetEvent(m_hWrEvent);
	}

	//
	Unlock();

	return TRUE;
}

BOOL
CUdpCommClient::Push(const SOCKADDR_IN& stRemote, ULONG ulLenHd, const BYTE* pbyHdBuff, ULONG ulLenBd, const BYTE* pbyBdBuff)
{
	if (ulLenHd + ulLenBd >= m_ulMaxSndPktLen)
	{
		HandleError(OCFCLIENT_ERROR_USERDEF, OCFCLIENT_ERROR_USERDEF_CATEGORY_PKTMAXSIZE, 0);
		return FALSE;
	}

	//
	CUdpPacketNode* pcPktNode = m_cSndPktPool.Allocate();
	if (NULL == pcPktNode)
	{
		HandleError(OCFCLIENT_ERROR_USERDEF, OCFCLIENT_ERROR_USERDEF_CATEGORY_MEMORY, 0);
		return FALSE;
	}

	//
	if (FALSE == pcPktNode->Init(stRemote, ulLenHd, pbyHdBuff, ulLenBd, pbyBdBuff))
	{
		m_cSndPktPool.Deallocate(pcPktNode);

		HandleError(OCFCLIENT_ERROR_USERDEF, OCFCLIENT_ERROR_USERDEF_CATEGORY_MEMORY, 0);
		return FALSE;
	}

	//
	Lock();

	m_cSndPktList.Add(pcPktNode);

	if (TRUE == m_bConnectFlag && FALSE == m_bWouldBlock)
	{
		SetEvent(m_hWrEvent);
	}

	//
	Unlock();

	return TRUE;
}

BOOL 
CUdpCommClient::Push(CUdpPacketNode* pcPktNode)
{
	if (pcPktNode->GetRawLen() >= m_ulMaxSndPktLen)
	{
		HandleError(OCFCLIENT_ERROR_USERDEF, OCFCLIENT_ERROR_USERDEF_CATEGORY_PKTMAXSIZE, 0, pcPktNode->GetRawLen(), pcPktNode->GetRawBuff());
		return FALSE;
	}

	//
	Lock();

	m_cSndPktList.Add(pcPktNode);

	if (TRUE == m_bConnectFlag && FALSE == m_bWouldBlock)
	{
		SetEvent(m_hWrEvent);
	}

	//
	Unlock();

	return TRUE;
}

void 
CUdpCommClient::Reset()
{
	CUdpPacketNode* pcPktNode;

	Lock();

	while (FALSE == m_cSndPktList.IsEmpty())
	{
		pcPktNode = static_cast<CUdpPacketNode*>(m_cSndPktList.DelBeginWithLive());
		m_cSndPktPool.Deallocate(pcPktNode);
	}

	Unlock();
}

BOOL
CUdpCommClient::UdpFixBind()
{
	if (0 != m_sinLocal.sin_port)
	{
		if (SOCKET_ERROR == bind(m_dSock, reinterpret_cast<const struct sockaddr *>(&m_sinLocal), sizeof(m_sinLocal)))
		{
			m_nErrorCode = WSAGetLastError();
			HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
			return FALSE;
		}

		return TRUE;
	}

	//
	// 이 포트 값의 갯수는 최소한 1개 이상이어야 한다...
	//
	const USHORT auiPort[] = { 57654, 51001, 54900, 52119, 58900, 59011, 52789, 0 };

	//
	int nI = 0;

	//
	do
	{
		m_sinLocal.sin_port = htons(auiPort[nI]);

		//
		if (SOCKET_ERROR != bind(m_dSock, reinterpret_cast<const struct sockaddr *>(&m_sinLocal), sizeof(m_sinLocal)))
		{
			return TRUE;
		}

		//
		m_nErrorCode = WSAGetLastError();
		HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
	} while (0 != auiPort[nI++]);

	return FALSE;
}

void 
CUdpCommClient::OnWinSockEvent()
{
	WSANETWORKEVENTS wsaEnumWork;

	if (SOCKET_ERROR == WSAEnumNetworkEvents(m_dSock, m_hSockEvent, &wsaEnumWork))
	{
		m_nErrorCode = WSAGetLastError();
		HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
		return;
	}

	if (wsaEnumWork.lNetworkEvents & FD_WRITE)
	{
		if (0 == wsaEnumWork.iErrorCode[FD_WRITE_BIT])
		{
			HandleWrite();
		}
		else
		{
			m_nErrorCode = wsaEnumWork.iErrorCode[FD_READ_BIT];
			HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
		}
	}

	if (wsaEnumWork.lNetworkEvents & FD_READ)
	{
		if (0 == wsaEnumWork.iErrorCode[FD_READ_BIT])
		{
			HandleRead();
		}
		else
		{
			m_nErrorCode = wsaEnumWork.iErrorCode[FD_READ_BIT];
			HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
		}
	}
}

BOOL 
CUdpCommClient::HandleWrite()
{
	int nRet;
	CUdpPacketNode* pcPktNode;

	//
	m_bWouldBlock = FALSE;

	//
	while (TRUE)
	{
		Lock();

		//
		if (TRUE == m_cSndPktList.IsEmpty())
		{
			Unlock();
			break;
		}

		/// 가져오는 순간 지운다...
		/// 값을 가져오고 성공시 지우는 방법으로 가야한다

		pcPktNode = static_cast<CUdpPacketNode*>(m_cSndPktList.DelBeginWithLive());
		
		//pcPktNode = static_cast<CUdpPacketNode*>(m_cSndPktList.GetBegin());

		//
		Unlock();

		//
        nRet = sendto(m_dSock, reinterpret_cast<const char*>(pcPktNode->GetRawBuff()), pcPktNode->GetRawLen(), 0, reinterpret_cast<SOCKADDR *>(&(pcPktNode->GetRemoteAddress())), pcPktNode->GetRemoteAddressLength());
		if (SOCKET_ERROR == nRet)
        {
            if (WSAEWOULDBLOCK == WSAGetLastError())
            {

				Lock();
				// WSAGetLastError 를 보고 판별하여 하면될듯하다
				m_cSndPktList.AddAtFirst(pcPktNode);

				Unlock();

				m_bWouldBlock = TRUE;
				break;
            }
			else
			{
				m_nErrorCode = WSAGetLastError();
				HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
			}
        }

		// 잘생각해보자
		//m_cSndPktList.Delete(pcPktNode);
	
		//
		m_cSndPktPool.Deallocate(pcPktNode);
	}

	return TRUE;
}

BOOL 
CUdpCommClient::HandleRead()
{
	int nRet;
	int nLen;
	SOCKADDR_IN stRemote;

	//
	nLen = sizeof(stRemote);

	//
    nRet = recvfrom(m_dSock, reinterpret_cast<char*>(m_cRecvChunk.GetMemChunk()), m_ulMaxRcvPktLen, 0, reinterpret_cast<SOCKADDR *>(&stRemote), &nLen);
    if (SOCKET_ERROR == nRet)
	{
		m_nErrorCode = WSAGetLastError();
		HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
	
		return FALSE;
	}

	//
	Pop(stRemote, nRet, reinterpret_cast<const BYTE*>(m_cRecvChunk.GetMemChunk()));

	return TRUE;
}

void
CUdpCommClient::HandleIdle()
{
}

void
CUdpCommClient::IdleWrite()
{
	int nRet;
	CUdpPacketNode* pcPktNode;

	//
	if (TRUE == m_bWouldBlock)
	{

		return;
	}

	//
	while (TRUE)
	{
		Lock();

		//
		if (TRUE == m_cSndPktList.IsEmpty())
		{
			Unlock();
			break;
		}

		pcPktNode = static_cast<CUdpPacketNode*>(m_cSndPktList.DelBeginWithLive());

		//
		Unlock();

		//
        nRet = sendto(m_dSock, reinterpret_cast<const char*>(pcPktNode->GetRawBuff()), pcPktNode->GetRawLen(), 0, reinterpret_cast<SOCKADDR *>(&(pcPktNode->GetRemoteAddress())), pcPktNode->GetRemoteAddressLength());
        if (SOCKET_ERROR == nRet)
        {
            if (WSAEWOULDBLOCK == WSAGetLastError())
            {
				Lock();
				
				m_cSndPktList.AddAtFirst(pcPktNode);

				Unlock();

				m_bWouldBlock = TRUE;
		     }
			else
			{
				m_nErrorCode = WSAGetLastError();
				HandleError(OCFCLIENT_ERROR_WINSOCK, m_nErrorCode, 0xFF, 0, NULL);
			}

            break;
        }

		//
		m_cSndPktPool.Deallocate(pcPktNode);
	}

}

