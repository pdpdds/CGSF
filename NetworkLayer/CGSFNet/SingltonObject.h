#pragma once
#include "ACEHeader.h"
#include "ProactorService.h"
#include "ProactorServiceManager.h"

typedef ACE_Singleton<ProactorServiceManager<ACE_Null_Mutex, ProactorService, 5000>, ACE_Thread_Mutex> ProactorServiceManagerSinglton;
