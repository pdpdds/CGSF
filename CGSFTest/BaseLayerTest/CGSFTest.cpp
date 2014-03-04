// CGSFTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "INITest.h"
#include "DumpTest.h"
#include "BitArrayTest.h"
#include "StringTest.h"
#include "RexTest.h"
#include "SystemCheckTest.h"
#include "SFExcelTest.h"
#include "GLogTest.h"
#include "RegistryTest.h"
#include "LockQueueTest.h"
#include "CRCTest.h"
#include "DispatchTest.h"
#include "EncryptTest.h"
#include "CompressTest.h"

//#include "vld.h"
//#include "SFCheckSum.h"
#include "SFMessage.h"
#include "SFObjectPool.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CompressTest test;
	test.Run();
	getchar();
		
	return 0;
}
