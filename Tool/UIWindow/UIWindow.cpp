// UIWindow.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Resource.h"
#include "SampleUIWindow.h"

SampleUIWindow* g_SampleWindow = NULL;

int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR		lpCmdLine,
	int       nCmdShow)
{
	

	g_SampleWindow = new SampleUIWindow();

	if (!g_SampleWindow)
	{
		return 0;
	}

	WCHAR	szTitle[32];
	_stprintf_s(szTitle, L"%s", L"SampleUIWindow");

	if (!g_SampleWindow->Init(hInstance, nCmdShow, 320, 300, szTitle, MAKEINTRESOURCE(IDI_UIWINDOW)))
		return 0;

	TCHAR szPath[MAX_PATH];

	if (!GetModuleFileName(NULL, szPath, MAX_PATH))
	{
		return 0;
	}

	g_SampleWindow->StartCommand();

	delete g_SampleWindow;
	return 0;
}
