// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#pragma warning(disable : 4996)

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

#include "ChatPacket.pb.h"
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>