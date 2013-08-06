#pragma warning(disable : 4996)
#include <ace/ace.h>
#include <ace/Thread_Manager.h>
#include <ace/Thread_Mutex.h>
#include <ace/Proactor.h>
#include <ace/Task.h>

#include "SFConstant.h"
#include "SFStructure.h"
#include "SFMacro.h"

#define GOOGLE_GLOG_DLL_DECL
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include "glog/logging.h"

#pragma comment(lib, "libglog.lib")