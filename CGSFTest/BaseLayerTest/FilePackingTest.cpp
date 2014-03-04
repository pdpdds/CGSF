#include "stdafx.h"
#include "FilePackingTest.h"
#include "SFFilePack.h"

FilePackingTest::FilePackingTest(void)
{
}


FilePackingTest::~FilePackingTest(void)
{
}

//////////////////////////////////////////////////////////////////////////
//파일 패킹, 압축 및 해제(메모리상) 테스트
//////////////////////////////////////////////////////////////////////////
bool FilePackingTest::Run()
{
	SFFilePack FilePack;
	if(FilePack.Initialize() == TRUE)	
		FilePack.UnPackFile(_T("test.zip"));

	return true;
}
