////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2003.03.08
// 설  명 :
//

#ifndef __ALLOCFCLIENT_H__
#define __ALLOCFCLIENT_H__

#pragma warning(disable : 4996)

////////////////////////////////////////////////////////////////////////////////
//
/*#ifdef _DEBUG
// 	#ifdef	_AFXDLL
// 		#pragma comment(lib,"OcfClientSD.lib")
// 	#else
		#pragma comment(lib,"OcfClientD.lib")
//	#endif
#else
// 	#ifdef	_AFXDLL
// 		#pragma comment(lib,"OcfClientS.lib")
// 	#else
		#pragma comment(lib,"OcfClient.lib")
//	#endif
#endif*/

////////////////////////////////////////////////////////////////////////////////
//
#include "OcfCommConst.h"
#include "CriticalSessionClient.h"
#include "DataStructureClient.h"
#include "CtrlMem.h"
#include "MemChunk.h"
#include "MemPool.h"
#include "ThreadCtrlClient.h"
#include "UdpPacketNode.h"
#include "UdpCommClient.h"
#include "SmartX.h"


#endif
