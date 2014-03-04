#pragma once
#include <dshow.h>

#define WM_GRAPHNOTIFY  WM_USER+13 //필터그래프 메세지 발생시 HandleGraphEvent 호출!!

class CMP3Player
{
public:
	CMP3Player(void);
	virtual ~CMP3Player(void);

	BOOL Initialize(HWND hWnd);
	BOOL Finally();

	BOOL OnPlayAudio(TCHAR* szName, BOOL bLooped = FALSE );

//윈도우 이벤트를 받아야 되므로 콘솔에서는 처리할 수가 없다.
//다이얼로그를 생성해서 메세지를 받을 상황이 되면 자세히 구현하도록 한다.
	BOOL HandleGraphEvent();

	static CMP3Player* GetInstance()
	{
		if (NULL == m_pMP3Manager)
		{
			m_pMP3Manager = new CMP3Player();
		}
		return m_pMP3Manager;
	}

protected:

private:
	HWND m_hWnd;

	// DirectShow Graph, Filter & Pins used
	IGraphBuilder *m_pGraphBuilder;
	IMediaControl *m_pMediaControl;
	IMediaSeeking *m_pMediaSeeking;
	IBaseFilter   *m_pSourceCurrent;
	IBaseFilter   *m_pSourceNext;
	TCHAR          m_szCurrentFile[128];
	IMediaEventEx *m_pME;

	static CMP3Player* m_pMP3Manager;
};
