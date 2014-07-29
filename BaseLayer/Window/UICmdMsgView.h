/** 
 *  @file  		UICmdMsgView.h
 *  @brief 		Server UIFramework에서 사용하는 List, Info컨트롤
 *  @remarks 
 *  @author  	강동명(edith2580@gmail.com)
 *  @date  		2009-05-09
 */
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

	static HWND s_hInfoWnd; // 리스트
	static HWND s_hListWnd;	// 리스트

	static HWND InitMsgView( HWND hParent, HFONT hFont );
	static VOID ClearMsgView( void );

	static VOID UpdateInfo(WCHAR* strInfo);

	static VOID ResizeLog( HWND hParent );

	// ListView 입력.
	static LRESULT OnVkeyToItem( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};
