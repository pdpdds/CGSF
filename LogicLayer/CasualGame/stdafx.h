// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers



// TODO: reference additional headers your program requires here
#include <windows.h>

#pragma warning(disable : 4996)

#include <ace/ace.h>
#include <ace/Thread_Manager.h>
#include <ace/Thread_Mutex.h>
#include <ace/Proactor.h>
#include <ace/Task.h>

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

// TODO: reference additional headers your program requires here
#include "SFConstant.h"
#include "EngineInterface/EngineStructure.h"
#include "SFGameConstant.h"
#include "SFStructure.h"
#include "SFGameStructure.h"
#include <SFPacketStore/SFPacketStore.pb.h>
#include <SFPacketStore/SFPacketID.h>
#include <SFPacketStore/PacketCore.pb.h>
#include "SFMacro.h"
#include "SFMessage.h"
#include "SFSendPacket.h"
#include "SFPacket.h"
#include "SFLogicEntry.h"
#include "SFSinglton.h"
#include "SFEngine.h"
#include "SFProtobufPacket.h"

using namespace google;
//#pragma comment(lib, "libprotobuf.lib")