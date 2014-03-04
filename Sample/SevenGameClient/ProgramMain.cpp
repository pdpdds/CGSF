#include "StdAfx.h"
#include "ProgramMain.h"
#include "DirectXSystem.h"

CProgramMain::CProgramMain(void)
{
}

CProgramMain::~CProgramMain(void)
{
}

int CProgramMain::Run()
{
	return CDirectXSystem::GetInstance()->Run();
}

BOOL CProgramMain::Initialize()
{
	CDirectXSystem::GetInstance()->Initialize();

	return 0;
}
