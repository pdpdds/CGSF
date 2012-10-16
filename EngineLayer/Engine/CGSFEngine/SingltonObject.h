#pragma once
#include "ACEHeader.h"
#include "ProactorService.h"
#include "ProactorIDMap.h"

typedef ACE_Singleton<ProactorServiceIDMap<ACE_Thread_Mutex, ProactorService, 5000>, ACE_Thread_Mutex> ProactorServiceMapSingleton;
