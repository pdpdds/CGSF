#include "stdafx.h"
#include "SFExcelTest.h"
#include "SFExcel.h"

SFExcelTest::SFExcelTest(void)
{
}


SFExcelTest::~SFExcelTest(void)
{
}

bool SFExcelTest::Run()
{
	SFExcel Excel;
	Excel.Initialize();
	Excel.Read("test.xls", 0);
	Excel.Finally();

	return true;
}
