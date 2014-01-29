// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here
///////////////////////////////////////////////////////////////////////////////////
#include <ace/ace.h>
#include <ace/Thread_Manager.h>
#include <ace/Thread_Mutex.h>
#include <ace/Proactor.h>
#include <ace/Task.h>
///////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "SFMacro.h"
#include "SFConstant.h"
#include "SFStructure.h"
#include "Macro.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

using namespace google;