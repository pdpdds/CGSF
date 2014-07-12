// SevenGame.cpp : Defines the entry point for the application.

#include "stdafx.h"
#include "SevenGame.h"
#include "ProgramMain.h"

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")

#ifdef _DEBUG
#pragma comment(lib, "d3dx9d.lib")
#else
#pragma comment(lib, "d3dx9.lib")
#endif

#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "dsound.lib")

          
int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{

	CProgramMain* pMain = new CProgramMain();
	pMain->Initialize();

	int iRet = pMain->Run();

	delete pMain;

	return iRet;
}