/** 
 *  @file  		UICmdEdit.h
 *  @brief 		Server UIFramework에서 사용하는 Edit컨트롤
 *  @remarks 
 *  @author  	강동명(edith2580@gmail.com)
 *  @date  		2009-05-09
 */
#pragma once

class UICmdEdit 
{
public:
	UICmdEdit();
	~UICmdEdit();

public:
#define MAX_EDITSTRING	1024
#define WM_EDIT_RETURN	(WM_USER+101)
#define EDIT_HEIGHT		16
#define ID_EDIT			200

	static HWND s_hEditWnd;	// 에디트,
	static CHAR s_EditStr[MAX_EDITSTRING];
	static WNDPROC s_OldEditProc;

	static LRESULT OnEditDefault( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static LRESULT OnEditKeyDown( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static LRESULT OnEditChar( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK EditProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static HWND InitEditProc( HWND hParent, HFONT hFont );

	static VOID ResizeEdit( HWND hParent );
};
