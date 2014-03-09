// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include "ACEHeader.h"
#include "CommonHeader.h"

#include <windows.h>
#include <SFPacketStore/FPSPacket.pb.h>
#include <SFPacketStore/FPSPacketID.h>

#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "liblzf.lib")
#pragma comment(lib, "libprotobuf.lib")
#pragma comment(lib, "BaseLayer.lib")
#pragma comment(lib, "CasualGame.lib")
#pragma comment(lib, "EngineLayer.lib")
#pragma comment(lib, "DatabaseLayer.lib")