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
#include "SFFastCRC.h"
//#include "SFPacketStore.pb.h"
//#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "SFPacketHandler.h"
#include "SFDispatch.h"
#include "SFMessage.h"
#include "SFObjectPool.h"
//#include "SFNetworkHandler.h"
#include <map>
#include "SFServiceController.h"
#include "SFSampleDB.h"
#include "SFFilePack.h"
#include "CPUDesc.h"
//#include "SFProactiveServer.h"




#include <fstream>
#include <iostream> 
#include <sstream>  

using namespace std;

#pragma comment(lib, "enginelayer.lib")
#pragma comment(lib, "baselayer.lib")
#pragma comment(lib, "databaselayer.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "liblzf.lib")
#pragma comment(lib, "libprotobuf.lib")
#ifdef _DEBUG
#pragma comment(lib, "aced.lib")
#else
#pragma comment(lib, "ace.lib")
#endif



/*void sampleFunc(protobuf::io::ArrayInputStream& input)
{
	return;
}*/



void DatabaseTest();
void ServiceTest(int argc, TCHAR* argv[]);
void FileMemoryPackingTest();
void DataStructureTest();
void AceLoggerTest();
void AceServerTest();
void SQLiteTest();
void PCRETest();
void ACEDataStructureTest();

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

int _tmain(int argc, _TCHAR* argv[])
{
	LockQueueTest test;
	test.Run();

	

	//AceLoggerTest();

	//ACEDataStructureTest();

	

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
//Database Test
//////////////////////////////////////////////////////////////////////////
	//DatabaseTest();	


	return 0;
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

void FileMemoryPackingTest()
{
	SFFilePack FilePack;
	if(FilePack.Initialize() == TRUE)	
		FilePack.UnPackFile(_T("test.zip"));
}

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
	
	
	

	SFFastCRC FastCrc;
	FastCrc.Initialize();
	DWORD crc;

	FastCrc.GetCRC(szJuhang, 8, crc);
	printf("The crcFast() of \"123456789\" is 0x%X\n", crc);

	SFMessage Message;
		

	

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