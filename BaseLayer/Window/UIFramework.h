#pragma once

#include "UICmdEdit.h"
#include "UICmdMsgView.h"
#include <vector>


#define ADD_COMMAND(cmd, object, msg) AddCommand(cmd, new object, msg)

class UICommand
{
public:
	virtual BOOL DoProcess(WCHAR* lpParam) = 0;    
};

class UICommandFactory
{
public:
	BOOL AddCommand(const WCHAR* szCommand, UICommand* lpCommand, const WCHAR* szMessage);


	VOID ReleaseCommand();
	

	virtual VOID	InitializeCommand() {};

	virtual VOID	StartCommand() {};
	virtual VOID	EndCommand() {};

	VOID ShowCommand();


	VOID DoCommand(WCHAR* Command);
	

public:
	UICommandFactory()
	{
	}
	~UICommandFactory()
	{
		ReleaseCommand();
	}

private:
	struct COMMAND
	{
		UINT	m_uiHash;
		WCHAR			m_szCMD[32];
		WCHAR			m_szMSG[128];
		UICommand*		m_lpCMD;

		COMMAND(const WCHAR* szCommand, UICommand* lpCMD, const WCHAR* szMessage);
		
	};

	typedef std::vector<COMMAND>	CMDVector;
	CMDVector						m_CMDVector;
};

class UIConsol : public UICommandFactory
{
	BOOL			m_bCommandExit;
public:	
	virtual VOID	InitObject() {};	
	virtual VOID	ReleaseObject() {};

	BOOL			Init();

	virtual VOID	InitializeCommand() {};
	virtual VOID	StartCommand();	
	virtual VOID	EndCommand();

public:
	UIConsol(VOID);
	~UIConsol(VOID);
};

class UIWindow : public UICommandFactory
{
public:
	HWND			GetWnd() { return m_hMainWnd; }
	BOOL			IsExit() { return m_bCommandExit; }
	
	virtual VOID	InitObject() {};	
	virtual VOID	ReleaseObject() {};

	virtual VOID	ShowWindow(int nCmdShow);
	
	virtual LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	BOOL			Init(HINSTANCE hInstance, int nCmdShow, int iWidth, int iHeight, WCHAR* AppName, WCHAR* Icon);
	void SetLogHandle(HWND hWnd){ m_logHwnd = hWnd; }

	virtual VOID	InitializeCommand() {};
	virtual VOID	StartCommand();
	virtual VOID	EndCommand();

public:
	UIWindow(VOID);
	~UIWindow(VOID);

	static HWND			m_logHwnd;

private:
	HWND			m_hMainWnd;
	BOOL			m_bCommandExit;
	HFONT			m_hFont;
	

	VOID			ResizeWindows( VOID );
	
	virtual VOID	OnEditReturn(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual VOID	OnCommand(HWND hWnd, INT nID, INT iEvent, LPARAM lParam) 
	{
		UNREFERENCED_PARAMETER(hWnd); UNREFERENCED_PARAMETER(nID); UNREFERENCED_PARAMETER(iEvent); UNREFERENCED_PARAMETER(lParam);
		return; 
	}
};