// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include "CommonHeader.h"

#include <windows.h>
#include <SFPacketStore/SFPacketStore.pb.h>

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