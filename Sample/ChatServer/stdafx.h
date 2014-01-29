// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"


#include <stdio.h>
#include <tchar.h>

#include "CommonHeader.h"
// TODO: reference additional headers your program requires here
///////////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include "SFPacketStore.pb.h"
#include "PacketID.h"
#ifdef _DEBUG
#pragma comment(lib, "aced.lib")
#else
#pragma comment(lib, "ace.lib")
#endif

#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "liblzf.lib")
#pragma comment(lib, "libprotobuf.lib")
#pragma comment(lib, "BaseLayer.lib")
#pragma comment(lib, "EngineLayer.lib")
#pragma comment(lib, "DatabaseLayer.lib")
#pragma comment(lib, "libglog.lib")