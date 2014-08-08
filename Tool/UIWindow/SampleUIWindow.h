#pragma once
#include "Window/UIFramework.h"

class SampleUIWindow : public UIWindow
{
public:
	SampleUIWindow(void);
	virtual ~SampleUIWindow(void);

	long						m_uTimerID;
	long						m_uTimerPeriod;
	static void __stdcall		CALLBACK	TimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

	VOID						Update();									// Process 처리 함수

	virtual VOID				InitializeCommand();

	virtual VOID				InitObject();
	virtual VOID				ReleaseObject();

public:	
	VOID						UpdateInfo();
	VOID						SendMsg(WCHAR* strParam);
};
