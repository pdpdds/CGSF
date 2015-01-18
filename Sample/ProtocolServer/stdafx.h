// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
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

#include "../../Common/SFPacketStore/ProtocolPacket.pb.h"
#include "../../Common/SFPacketStore/ProtocolPacketID.h"

#define GOOGLE_GLOG_DLL_DECL
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include "glog/logging.h"