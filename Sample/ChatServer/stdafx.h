// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#pragma warning(disable : 4996)
#include <ace/ace.h>
#include <ace/Thread_Manager.h>
#include <ace/Thread_Mutex.h>
#include <ace/Proactor.h>
#include <ace/Task.h>


// TODO: reference additional headers your program requires here
///////////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include "SFStructure.h"
#include "SFMacro.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "ChatPacket.pb.h"