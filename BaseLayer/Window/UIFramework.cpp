#include "stdafx.h"
#include "UIFramework.h"
#include "SFUtil.h"

#pragma warning(disable : 4100) 

using namespace SFUtil;

HWND UIWindow::m_logHwnd;

void AddLogMsg(HWND hWnd, WCHAR* String, int Len)
{
	// 삭제
	int iCount = (int)SendMessageW(hWnd, LB_GETCOUNT, 0, 0L) - 128;
	for (int i = 0; i < iCount; ++i)
		SendMessageW(hWnd, LB_DELETESTRING, 0, 0L);

	// 추가
	WCHAR* p;
	int k;
	p = String;

	int iAdd = 0;

	k = 0;
	while (k++ < Len)
	{
		switch (*String)
		{
		case L'\n':
			*String = 0;
			++iAdd;
			SendMessageW(hWnd, LB_ADDSTRING, 0, (LPARAM)(LPWSTR)p);
			p = ++String;
			break;
		default:
			++String;
		}
	}

	if (*p)
	{
		++iAdd;
		SendMessageW(hWnd, LB_ADDSTRING, 0, (LPARAM)(LPWSTR)p);
	}

	int Top = (int)SendMessageW(hWnd, LB_GETTOPINDEX, 0, 0L);
	SendMessageW(hWnd, LB_SETTOPINDEX, Top + iAdd, 0L);
}

void LOG(const WCHAR* msg, ...)
{
	va_list v;
	WCHAR buf[1024];
	int len;

	va_start(v, msg);
	len = vswprintf_s(buf, msg, v);

	va_end(v);

	AddLogMsg(UIWindow::m_logHwnd, buf, (int)wcslen(msg));
}

//////////////////////////////////////
UIConsol::UIConsol()
{
	m_bCommandExit = FALSE;
}

UIConsol::~UIConsol()
{
}

BOOL UIConsol::Init()
{
	//CLog::SetLogHandle(UICmdMsgView::s_hListWnd);
	//		Nave::NFLog::EnableLogFile(true);

	// 오브젝트 초기화
	InitObject();

	// 커멘드 초기화.
	InitializeCommand();

	m_bCommandExit = FALSE;
	return TRUE;
}

VOID UIConsol::StartCommand()
{
	wprintf(L"\n");
	WCHAR Command[1024];
	while(!m_bCommandExit)
	{
		_getws_s(Command);

		DoCommand(Command);
	}

	ReleaseObject();
}

VOID UIConsol::EndCommand()
{
	m_bCommandExit = TRUE;
}

static UIWindow* g_pMain = NULL;
///////////////////////////////////////////////////////////////////////////////
//	Descrip	: 말그대로 윈도우용. Proc..
//	Date	: 2001-04-11오전 10:41:02
///////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(g_pMain)
		return g_pMain->MsgProc(hWnd, uMsg, wParam, lParam);

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

UIWindow::UIWindow()
{
	m_bCommandExit = FALSE;
	m_hFont = NULL;
	g_pMain = this;
}

UIWindow::~UIWindow()
{
	g_pMain = NULL;

	if(m_hFont)
		DeleteObject(m_hFont);
	m_hFont = NULL;
}

VOID UIWindow::OnEditReturn(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WCHAR buff[MAX_EDITSTRING];
	ZeroMemory(buff, sizeof(buff));
	CopyMemory( buff, (WCHAR*)lParam, sizeof(buff) );

	// 여기서 입력된 문자열을 가지고 분석해서 입력한 값에 따라 처리한다.
	DoCommand(buff);
}

VOID UIWindow::ResizeWindows( VOID ) 
{
	UICmdEdit::ResizeEdit(m_hMainWnd);
	UICmdMsgView::ResizeLog(m_hMainWnd);
}

///////////////////////////////////////////////////////////////////////////////
//	Descrip	: 말그대로 사용할 MsgProc , WindowProc에서 호출된다.
//	Date	: 2001-04-11오전 10:41:02
///////////////////////////////////////////////////////////////////////////////
LRESULT UIWindow::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch(uMsg)
	{
	case WM_SETFOCUS:
		SetFocus( UICmdEdit::s_hEditWnd );
		break;

	case WM_ERASEBKGND:
		return FALSE;

	case WM_SIZE:
		ResizeWindows();
		return FALSE;

	case WM_COMMAND:
		OnCommand(hWnd, LOWORD(wParam), HIWORD(wParam), lParam);
		return FALSE;

		// ListView 키보드 입력.
	case WM_VKEYTOITEM :
		return UICmdMsgView::OnVkeyToItem(hWnd, uMsg, wParam, lParam);

		// EditBox 엔터 입력.
	case WM_EDIT_RETURN:
		OnEditReturn(hWnd, uMsg, wParam, lParam);
		return FALSE;

	case WM_CLOSE:
	case WM_DESTROY:
	case WM_QUIT:
		EndCommand();
		return FALSE;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

BOOL UIWindow::Init(HINSTANCE hInstance, int nCmdShow, int iWidth, int iHeight, WCHAR* AppName, WCHAR* Icon)
{
	// 윈도우의 생성
	WNDCLASSEX wcex;
	wcex.cbSize			= sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc	= (WNDPROC)WindowProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 16;
	wcex.hInstance		= (HINSTANCE)hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)Icon);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= AppName;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, Icon);
	// 레지등록.
	RegisterClassExW(&wcex);

	if(iHeight < INFO_HEIGHT+150)
	{
		iHeight = INFO_HEIGHT+150;
	}

	// 윈도우 생성
	m_hMainWnd = CreateWindowExW(WS_EX_WINDOWEDGE, AppName, AppName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, iWidth, iHeight, 
		NULL, NULL, hInstance, NULL);

	if(!m_hMainWnd)
	{
		MessageBoxW(NULL, L"윈도우 생성 에러", L"Error", MB_OK);
		return FALSE;
	}

	m_hFont = ::CreateFontW(12, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 3, 2, 1, VARIABLE_PITCH|FF_ROMAN, L"굴림체");

	UICmdEdit::InitEditProc(m_hMainWnd, m_hFont);
	UICmdMsgView::InitMsgView(m_hMainWnd, m_hFont);

	SetLogHandle(UICmdMsgView::s_hListWnd);
	//		Nave::NFLog::EnableLogFile(true);

	ShowWindow(nCmdShow);					// Show The UIWindow
	SetForegroundWindow(m_hMainWnd);					// Slightly Higher Priority
	// EditBox에 포커스 주기
	SetFocus(UICmdEdit::s_hEditWnd);								// Sets Keyboard Focus To The UIWindow

	// 아이콘 로드 (작은 것 따로 하나)
	HICON hIcon = LoadIcon( hInstance, (LPCTSTR)Icon );
	SendMessage( m_hMainWnd, WM_SETICON, (WPARAM)ICON_BIG,   (LPARAM)hIcon );
	SendMessage( m_hMainWnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon );

	// 오브젝트 초기화
	InitObject();

	// 커멘드 초기화.
	InitializeCommand();

	m_bCommandExit = FALSE;
	return TRUE;
}

VOID UIWindow::ShowWindow(int nCmdShow)
{
	::ShowWindow(m_hMainWnd, nCmdShow);					// Show The UIWindow
}

VOID UIWindow::StartCommand()
{
	MSG		msg;

	while( !m_bCommandExit ) 
	{ 
		if( GetMessage( &msg, NULL, 0U, 0U ) ) 
		{ 
			TranslateMessage( &msg ); 
			DispatchMessage( &msg ); 
		} 
	}

	LOG((L"Programe Quitting"));

	ReleaseObject();

	InvalidateRect(m_hMainWnd, NULL, FALSE);

	// 종료 메시지 표현 해 주기 위해서
	for(int i = 0; i < 10; i++)
	{
		// 마지막 메시지를 화면에 출력하기 위해서.
		if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) ) 
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	Sleep(1000);
}

VOID UIWindow::EndCommand()
{
	m_bCommandExit = TRUE;
	InvalidateRect(m_hMainWnd, NULL, FALSE);
}


BOOL UICommandFactory::AddCommand( const WCHAR* szCommand, UICommand* lpCommand, const WCHAR* szMessage )
{
	if(0 == szCommand, 0 == lpCommand)
		return FALSE;    

	m_CMDVector.push_back(COMMAND(szCommand, lpCommand, szMessage));
	return TRUE;
}

VOID UICommandFactory::ReleaseCommand()
{
	int iSize = (int)m_CMDVector.size();
	for(int i = 0; i < iSize; ++i)
	{
		delete m_CMDVector[i].m_lpCMD;
	}

	m_CMDVector.clear();
}

VOID UICommandFactory::ShowCommand()
{
	LOG( (L"------------------- Commands -------------------") );
	int iSize = (int)m_CMDVector.size();
	for(int i = 0; i < iSize; ++i)
	{
		LOG((L"%s : %s", m_CMDVector[i].m_szCMD, m_CMDVector[i].m_szMSG));
	}
	LOG((L"----------------- Commands End -----------------"));
}

VOID UICommandFactory::DoCommand( WCHAR* Command )
{
	WCHAR Buff[1024];
	ZeroMemory(Buff, 1024);
	wcscpy_s(Buff, Command);

	WCHAR* cmd;
	cmd = wcstok(Command, L" ");
	if(cmd)
	{
		UINT uiHash = Hash(cmd);
		WCHAR* NextCmd = &Buff[wcslen(cmd)+1];

		int iSize = (int)m_CMDVector.size();
		for(int i = 0; i < iSize; ++i)
		{
			if(m_CMDVector[i].m_uiHash != uiHash)
				continue;

			m_CMDVector[i].m_lpCMD->DoProcess(NextCmd);
			return;
		}
	}
}

UICommandFactory::COMMAND::COMMAND( const WCHAR* szCommand, UICommand* lpCMD, const WCHAR* szMessage )

{
	m_uiHash = Hash(szCommand);

	wcscpy_s(m_szCMD, szCommand);

	if(szMessage)
		wcscpy_s(m_szMSG, szMessage);
	else
		wcscpy_s(m_szMSG, L"No Help Message");

	m_lpCMD = lpCMD;
}