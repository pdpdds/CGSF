// CGSFTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "vld.h"
//#include "SFPacketAnalyzer.h"
#include "SFPacket.h"
#include "SFCompressZLib.h"
#include "SFCompressLzf.h"
#include "SFCompressor.h"
//#include "SFCheckSum.h"
#include "SFEncryptionXOR.h"
#include "SFString.h"
#include "SFFString.h"
#include "SFFastCRC.h"
//#include "SFPacketStore.pb.h"
//#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "SFPacketHandler.h"
#include "SFDispatch.h"
#include "SFLockQueue.h"
#include "SFLockDeque.h"
#include "SFIOCPQueue.h"
#include "SFMessage.h"
#include "Mmsystem.h"
#include "SFRegistry.h"
#include "SFObjectPool.h"
//#include "SFNetworkHandler.h"
#include <map>
#include "SFIni.h"
#include "SFServiceController.h"
#include "SFBitArray.h"
#include "SFSampleDB.h"
#include "SFFilePack.h"
#include "SFUtil.h"
#include "SFSystemInfo.h"
#include "GPGLockFreeQueue.h"
#include "SFRegexChecker.h"
//#include "EHManager.h"
#include "SFExtensionClass.h"
#include "CPUDesc.h"
//#include "SFExcel.h"
//#include "CSMTP.h"
//#include <tomcrypt.h>
#include <SFMath.h>
//#include "SFProactiveServer.h"
//#include "liblfds.h"


//#include <boost/serialization/serialization.hpp>
//#include <boost/serialization/vector.hpp>
//#include <boost/serialization/string.hpp>
//#include <boost/archive/text_oarchive.hpp>
//#include <boost/archive/text_iarchive.hpp>
//#include <boost/archive/binary_iarchive.hpp> 
//#include <boost/archive/binary_oarchive.hpp>
#include <fstream>
#include <iostream> 
#include <sstream>  

using namespace std;

/*class SampleBoostSerialization
{
	friend class boost::serialization::access;

public:
	SampleBoostSerialization(){}
	virtual ~SampleBoostSerialization(){}

	void SetHP(int HP){m_HP = HP;}
	void SetMP(int MP){m_MP = MP;}

protected:

private: 
	template<class Archive> void serialize( Archive& ar, unsigned int ver )
	{
		ar & m_HP;
		ar & m_MP;
	}

private:
	int m_HP;
	int m_MP;
};*/

#pragma comment(lib, "enginelayer.lib")
#pragma comment(lib, "baselayer.lib")
#pragma comment(lib, "databaselayer.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "liblzf.lib")
#pragma comment(lib, "libprotobuf.lib")
//#pragma comment(lib, "tomcrypt.lib")
#ifdef _DEBUG
#pragma comment(lib, "aced.lib")
#else
#pragma comment(lib, "ace.lib")
#endif

#pragma comment(lib, "Winmm.lib")

#define MAX_NICKNAME_LEN 8
DECLARE_STRING_TYPE(SSNickName, MAX_NICKNAME_LEN);

SFLockQueue<int> LockQueue;
SFIOCPQueue<int> IOCPQueue;
SFLockDeque<int> LockDeque;

static void FakeWorkThread(void* Args)
{
	//ACE_Proactor::instance()->proactor_run_event_loop();

	SFTSSyncQueue<int>* pQueue = (SFTSSyncQueue<int>*)Args;

	for(int i = 0; i < 3000; i++)
	{
		int* p = new int;
		*p = i;
		pQueue->Push(p);
	}
}

static void BusinessThread(void* Args)
{
	SFTSSyncQueue<int>* pQueue = (SFTSSyncQueue<int>*)Args;
	while(1)
	{
		int* Num = pQueue->Pop();
		//printf("%d\n", *Num);
		delete Num;
	}
}

/*void sampleFunc(protobuf::io::ArrayInputStream& input)
{
	return;
}*/

static void LockFreeProducer(void* Args)
{
	//ACE_Proactor::instance()->proactor_run_event_loop();

	GPG::LockFreeQueue<int>* pQueue = (GPG::LockFreeQueue<int>*)Args;

	for(int i = 0; i < 10000; i++)
	{
		
		GPG::node<int>* p = new GPG::node<int>(i);
		
		pQueue->Add(p);
	}
}

static void LockFreeConsumer(void* Args)
{
	//ACE_Proactor::instance()->proactor_run_event_loop();

	GPG::LockFreeQueue<int>* pQueue = (GPG::LockFreeQueue<int>*)Args;

	for(int i = 0; i < 10000; i++)
	{
		GPG::node<int>* p = pQueue->Remove();

		if(p != NULL)
			delete p;
	}
}

struct PureCallBase 
{ 
	PureCallBase() { mf(); } 
	void mf() 
	{ 
		pvf(); 
	} 
	virtual void pvf() = 0; 
};

struct PureCallExtend : public PureCallBase
{ 
	PureCallExtend() {}
	virtual void pvf() {}
};


void GPGLockFreeQueueTest();
void RegularExpressionTest();
void DatabaseTest();
void BitArrayTest();
void ServiceTest(int argc, TCHAR* argv[]);
void SystemCheckTest();
void FileMemoryPackingTest();
void BoostSerializationTest();
void ServerCrashTest();
void SFExcelTest();
void SendEMailTest();
void TomCryptTest();
void DataStructureTest();
void AceLoggerTest();
void AceServerTest();
void SQLiteTest();
void PCRETest();
void ACEDataStructureTest();
void LokiTest();
void libfdsTest();

int _tmain(int argc, _TCHAR* argv[])
{
	/*EHManager ProcessDump;
	//if(FALSE == ProcessDump.Install(DL_MY_HANDLER_STACKOVERFLOW, EH_BUGTRAP))
	if(FALSE == ProcessDump.Install(DL_MY_HANDLER_STACKOVERFLOW, EH_MINIDUMP))
	{
		printf("예외 핸들러 설치 실패\n");
		getchar();
		return 0;
	}*/

	//AceLoggerTest();

	//ACEDataStructureTest();

	LokiTest();

	//libfdsTest();

//////////////////////////////////////////////////////////////////////////
//게임 프로그래머를 위한 자료구조와 알고리즘 소스 테스트
//////////////////////////////////////////////////////////////////////////
	DataStructureTest();

/////////////////////////////////////////////////////////////////////////
//ACE Server Test
//////////////////////////////////////////////////////////////////////////
	//AceServerTest();

	//SQLiteTest();

	//PCRETest();
//////////////////////////////////////////////////////////////////////////
//서비스 생성 삭제, 시작, 제어 관련 테스트
//////////////////////////////////////////////////////////////////////////
//ServiceTest(argc, argv);

//////////////////////////////////////////////////////////////////////////
//파일 패킹, 압축 및 해제(메모리상) 테스트
//////////////////////////////////////////////////////////////////////////
	//FileMemoryPackingTest();
	
//////////////////////////////////////////////////////////////////////////
//SystemCheck Test
//////////////////////////////////////////////////////////////////////////
	//SystemCheckTest();

//////////////////////////////////////////////////////////////////////////
//LockFreeQueueTest
//////////////////////////////////////////////////////////////////////////	
	//GPGLockFreeQueueTest();

//////////////////////////////////////////////////////////////////////////
//Regular Expression Check
//////////////////////////////////////////////////////////////////////////
	//RegularExpressionTest();

//////////////////////////////////////////////////////////////////////////
//Database Test
//////////////////////////////////////////////////////////////////////////
	//DatabaseTest();

//////////////////////////////////////////////////////////////////////////
//BitArray Test
//////////////////////////////////////////////////////////////////////////
	//BitArrayTest();

//////////////////////////////////////////////////////////////////////////
//부스트 직렬화 테스트
//////////////////////////////////////////////////////////////////////////
	//BoostSerializationTest();

	//SFExcelTest();

	//SendEMailTest();
	//TomCryptTest();

//////////////////////////////////////////////////////////////////////////
//Process Dump Test
//////////////////////////////////////////////////////////////////////////
	//ServerCrashTest();

	return 0;
}

void GPGLockFreeQueueTest()
{
	
	ACE::init();

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	int OptimalThreadCount = si.dwNumberOfProcessors * 2;

	GPG::node<int>* pNode = new GPG::node<int>(-1);
	GPG::LockFreeQueue<int> GPGQueue(pNode);



	int GroupID = ACE_Thread_Manager::instance()->spawn_n(OptimalThreadCount, (ACE_THR_FUNC)LockFreeProducer, &GPGQueue, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1);

	if(GroupID == -1)
	{
		SFASSERT(0);
	}

	int ConsumerGroupID = ACE_Thread_Manager::instance()->spawn_n(OptimalThreadCount, (ACE_THR_FUNC)LockFreeConsumer, &GPGQueue, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2);

	if(ConsumerGroupID == -1)
	{
		SFASSERT(0);
	}

	ACE_Thread_Manager::instance()->wait_grp(GroupID);
	ACE_Thread_Manager::instance()->wait_grp(ConsumerGroupID);

	ACE::fini();
}

void RegularExpressionTest()
{
	SFRegexChecker Checker;

	//주민등록번호
	SFASSERT(FALSE == Checker.IsValidResidentRegistrationNumber(L"801234-798999"));
	SFASSERT(TRUE == Checker.IsValidResidentRegistrationNumber(L"801234-7989990"));
	SFASSERT(FALSE == Checker.IsValidResidentRegistrationNumber(L"801234-79899909"));

	//URL
	SFASSERT(TRUE == Checker.IsValidURL(L"http://onlinegameserver.org/444/"));
	SFASSERT(TRUE == Checker.IsValidURL(L"http://onlinegameserver.org/444"));
	SFASSERT(TRUE == Checker.IsValidURL(L"http://onlinegameserver.org"));
	SFASSERT(TRUE == Checker.IsValidURL(L"http://onlinegameserver"));
	SFASSERT(FALSE == Checker.IsValidURL(L"http://"));
	SFASSERT(FALSE == Checker.IsValidURL(L"http://["));

	//이메일
	SFASSERT(FALSE == Checker.IsValidEMail(L"email"));
	SFASSERT(FALSE == Checker.IsValidEMail(L"email@"));
	SFASSERT(FALSE == Checker.IsValidEMail(L"email@daum"));
	SFASSERT(FALSE == Checker.IsValidEMail(L"email@daum."));
	SFASSERT(TRUE == Checker.IsValidEMail(L"a@daum.net"));
	SFASSERT(TRUE == Checker.IsValidEMail(L"email-A@DAUM.Net"));
	SFASSERT(FALSE == Checker.IsValidEMail(L"email-A@DAUM.Neta"));
	SFASSERT(TRUE == Checker.IsValidEMail(L"email-A@DAUM.Ne"));
	SFASSERT(FALSE == Checker.IsValidEMail(L"email-A@DAUM.N"));
	SFASSERT(FALSE == Checker.IsValidEMail(L"email--A@DAUM.NET"));

	//IP Address
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333"));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333."));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333.333"));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333.333."));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333.333.333"));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333.333.333."));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333.333.333.333"));
	SFASSERT(TRUE == Checker.IsValidIPAddress(L"133.133.133.133"));
	SFASSERT(TRUE == Checker.IsValidIPAddress(L"255.255.255.255"));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"255.255.255.256"));
	SFASSERT(TRUE == Checker.IsValidIPAddress(L"56.255.255.255"));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333.333.333.333."));
	SFASSERT(FALSE == Checker.IsValidIPAddress(L"333.333.333.333.333"));

	//캐릭터이름
	SFASSERT(FALSE == Checker.IsValidCharName(L"★가나다abc-][(){}_AB194"));
	SFASSERT(TRUE == Checker.IsValidCharName(L"가나다abc-][(){}_AB194"));
	SFASSERT(FALSE == Checker.IsValidCharName(L"ㅁ가나다abc-][(){}_AB194"));
}

void DatabaseTest()
{
	SFSampleDB ODBC;

	_DBConnectionInfo Info = {L"ServiceName", L"test", L"ID", L"Password"};

	if(TRUE == ODBC.Initialize(&Info))
	{
		if(TRUE == ODBC.AddStatement())
		{
			ODBC.SPLoadUser(1, L"jUHANG");
		}
	}
}

void BitArrayTest()
{

	// initialize CBitArray object
	SFBitArray a;
	// set the bit 4578 in the bit array buffer (at byte 4578/8, at bit 4578%8)
	a.SetAt(4578);
	// set the bit 323 in the bit array buffer (at byte 323/8, at bit 323%8)
	a.SetAt(323);
	// get the count of '1's
	int nCount = a.GetCount(); // return 2
	// xor bit number 323
	a.XOrAt(323);
	// get the count of '1's
	nCount = a.GetCount(); // return 1
	// initialize CBitArray object
	SFBitArray b;
	// attach buffer which is allocated with some bytes
	//char* buffer[] = {1,1,1,1};
	//b.Attach((BYTE*)buffer, 4); 
	// AND b with a

	// ...
	// and so on

	BYTE* pBuffer = new BYTE[500000];
	memset(pBuffer, 0, 500000);
	b.Attach(pBuffer, 500000);
	b.SetAt(100000);
	b.SetAt(999999);

	////////////////////////////////////////////////////
	//5행 5열 타일에서 5행 6열의 타일이 보이는가?
	//5행 5열은 25번째 인덱스 5행 6열은 26번째 인덱스
	////////////////////////////////////////////////////
	//25* 100 + 250???
	nCount = b.GetCount();
	b.Detach();
	free(pBuffer);
}

void ServiceTest(int argc, TCHAR* argv[])
{
#ifdef _DEBUG
	//SFFacade Facade;
	//Facade.Initialize();
#else
	SFServiceController Controller;
	Controller.ServiceEntry(L"CGSF");
#endif

	if(argc == 2)
	{
		SFServiceController Controller;

		if(_tcscmp(argv[1], L"-I") == 0)
		{
			Controller.InstallService(L"CGSF", L"CGSF", L"D:\\NPSvr\\CGSFTest.exe");
		}
		else if(_tcscmp(argv[1], L"-U") == 0)
		{
			Controller.DeleteService(L"CGSFTest");
		}
	}
}

void SystemCheckTest()
{
	float Score = SFUtil::GetWindowAssetPoint();

	SFSystemInfo SystemInfo;

	if(TRUE == SystemInfo.Initialize())
	{
		OSInfo* pOSInfo = SystemInfo.GetOSInfo();
		DWORD dwOSMajorVersion = pOSInfo->dwOSMajorVer;
		DWORD dwOSMinorVersion = pOSInfo->dwOSMinorVer;

		printf("##Operation System Infomation##\n");
		printf("OSName : %s, Ver : %d.%d\n", pOSInfo->szOperatingSystem, dwOSMajorVersion, dwOSMinorVersion);

		MEMORYSTATUS* pMemoryInfo = SystemInfo.GetMemoryInfo();

		DWORD dwTotalPhysicalMemory = pMemoryInfo->dwTotalPhys;
		dwTotalPhysicalMemory = dwTotalPhysicalMemory /1048576;

		DWORD dwTotalAvailablePhysicalMemory = pMemoryInfo->dwAvailPhys;
		dwTotalAvailablePhysicalMemory = dwTotalAvailablePhysicalMemory /1048576;

		printf("\n\n##MemoryInfomation##\n");
		printf("Physical Memory : %dMB\n", dwTotalPhysicalMemory);
		printf("Available Memory : %dMB\n", dwTotalAvailablePhysicalMemory);

///////////////////////////////////////////////////////////////////////////////////////////////////
//CPU Info
///////////////////////////////////////////////////////////////////////////////////////////////////
		CCPUDesc* pDesc = SystemInfo.GetCPUDesc();
		if(true == pDesc->IsSupportCPUID())
		{	
			printf("\n\n##CPU Infomation##\n");
			printf("VenderID : %s\n", pDesc->GetVendorID());
			printf("TypeID : %s\n", pDesc->GetTypeID());
			printf("FamilyID : %s\n", pDesc->GetFamilyID());
			printf("ModelID : %s\n", pDesc->GetModelID());
			printf("SteppingCode : %s\n", pDesc->GetSteppingCode());
			printf("ExtendedProcessorName : %s\n", pDesc->GetExtendedProcessorName());
			printf("GetProcessorClockFrequency : %d\n", pDesc->GetProcessorClockFrequency());
			printf("GetProcessorNum : %d\n", pDesc->GetProcessorNum());
		}
		
		PPROCESSORINFO pProcessorsInfo = pDesc->GetProcessorsInfo();

		/*for(int nProcessorCount = 0; nProcessorCount < pDesc->GetProcessorNum(); nProcessorCount++)
		{
			printf("\n\n##Processor %d Infomation##\n",  pProcessorsInfo[nProcessorCount].dwProcessorIndex);

			if(TRUE == pProcessorsInfo[nProcessorCount].bL1Cache)
			{
				printf("L1Cache : %d\n", pProcessorsInfo[nProcessorCount].L1CacheSize);
			}

			if(TRUE == pProcessorsInfo[nProcessorCount].bL2Cache)
			{
				printf("L2Cache : %d\n", pProcessorsInfo[nProcessorCount].L2CacheSize);
			}

			if(TRUE == pProcessorsInfo[nProcessorCount].bL3Cache)
			{
				printf("L3Cache : %d\n", pProcessorsInfo[nProcessorCount].L3CacheSize);
			}

			if(TRUE == pProcessorsInfo[nProcessorCount].bHyperthreadingInstructions)
			{
				printf("bHyperthreadingInstructions Enable\n");
				printf("LogicalProcessorsPerPhysical : %d\n", pProcessorsInfo[nProcessorCount].LogicalProcessorsPerPhysical);
			}	

		}*/

		DWORD dwL1CacheSize = pDesc->GetL1CacheSize();
		DWORD dwL2CacheSize = pDesc->GetL2CacheSize();
		DWORD dwL3CacheSize = pDesc->GetL3CacheSize();

		DWORD dwMB = 0;
		DWORD dwKB = 0;

		SFUtil::ConvertBytesToMB(dwL1CacheSize, dwMB, dwKB);
		_tprintf(TEXT("L1 cache size : %dMB %dKB\n"), dwMB, dwKB);

		SFUtil::ConvertBytesToMB(dwL2CacheSize, dwMB, dwKB);
		_tprintf(TEXT("L2 cache size : %dMB %dKB\n"), dwMB, dwKB);

		SFUtil::ConvertBytesToMB(dwL3CacheSize, dwMB, dwKB);
		_tprintf(TEXT("L3 cache size : %dMB %dKB\n"), dwMB, dwKB);
	}
}

void FileMemoryPackingTest()
{
	SFFilePack FilePack;
	if(FilePack.Initialize() == TRUE)	
		FilePack.UnPackFile(_T("test.zip"));
}

/*void BoostSerializationTest()
{
	//std::ostringstream archive_stream;
	std::stringstream ss;
    {
        SampleBoostSerialization SamplePlayer;
		SamplePlayer.SetHP(100);
		SamplePlayer.SetMP(90);
       
        ofstream file("PlayerInfo.dat");
		
        boost::archive::text_oarchive oa(ss);

        oa << SamplePlayer;
    }

    {
        SampleBoostSerialization SamplePlayer;

		ifstream file("PlayerInfo.dat");
        boost::archive::text_iarchive ia(ss);
        ia >> SamplePlayer;

		int j = 6;
    }
}*/

void Func3()
{
	//int aLocalVar[2];
	//aLocalVar[3] = 0x45678;
}

void Func2()
{
	Func3();
}

void Func1()
{
	Func2();
}

void ServerCrashTest()
{
	

////////////////////////////////////////////////////////////////////////
//Generic Error
////////////////////////////////////////////////////////////////////////
	/*int* pPoint = 0;
	*pPoint = 1234;*/

////////////////////////////////////////////////////////////////////////
//CRT Error
////////////////////////////////////////////////////////////////////////
	/*TCHAR szData[100000] = L"SampleChatStringDataNoEndSpace!!";
	TCHAR szTargetBuffer[10] = {0,};
	_tcsncpy_s(szTargetBuffer, szData, 10);*/

////////////////////////////////////////////////////////////////////////
//Out Of Memory
////////////////////////////////////////////////////////////////////////
	/*SFExtensionClass* pExtensionClass = new SFExtensionClass();

	pExtensionClass->ProcessOutofMemory();*/

////////////////////////////////////////////////////////////////////////
//Heap Corruption
////////////////////////////////////////////////////////////////////////
	/*SFBaseClass* pBaseClass = new SFBaseClass();

	SFExtensionClass* pExtensionClass = static_cast<SFExtensionClass*>(pBaseClass);

	pExtensionClass->SetExtensionVar(12345678);
	pExtensionClass->ProcessHeapCorruption();	

	delete pBaseClass;*/

////////////////////////////////////////////////////////////////////////
//Pure Function Call
////////////////////////////////////////////////////////////////////////
	//PureCallExtend Temp;

////////////////////////////////////////////////////////////////////////
//Stack Overflow
////////////////////////////////////////////////////////////////////////
	/*SFExtensionClass* pExtensionClass = new SFExtensionClass();

	pExtensionClass->ProcessStackOverFlow();	

	delete pExtensionClass;*/

////////////////////////////////////////////////////////////////////////
//STATUS_ARRAY_BOUNDS_EXCEEDED : 확인해 볼 필요 있음
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//Return Address Manipulation (Stack Destroy)
////////////////////////////////////////////////////////////////////////
	Func1();
}

/*void SFExcelTest()
{
	SFExcel Excel;
	Excel.Initialize();
	Excel.Read("test.xls", 0);
	Excel.Finally();
}*/

/*void SendEMailTest()
{

	bool bError = false;

	try
	{
		CSmtp mail;

		//#define test_gmail_tls

#if defined(test_gmail_tls)
		//mail.SetSMTPServer("smtp.gmail.com",587);
		//mail.SetSecurityType(CSmtp::USE_TLS);
#elif defined(test_gmail_ssl)
		mail.SetSMTPServer("smtp.gmail.com",465);
		mail.SetSecurityType(CSmtp::USE_SSL);
#elif defined(test_hotmail_TLS)
		mail.SetSMTPServer("smtp.live.com",25);
		mail.SetSecurityType(CSmtp::USE_TLS);
#elif defined(test_aol_tls)
		mail.SetSMTPServer("smtp.aol.com",587);
		mail.SetSecurityType(CSmtp::USE_TLS);
#elif defined(test_yahoo_ssl)
		mail.SetSMTPServer("plus.smtp.mail.yahoo.com",465);
		mail.SetSecurityType(CSmtp::USE_SSL);
#endif

		mail.SetSMTPServer("smtp.daum.net",465);
		mail.SetSecurityType(USE_SSL);

		mail.SetLogin("***"); //daum id
		mail.SetPassword("***"); //daum password

		mail.SetSenderName("***"); //자기이름
		mail.SetSenderMail("***");//SenderMail
		mail.SetReplyTo("***");//SenderMail
		mail.SetSubject("The message");
		mail.AddRecipient("***"); //ReceiverMail
		mail.SetXPriority(XPRIORITY_HIGH);
		mail.SetXMailer("The Bat! (v3.02) Professional");
		mail.AddMsgLine("Hello,");
		mail.AddMsgLine("");
		mail.AddMsgLine("...");
		mail.AddMsgLine("How are you today?");
		mail.AddMsgLine("");
		mail.AddMsgLine("Regards");
		mail.ModMsgLine(5,"regards");
		mail.DelMsgLine(2);
		mail.AddMsgLine("User");

		//mail.AddAttachment("../test1.jpg");
		//mail.AddAttachment("c:\\test2.exe");
		//mail.AddAttachment("c:\\test3.txt");
		mail.Send();
	}
	catch(ECSmtp e)
	{
		std::cout << "Error: " << e.GetErrorText().c_str() << ".\n";
		bError = true;
	}
	if(!bError)
		std::cout << "Mail was send successfully.\n";
}*/



#include "Array2D.h"
#include "Queue.h"
#include "Heap.h"

using namespace CGBase;

int CompareIntDescending( int left, int right )
{
    if( left < right )
        return 1;

    if( left > right)
        return -1;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DataStructureTest()
{
/////////////////////////////////
//Array Test
/////////////////////////////////
	Array2D<int> Array2D_( 5, 4 );
	(*Array2D_.Get(4,3)) = 5;

	int* ArrayValue = Array2D_.Get(4,3);

	SFASSERT(*ArrayValue == 5);

/////////////////////////////////
//Queue Test
/////////////////////////////////
	LQueue<int> Queue;
	int Data = 5;
	Queue.Enqueue(Data);
	Queue.Enqueue(Data);

/////////////////////////////////
//Heap Test
/////////////////////////////////
	Heap<int> IntHeap( 100, CompareIntDescending );

	Data = 7;
	IntHeap.Enqueue(Data);
	Data = 10;
	IntHeap.Enqueue(Data);
	Data = 8;
	IntHeap.Enqueue(Data);

	int HeapTop = IntHeap.Item();

	SFASSERT(HeapTop == 7);

	/*SFMath::Vector2D Vec2d(5.0f, 4.0f);
	SFMath::Vector2D Vec2d2(5.0f, -4.0f);

	Vec2d = Vec2d + Vec2d2;*/

	SFMath::Vector2D Vec2d(1.0f, 0.0f);
	SFMath::Vector2D Vec2d2(-3.0f, 4.0f);

	Vec2d2 = Vec2d.Reflection(Vec2d2);

	SFMath::Vector3D Vec3d(5.0f, 4.0f, 7.0f);
	SFMath::Vector3D Vec3d2(5.0f, -4.0f, -4.0f);

	Vec3d = Vec3d + Vec3d2;

	Vec2d += Vec2d;

	Vec3d += Vec3d;

	return;

}

static void ProactiveWorkThread(void* Args)
{
	ACE_Proactor::instance()->proactor_run_event_loop();
}

/*void AceServerTest()
{
	ACE::init();

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	SFProactiveServer Server;
	Server.Run();

	int OptimalThreadCount =si.dwNumberOfProcessors * 2;

	DWORD dwTime = timeGetTime();
	int GroupID = ACE_Thread_Manager::instance()->spawn_n(OptimalThreadCount, (ACE_THR_FUNC)ProactiveWorkThread, &LockDeque, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1);

	if(GroupID == -1)
	{
		SFASSERT(0);
	}

	if(ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)BusinessThread, &LockDeque, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2) == -1)
	{
		SFASSERT(0);
	}

	ACE_Thread_Manager::instance()->wait_grp(GroupID);
	ACE_Thread_Manager::instance()->wait_grp(2);

	DWORD dwCompleteTime = timeGetTime();

	DWORD dwElapsed = dwCompleteTime - dwTime;

	printf("ElapsedTime %d\n", dwElapsed);

	ACE_Thread_Manager::instance()->wait_grp(2);

	ACE::fini();

	//delete pPacketHandler;
	//delete pCompress;
}*/

#include <ace/ace.h>
#include <ace/Task.h>
#include "ACELogger.h"

class MyCallback : public ACE_Log_Msg_Callback
{
public:
	void log(ACE_Log_Record& log_record)
	{
		ACE_Time_Value tv = log_record.time_stamp();
		FILETIME ft(tv);
		SYSTEMTIME st;
		FileTimeToSystemTime(&ft, &st);
		st.wHour += 9;
		printf("%02d-%02d %02d:%02d:%02d %s", 
			st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, log_record.msg_data());
	}
};

class LoggingTask : public ACE_Task_Base
{
public:
	int svc()
	{
		ACE_DEBUG((LM_INFO, "ProcessId(%P) ThreadId(%t) In Task\n"));
		return 0;
	}
};

void AceLoggerTest()
{

	ACE::init();

	// Process/Thread Id 출력
	ACE_DEBUG((LM_INFO, "ProcessId(%P) ThreadId(%t)\n"));
	LoggingTask task;
	task.activate(THR_NEW_LWP, 5);

	Sleep(1000);

	// Source code 정보 출력
	ACE_DEBUG((LM_INFO, "%N %l Source code Info\n"));

	// 날짜/시간 정보 출력
	ACE_DEBUG((LM_INFO, "%D Date Info\n"));

	// 로깅 출력방식 변경
	// 파일로 변경
	ACE_LOGGER->RedirectToFile(L"FileLog.txt");
	ACE_DEBUG((LM_INFO, "%D Date Info To File\n"));

	// 역호출 객체(Callback)로 변경
	ACE_LOGGER->RedirectToCallback(new MyCallback);
	ACE_DEBUG((LM_INFO, "Date Info To Callback\n"));
	
	getchar();
	ACE::fini();

}

void LogTest()
{
	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	/*google::InitGoogleLogging("CGSF");

	google::SetLogDestination(GLOG_ERROR, "./");

	LOG(ERROR) << "This should work";
	LOG_IF(ERROR, 6 > 5) << "This should be also OK";

	LOG(INFO) << "info";
	LOG(WARNING) << "warn";
	LOG(ERROR) << "error 주항";
	//LOG(FATAL) << "fatal";

	std::vector<std::string> dir = google::GetLoggingDirectories();
	std::vector<std::string>::iterator it;
	for(it=dir.begin(); it!=dir.end(); ++it){
		LOG(INFO) << *it;
	}	

	//DLOG(FATAL) << "주항";
	*/
}

#include "CppSQLite3.h"
#include <ctime>
#include <iostream>
const char* gszFile = "test.db";

#pragma comment(lib, "SQLite3.lib")

void SQLiteTest()
{
	int i, fld;
	time_t tmStart, tmEnd;
	CppSQLite3DB db;

	cout << "SQLite Version: " << db.SQLiteVersion() << endl;

	remove(gszFile);
	db.open(gszFile);

	cout << endl << "emp table exists=" << (db.tableExists("emp") ? "TRUE":"FALSE") << endl;
	cout << endl << "Creating emp table" << endl;
	db.execDML("create table emp(empno int, empname char(20));");
	cout << endl << "emp table exists=" << (db.tableExists("emp") ? "TRUE":"FALSE") << endl;
	////////////////////////////////////////////////////////////////////////////////
	// Execute some DML, and print number of rows affected by each one
	////////////////////////////////////////////////////////////////////////////////
	cout << endl << "DML tests" << endl;
	int nRows = db.execDML("insert into emp values (7, 'David Beckham');");
	cout << nRows << " rows inserted" << endl;

	nRows = db.execDML("update emp set empname = 'Christiano Ronaldo' where empno = 7;");
	cout << nRows << " rows updated" << endl;

	nRows = db.execDML("delete from emp where empno = 7;");
	cout << nRows << " rows deleted" << endl;

	////////////////////////////////////////////////////////////////////////////////
	// Transaction Demo
	// The transaction could just as easily have been rolled back
	////////////////////////////////////////////////////////////////////////////////
	int nRowsToCreate(50000);
	cout << endl << "Transaction test, creating " << nRowsToCreate;
	cout << " rows please wait..." << endl;
	tmStart = time(0);
	db.execDML("begin transaction;");

	for (i = 0; i < nRowsToCreate; i++)
	{
		char buf[128];
		sprintf_s(buf, "insert into emp values (%d, 'Empname%06d');", i, i);
		db.execDML(buf);
	}

	db.execDML("commit transaction;");
	tmEnd = time(0);

	////////////////////////////////////////////////////////////////////////////////
	// Demonstrate CppSQLiteDB::execScalar()
	////////////////////////////////////////////////////////////////////////////////
	cout << db.execScalar("select count(*) from emp;") << " rows in emp table in ";
	cout << tmEnd-tmStart << " seconds (that was fast!)" << endl;

	////////////////////////////////////////////////////////////////////////////////
	// Re-create emp table with auto-increment field
	////////////////////////////////////////////////////////////////////////////////
	cout << endl << "Auto increment test" << endl;
	db.execDML("drop table emp;");
	db.execDML("create table emp(empno integer primary key, empname char(20));");
	cout << nRows << " rows deleted" << endl;

	for (i = 0; i < 5; i++)
	{
		char buf[128];
		sprintf_s(buf, "insert into emp (empname) values ('Empname%06d');", i+1);
		db.execDML(buf);
		cout << " primary key: " << (int)db.lastRowId() << endl;
	}

	////////////////////////////////////////////////////////////////////////////////
	// Query data and also show results of inserts into auto-increment field
	////////////////////////////////////////////////////////////////////////////////
	cout << endl << "Select statement test" << endl;
	CppSQLite3Query q = db.execQuery("select * from emp order by 1;");

	for (fld = 0; fld < q.numFields(); fld++)
	{
		cout << q.fieldName(fld) << "(" << q.fieldDeclType(fld) << ")|";
	}
	cout << endl;

	while (!q.eof())
	{
		cout << q.fieldValue(0) << "|";
		cout << q.fieldValue(1) << "|" << endl;
		q.nextRow();
	}


	////////////////////////////////////////////////////////////////////////////////
	// SQLite's printf() functionality. Handles embedded quotes and NULLs
	////////////////////////////////////////////////////////////////////////////////
	cout << endl << "SQLite sprintf test" << endl;
	CppSQLite3Buffer bufSQL;
	bufSQL.format("insert into emp (empname) values (%Q);", "He's bad");
	cout << (const char*)bufSQL << endl;
	db.execDML(bufSQL);

	bufSQL.format("insert into emp (empname) values (%Q);", NULL);
	cout << (const char*)bufSQL << endl;
	db.execDML(bufSQL);

	////////////////////////////////////////////////////////////////////////////////
	// Fetch table at once, and also show how to use CppSQLiteTable::setRow() method
	////////////////////////////////////////////////////////////////////////////////
	cout << endl << "getTable() test" << endl;
	CppSQLite3Table t = db.getTable("select * from emp order by 1;");

	for (fld = 0; fld < t.numFields(); fld++)
	{
		cout << t.fieldName(fld) << "|";
	}
	cout << endl;
	for (int row = 0; row < t.numRows(); row++)
	{
		t.setRow(row);
		for (int fld = 0; fld < t.numFields(); fld++)
		{
			if (!t.fieldIsNull(fld))
				cout << t.fieldValue(fld) << "|";
			else
				cout << "NULL" << "|";
		}
		cout << endl;
	}


	////////////////////////////////////////////////////////////////////////////////
	// Test CppSQLiteBinary by storing/retrieving some binary data, checking
	// it afterwards to make sure it is the same
	////////////////////////////////////////////////////////////////////////////////
	cout << endl << "Binary data test" << endl;
	db.execDML("create table bindata(desc char(10), data blob);");

	unsigned char bin[256];
	CppSQLite3Binary blob;

	for (i = 0; i < sizeof bin; i++)
	{
		bin[i] = i;
	}

	blob.setBinary(bin, sizeof bin);

	bufSQL.format("insert into bindata values ('testing', %Q);", blob.getEncoded());
	db.execDML(bufSQL);
	cout << "Stored binary Length: " << sizeof bin << endl;

	q = db.execQuery("select data from bindata where desc = 'testing';");

	if (!q.eof())
	{
		blob.setEncoded((unsigned char*)q.fieldValue("data"));
		cout << "Retrieved binary Length: " << blob.getBinaryLength() << endl;
	}
	q.finalize();

	const unsigned char* pbin = blob.getBinary();
	for (i = 0; i < sizeof bin; i++)
	{
		if (pbin[i] != i)
		{
			cout << "Problem: i: ," << i << " bin[i]: " << pbin[i] << endl;
		}
	}


	////////////////////////////////////////////////////////////////////////////////
	// Pre-compiled Statements Demo
	////////////////////////////////////////////////////////////////////////////////
	cout << endl << "Transaction test, creating " << nRowsToCreate;
	cout << " rows please wait..." << endl;
	db.execDML("drop table emp;");
	db.execDML("create table emp(empno int, empname char(20));");
	tmStart = time(0);
	db.execDML("begin transaction;");

	CppSQLite3Statement stmt = db.compileStatement("insert into emp values (?, ?);");
	for (i = 0; i < nRowsToCreate; i++)
	{
		char buf[16];
		sprintf_s(buf, "EmpName%06d", i);
		stmt.bind(1, i);
		stmt.bind(2, buf);
		stmt.execDML();
		stmt.reset();
	}

	db.execDML("commit transaction;");
	tmEnd = time(0);

	cout << db.execScalar("select count(*) from emp;") << " rows in emp table in ";
	cout << tmEnd-tmStart << " seconds (that was even faster!)" << endl;
	cout << endl << "End of tests" << endl;
}

/*ACE::init();
SFDatabaseProxy* pProxyLocal = new SFDatabaseProxyLocal();
SFDatabaseProxy* pProxy = new SFDatabaseProxyImpl(pProxyLocal);
pProxy->Initialize();

SFObjectPool<SFMessage> Pool(100);
for(int i= 0; i < 100; i++)
{
SFMessage* pMessage = Pool.Alloc();
pProxy->SendDBRequest(pMessage);
}

getchar();
ACE::fini();*/



/*
	SFFacade Sys;
	Sys.Initialize();

	SFObjectPool<int> Pool(100);

	SFIni ini;
	WCHAR szServerName[20];
	int Ver;

	ini.SetPathName(_T("./Test2.ini"));
	ini.GetString(L"ServerInfo",L"Name",szServerName, 20);
	Ver = ini.GetInt(L"ServerInfo",L"Ver",1);

	

///////////////////////////////////////////////////////////////////////////////////////////
//패킷 자르기 테스트
///////////////////////////////////////////////////////////////////////////////////////////

	SFCompress* pCompress = new SFCompressZLib();
	//SFCompress* pCompress = new SFCompressLzf();

	BYTE szJuhang[1025] = "juhang3";
	int SrcSize = 1025;

	SFPacket PacketSend;
	PacketSend.MakePacket(pCompress, (BYTE*)szJuhang, SrcSize, PACKET_OPTION_DATACRC | PACKET_OPTION_COMPRESS);


///////////////////////////////////////////////////////////////////////////////////////////
//서버에서 데이터를 받음
///////////////////////////////////////////////////////////////////////////////////////////
	SFPacketAnalyzer Analyzer(pCompress);

	Analyzer.AddTransferredData((char*)PacketSend.GetHeader(), sizeof(SFPacketHeader));
	Analyzer.AddTransferredData((char*)PacketSend.GetDataBuffer(), PacketSend.GetDataSize());

	SFPacket PacketRecv;

	int ErrorCode = 0;
	Analyzer.GetPacket(PacketRecv, ErrorCode);

	printf("%s %d\n", PacketRecv.GetDataBuffer(), PacketRecv.GetDataSize());

	SFEncryptionXOR Enc;
	Enc.Encrypt(szJuhang, 8);
	Enc.Decrypt(szJuhang, 8);

	SFPacketHandler* pPacketHandler = new SFPacketHandler();

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
	SFDispatch<USHORT, std::tr1::function<void(protobuf::io::ArrayInputStream&)>, protobuf::io::ArrayInputStream> DispatchingSystem;

	DispatchingSystem.RegisterMessage(5, std::tr1::bind(&SFPacketHandler::OnAuth, pPacketHandler, std::tr1::placeholders::_1));

	
	//DispatchingSystem.RegisterMessage(5, sampleFunc);


	protobuf::io::ArrayInputStream a(NULL, 0);

	DispatchingSystem.HandleMessage(5, a);

	SFPacketStore::Auth PktAuth;
	PktAuth.set_encryptionkey(0x12345678);

	BYTE pBuff[4096] = {0,};
	int BuffSize = PktAuth.ByteSize();

	protobuf::io::ArrayOutputStream os(pBuff, BuffSize);
	PktAuth.SerializeToZeroCopyStream(&os);
	
	
	SSNickName szName;
	szName = _T("Juhang");

	SFFastCRC FastCrc;
	FastCrc.Initialize();
	DWORD crc;

	FastCrc.GetCRC(szJuhang, 8, crc);
	printf("The crcFast() of \"123456789\" is 0x%X\n", crc);

	SFMessage Message;
	SFFString String("juhang3");
	String.convert(SFFSTRType_Unicode16);
	wprintf(L"%s\n", String.getUnicode16());

	SFRegistry Reg(HKEY_LOCAL_MACHINE);

	if(TRUE == Reg.CreateRegistryKey(L"SOFTWARE\\JUHANG"))
	{
		DWORD dwValue;

		Reg.SetValue(L"TEST1", 555);
		Reg.GetValue(L"TEST1", dwValue);
		Reg.DeleteValue(L"TEST1");
		Reg.DeleteKey(L"SOFTWARE\\JUHANG");
	}

/////////////////////////////////////////////////////////////////////////////////////////////
//DB Test
/////////////////////////////////////////////////////////////////////////////////////////////
	

	ACE::init();

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	//SFNetworkHandler Handler(&LockDeque);
	//Handler.activate(THR_NEW_LWP | THR_JOINABLE, 5);

	int OptimalThreadCount =si.dwNumberOfProcessors * 2;

	DWORD dwTime = timeGetTime();
	int GroupID = ACE_Thread_Manager::instance()->spawn_n(OptimalThreadCount, (ACE_THR_FUNC)FakeWorkThread, &LockDeque, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1);

	if(GroupID == -1)
	{
		SFASSERT(0);
	}

	if(ACE_Thread_Manager::instance()->spawn_n(1, (ACE_THR_FUNC)BusinessThread, &LockDeque, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2) == -1)
	{
		SFASSERT(0);
	}

	ACE_Thread_Manager::instance()->wait_grp(GroupID);
	//ACE_Thread_Manager::instance()->wait_grp(Handler.grp_id());

	//Handler.wait();

	DWORD dwCompleteTime = timeGetTime();

	DWORD dwElapsed = dwCompleteTime - dwTime;

	printf("ElapsedTime %d\n", dwElapsed);

	ACE_Thread_Manager::instance()->wait_grp(2);

	ACE::fini();

	delete pPacketHandler;
	delete pCompress;
	Sys.Finally();
	*/


//#include "config.h"
/*
#include <stdio.h>
#include <string.h>     
#include <cassert>
#include <vector>
#include "pcrecpp.h"
using namespace std;

#ifdef _DEBUG
#pragma comment(lib, "pcred.lib")
#pragma comment(lib, "pcrecppd.lib")
#else
#pragma comment(lib, "pcre.lib")
#pragma comment(lib, "pcrecpp.lib")
#endif
	*/
/*
void PCRETest()
{
	int i;
	string s;
	pcrecpp::RE re("(\\w+):(\\d+)");
	if (re.error().length() > 0) {
		cout << "PCRE compilation failed with error:" << re.error() << "\n";
	}
	if (re.PartialMatch("root:1234", &s, &i))
		cout << s << " : " << i << "\n";
}*/
/*
#include "SFTree.h"
#include "ace/os_memory.h"

void ACEDataStructureTest()
{
	SFTree<int, int*> Tree_;
	for(int i = 0; i < 100; i++)
	{
		int* d = 0;
		ACE_NEW_NORETURN(d, int);
		*d = i;
		Tree_.Add(i, d);
	}

	int *q = 0;

	q = Tree_.Find(49);

	for(int i = 0; i < 100; i++)
	{
		int *h = Tree_.Remove(i);
	}

	
}*/

void LokiTest()
{

}
/*
#pragma comment(lib, "liblfds.lib")

static void PushThread(void* Args)
{
	

	struct stack_state* pStack = (struct stack_state*)Args;

	for(int i = 0; i < 3000; i++)
	{
		int* p = new int;
		*p = i;
		
		//stack_push(pStack, p);
		stack_guaranteed_push(pStack, p);
		
	}
}

static void PopThread(void* Args)
{
	struct stack_state* pStack = (struct stack_state*)Args;

	while(1)
	{
		int* t = NULL;
		int i = stack_pop(pStack, (void**)&t);

		if(t != NULL)
		{
			printf("pop success %d\n", GetCurrentThreadId());
			delete t;
		}
		else
		{
			//printf("pop fail\n");
		}
	}
}

void libfdsTest()
{
	struct stack_state* pStack = NULL;
	stack_new(&pStack, 10000);

	ACE::init();

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	int OptimalThreadCount =si.dwNumberOfProcessors;

	DWORD dwTime = timeGetTime();
	int GroupID = ACE_Thread_Manager::instance()->spawn_n(OptimalThreadCount, (ACE_THR_FUNC)PushThread, pStack, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1);

	if(GroupID == -1)
	{
		SFASSERT(0);
	}

	if(ACE_Thread_Manager::instance()->spawn_n(OptimalThreadCount, (ACE_THR_FUNC)PopThread, pStack, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 2) == -1)
	{
		SFASSERT(0);
	}

	ACE_Thread_Manager::instance()->wait_grp(GroupID);
	//ACE_Thread_Manager::instance()->wait_grp(Handler.grp_id());

	//Handler.wait();

	DWORD dwCompleteTime = timeGetTime();

	DWORD dwElapsed = dwCompleteTime - dwTime;

	printf("ElapsedTime %d\n", dwElapsed);

	ACE_Thread_Manager::instance()->wait_grp(2);

	ACE::fini();
}*/

std::string GetPluginDirectory ( void )
{
    char fullfilename[_MAX_PATH];
    ::GetModuleFileNameA ( NULL,
	                      fullfilename,
                          _MAX_PATH );
    
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char filename[_MAX_FNAME];
    char ext[_MAX_EXT];
    ::_splitpath_s ( fullfilename, drive, dir, filename, ext );

    return std::string(drive) + std::string(dir) + std::string("plugins");
}