// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

//additional header
#include "ACEHeader.h"
#include <windows.h>
#include "CommonHeader.h"
#include "SFEngine.h"

#include <SFPacketStore/SevenGamePacket.pb.h>
#include <SFPacketStore/SevenGamePacketID.h>

#include "Define.h"

#pragma comment(lib, "EngineLayer.lib")
#pragma comment(lib, "CasualGame.lib")
