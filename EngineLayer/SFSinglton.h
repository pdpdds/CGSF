#pragma once
#include <ace/Singleton.h>
#include <ace/Malloc_T.h>
#include <ace/Task.h>
#include "SFObjectPool.h"
#include "SFLogicGateway.h"
#include "SFPacketSendGateway.h"
#include "LogicEntry.h"
#include "SFPacket.h"

// class SFEngine;

typedef ACE_Singleton<SFObjectPool<SFPacket>, ACE_Null_Mutex> PacketPoolSingleton;
typedef ACE_Singleton<SFLogicGateway, ACE_Null_Mutex> LogicGatewaySingleton;
typedef ACE_Singleton<LogicEntry, ACE_Null_Mutex> LogicEntrySingleton;
typedef ACE_Singleton<SFPacketSendGateway, ACE_Null_Mutex> PacketSendSingleton;
//typedef ACE_Singleton<SFEngine, ACE_Null_Mutex> SFEngineSingleton;


typedef ACE_Dynamic_Cached_Allocator<ACE_Null_Mutex> ACEAllocator;
typedef ACE_Dynamic_Cached_Allocator<ACE_Thread_Mutex> ACETSAllocator;