// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// TODO: reference additional headers your program requires here
#include <windows.h>
#include "CommonHeader.h"

#include "BasePacket.h"

#include <tchar.h>
#include "Macro.h"
#include "SFString.h"
#include "SFCompressLzf.h"
#include "SFSinglton.h"
#include "SFPacketProtocol.H"
#include "SFBridgeThread.h"

#define GOOGLE_GLOG_DLL_DECL
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include "glog/logging.h"