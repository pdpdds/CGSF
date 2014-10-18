#include "stdafx.h"
#include "UICmdEdit.h"
#include "UICmdMsgView.h"

HWND UICmdEdit::s_hEditWnd = NULL;
CHAR UICmdEdit::s_EditStr[MAX_EDITSTRING];
WNDPROC UICmdEdit::s_OldEditProc;

LRESULT UICmdEdit::OnEditDefault( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return CallWindowProc( s_OldEditProc, hWnd, uMsg, wParam, lParam );
}

LRESULT UICmdEdit::OnEditKeyDown( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( wParam ) 
	{
	case VK_TAB:		
		SetFocus(UICmdMsgView::s_hListWnd);
		return FALSE;
	}
	return OnEditDefault(hWnd, uMsg, wParam, lParam);
}

LRESULT UICmdEdit::OnEditChar( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( wParam ) 
	{
	case 13:	// EditBox에서 return을 했을때
		SendMessage( hWnd, WM_GETTEXT, MAX_EDITSTRING, (LPARAM)(LPSTR )s_EditStr);
		if(strlen(s_EditStr) == 0)
			break;		
		SendMessage( hWnd, EM_SETSEL, 0, (LPARAM )-1 );		
		SendMessage( GetParent(hWnd), WM_EDIT_RETURN, strlen(s_EditStr), (LPARAM)(LPSTR )s_EditStr );
		return FALSE;
	}
	return OnEditDefault(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK UICmdEdit::EditProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ) 
	{
	case WM_KEYDOWN:
		return OnEditKeyDown( hWnd, uMsg, wParam, lParam );
	case WM_CHAR :
		return OnEditChar( hWnd, uMsg, wParam, lParam );
	default :
		return OnEditDefault( hWnd, uMsg, wParam, lParam );
	}
}

HWND UICmdEdit::InitEditProc( HWND hParent, HFONT hFont )
{
	ZeroMemory(s_EditStr, sizeof(s_EditStr));

	RECT rect;
	GetClientRect( hParent, &rect );

#ifdef _WIN64
	s_hEditWnd = CreateWindowW( L"EDIT", L"",
		WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_VISIBLE | WS_BORDER | ES_WANTRETURN | ES_AUTOHSCROLL,
		0, rect.bottom-EDIT_HEIGHT, rect.right, EDIT_HEIGHT,
		hParent, (HMENU )ID_EDIT,
		(HINSTANCE )GetWindowLongPtr( hParent, GWLP_HINSTANCE ), NULL );
#else
	s_hEditWnd = CreateWindowW(L"EDIT", L"",
		WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_VISIBLE | WS_BORDER | ES_WANTRETURN | ES_AUTOHSCROLL,
		0, rect.bottom - EDIT_HEIGHT, rect.right, EDIT_HEIGHT,
		hParent, (HMENU)ID_EDIT,
		(HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE), NULL);
#endif
	ShowWindow( s_hEditWnd, SW_SHOW );
	UpdateWindow( s_hEditWnd );
	
	::SendMessage(s_hEditWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE,0));

#ifdef _WIN64
	s_OldEditProc = (WNDPROC )GetWindowLongPtr( s_hEditWnd, GWLP_WNDPROC );
	SetWindowLongPtr( s_hEditWnd, GWLP_WNDPROC, (LONG )EditProc );
#else
	s_OldEditProc = (WNDPROC)GetWindowLong(s_hEditWnd, GWL_WNDPROC);
	SetWindowLong(s_hEditWnd, GWL_WNDPROC, (LONG)EditProc);
#endif

	return s_hEditWnd;
}

VOID UICmdEdit::ResizeEdit( HWND hParent )
{
	RECT r;
	GetClientRect( hParent, &r );

	MoveWindow( s_hEditWnd, 0, r.bottom-EDIT_HEIGHT, r.right, EDIT_HEIGHT, TRUE );
}