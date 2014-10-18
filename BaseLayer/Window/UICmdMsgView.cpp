#include "stdafx.h"
#include "UICmdMsgView.h"
#include "UICmdEdit.h"

HWND UICmdMsgView::s_hInfoWnd = NULL;
HWND UICmdMsgView::s_hListWnd = NULL;

LRESULT UICmdMsgView::OnVkeyToItem( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) 
{
	if( (HWND )lParam == s_hListWnd ) 
	{
		if(LOWORD( wParam ) == VK_TAB )
			SetFocus( GetParent(s_hListWnd) );
	}
	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

VOID UICmdMsgView::ClearMsgView( void )
{
	SendMessage(s_hListWnd, LB_RESETCONTENT, 0, 0);
}

VOID UICmdMsgView::UpdateInfo(WCHAR* strInfo)
{
	SendMessage(s_hInfoWnd, WM_SETTEXT, MAX_EDITSTRING, (LPARAM)strInfo);
}

HWND UICmdMsgView::InitMsgView( HWND hParent, HFONT hFont ) 
{
	RECT rect;
	GetClientRect( hParent, &rect );	

#ifdef _WIN64
	s_hInfoWnd = CreateWindowW(L"EDIT", L"Infomation",
		WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL,
		0, 0, rect.right, INFO_HEIGHT,
		hParent, (HMENU)ID_INFO,
		(HINSTANCE)GetWindowLongPtr(hParent, GWLP_HINSTANCE), NULL);
#else
	s_hInfoWnd = CreateWindowW( L"EDIT", L"Infomation",
		WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL,
		0, 0, rect.right, INFO_HEIGHT,
		hParent, (HMENU )ID_INFO,
		(HINSTANCE )GetWindowLong( hParent, GWL_HINSTANCE ), NULL );
#endif
	ShowWindow( s_hInfoWnd, SW_SHOW );
	UpdateWindow( s_hInfoWnd );
	
	::SendMessage(s_hInfoWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE,0));

#ifdef _WIN64
	s_hListWnd = CreateWindowW( L"LISTBOX", L"Message List",
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_BORDER | WS_VSCROLL
		| LBS_DISABLENOSCROLL | LBS_NOINTEGRALHEIGHT | LBS_WANTKEYBOARDINPUT | LBS_USETABSTOPS,
		0, INFO_HEIGHT, rect.right, rect.bottom - EDIT_HEIGHT - INFO_HEIGHT,
		hParent, (HMENU )ID_MSGVIEW,
		(HINSTANCE )GetWindowLongPtr( hParent, GWLP_HINSTANCE ),
		NULL );
#else
	s_hListWnd = CreateWindowW( L"LISTBOX", L"Message List",
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_BORDER | WS_VSCROLL
		| LBS_DISABLENOSCROLL | LBS_NOINTEGRALHEIGHT | LBS_WANTKEYBOARDINPUT | LBS_USETABSTOPS,
		0, INFO_HEIGHT, rect.right, rect.bottom - EDIT_HEIGHT - INFO_HEIGHT,
		hParent, (HMENU )ID_MSGVIEW,
		(HINSTANCE )GetWindowLong( hParent, GWL_HINSTANCE ),
		NULL );
#endif

	ShowWindow( s_hListWnd, SW_SHOW );
	UpdateWindow( s_hListWnd );
	
	::SendMessage(s_hListWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE,0));	

	return s_hListWnd;
}

VOID UICmdMsgView::ResizeLog( HWND hParent )
{
	RECT r;
	GetClientRect( hParent, &r );

	MoveWindow( s_hInfoWnd, 0, 0, r.right, INFO_HEIGHT, TRUE );
	MoveWindow( s_hListWnd, 0, INFO_HEIGHT, r.right, r.bottom - EDIT_HEIGHT - INFO_HEIGHT, TRUE );
}
