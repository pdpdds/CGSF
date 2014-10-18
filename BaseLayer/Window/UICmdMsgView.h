
#pragma once

class UICmdMsgView 
{
public:
	UICmdMsgView();
	~UICmdMsgView();

public:
#define INFO_HEIGHT		300
#define ID_INFO			201
#define ID_MSGVIEW		202

	static HWND s_hInfoWnd;
	static HWND s_hListWnd;

	static HWND InitMsgView( HWND hParent, HFONT hFont );
	static VOID ClearMsgView( void );

	static VOID UpdateInfo(WCHAR* strInfo);

	static VOID ResizeLog( HWND hParent );
	
	static LRESULT OnVkeyToItem( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};
