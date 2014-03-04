// SevenGame.cpp : Defines the entry point for the application.

#include "stdafx.h"
//#include "dxstdafx.h"
#include "SevenGame.h"
#include "ProgramMain.h"


// TODO: reference additional headers your program requires here
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
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "dsound.lib")

//#pragma comment(lib, "lua.lib")
//#pragma comment(lib, "OrangeDump.lib")
//#pragma comment(lib, "OrangeIni.lib")
          
int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	/*CEHModuleManager ModuleManger;

	if(FALSE == ModuleManger.Install(DL_MY_HANDLER_STACKOVERFLOW, EH_BUGTRAP_MIX))
	{
		MessageBox(NULL, _T("예외 핸들러 설치 실패"), _T("Seven Game"), MB_OK);
		return 0;
	}*/


	CProgramMain* pMain = new CProgramMain();
	pMain->Initialize();

	int iRet = pMain->Run();

	delete pMain;

	return iRet;
}