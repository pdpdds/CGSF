// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers



// TODO: reference additional headers your program requires here
#include <windows.h>
#include "DBStruct.h"
#include "EngineInterface/EngineStructure.h"

#include <ace/ace.h>
#include <ace/Thread_Manager.h>
#include <ace/Thread_Mutex.h>
#include <ace/Proactor.h>
#include <ace/Task.h>

#define GOOGLE_GLOG_DLL_DECL
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include "glog/logging.h"