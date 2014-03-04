// WindowServiceTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include "SFServiceController.h"

#pragma comment(lib, "BaseLayer.lib")


int _tmain(int argc, _TCHAR* argv[])
{
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
	else
	{
		#ifdef _DEBUG
	//SFFacade Facade;
	//Facade.Initialize();
#else
	SFServiceController Controller;
	Controller.ServiceEntry(L"CGSF");
#endif
	}

	return 0;
}

