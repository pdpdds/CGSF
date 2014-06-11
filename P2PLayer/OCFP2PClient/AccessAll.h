///////////////////////////////////////////////////////////////////////////////////////////
//
// Date			: 2006.11.17
// Programmer	: Lee Yong Hyu
// Desc			: 
//

#ifndef __ACCESSALL_H__
#define __ACCESSALL_H__

#pragma warning(disable : 4996)

///////////////////////////////////////////////////////////////////////////////////////////
//
#include "AllOcfClientSys.h"
#include "AllOcfClient.h"

#pragma comment(lib, "ws2_32.lib") 

////////////////////////////////////////////////////////////////////////////////
//
/*#ifdef _DEBUG
	#ifdef OCF_DLL
		#pragma comment(lib,"AccessNlsSD.lib")
	#else
		#pragma comment(lib,"AccessNlsD.lib")
	#endif
#else
	#ifdef OCF_DLL
		#pragma comment(lib,"AccessNlsS.lib")
	#else
		#pragma comment(lib,"AccessNls.lib")
	#endif
#endif*/

///////////////////////////////////////////////////////////////////////////////////////////
//
#pragma warning(disable: 4786)

#include <vector>
using namespace std;
///////////////////////////////////////////////////////////////////////////////////////////
//
#include "PuPktStructure.h"
#include "PuCfg.h"
#include "PuLog.h"
#include "PeerInfo.h"
#include "CtrlPeers.h"

#endif
