#pragma once
#include "service_define_template.h"
#include "testInterface.h"

typedef enum RPC_CONSTANT {
	RPC_OK = 0,
	RPC_OK_INVALID_SERVICE_NAME,
	RPC_OK_INVALID_METHOD_ID,
	RPC_OK_PEER_DISCONNECTED
};

BEGIN_DEFINE_SERVICE(TestInterface)
   //Expose method: virtual void echo(std::string& str) = 0;
   EXPOSE_METHOD(TestInterface,echo) /* zzz */
   EXPOSE_METHOD(TestInterface,echo1) // zzz
   EXPOSE_METHOD(TestInterface,echo2)
   EXPOSE_METHOD(TestInterface,echo3)
   EXPOSE_METHOD(TestInterface,echo4)
   EXPOSE_METHOD(TestInterface,echo5)
   EXPOSE_METHOD(TestInterface,echo6)
   EXPOSE_METHOD(TestInterface,echo7)
   EXPOSE_METHOD(TestInterface,echo8)
   EXPOSE_METHOD(TestInterface,echo9)
   EXPOSE_METHOD(TestInterface,echo10)

END_DEFINE_SERVICE