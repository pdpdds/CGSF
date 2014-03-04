
#pragma once
#include "DXUTguiIME.h"
#include "DXUTgui.h"
#include "ScrollBarCtrl.h"
#include <vector>

using namespace std;

// [6/24/2008 Psj] 텍스트 라인당 길이저장, 엔터 칠때 nEnterW에 L"\r\n" 길이가 들어간다.
typedef struct _TStrLineInfo
{
	int nTextW;
	int nTextNum;
	int nEnterW;
	_TStrLineInfo(int _nTextW=0,int _nTextNum=0,int _nEnterW=0)
	{
		nTextW = _nTextW;
		nTextNum = _nTextNum;
		nEnterW = _nEnterW;
	}
}TStrLineInfo;

#define VSTRLINEINFO vector<TStrLineInfo>
#define ITSTRLINEINFO vector<TStrLineInfo>::iterator

class CTextBoxCtrl : public CDXUTIMEEditBox
{

protected:

	/*/////////////////////////////////////////////////////////////////////
	m_nBeforeStrNumSav - [6/26/2008 Psj] 이전 버퍼스트링 저장용
	m_nSBWidth - 스크롤바 관련
	m_nSelected - Index of the selected item for single selection list box
	*///////////////////////////////////////////////////////////////////////

	int m_nShadow;
	int m_nCaretNowCol;
	int m_nHeightPassCaret;
	int m_nbuffWordSiz;
	int m_nSelStartSizX;
	int m_nBeforeStrNumSav;
	int m_nTextWidth;
	int m_nTextHeight;
	int m_mEnterWidth;
	int m_nSBWidth;     
	int m_nSelected;   
	int m_nCaretX;
	int m_nCaretLine;

	bool m_bSBInit;
	RECT m_RcCaret;
	D3DCOLOR m_ShadowFontColor;
	CScrollBarCtrl  m_ScrollBar;
	VSTRLINEINFO  m_vStrLineInfo;

public:

	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	GetScrollPosToCaretShow // 스크롤위치에 따른 caret가 보여지는지 확인해준다.
	SetScrollPosMovCaretPos // 스크롤 위치를 caret이 있는곳까지 이동시킨다.
	SetCaretNowCol // 직접적 라인값을 바로 설정한다.
	*//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	CTextBoxCtrl(CDXUTDialog *pDialog);
	~CTextBoxCtrl();
	int GetCaretNowCol(){return m_nCaretNowCol;}
	void Init(LPCWSTR pwszText, int nX, int nY, int nW, int nH, UINT nHotKey=NULL, bool bDefault=NULL);
	void Init(LPCWSTR pwszText, RECT &rcCtrl, UINT nHotKey=NULL, bool bDefault=NULL);
	void SetSpriteToScrollBar(DWORD nPackageID, LPCWSTR wszAnimation,int nBackImg, int nSBarBtnUp,int nSBarBtnDown,int nSBarBtnMv);
	void SetScrollRange();
	void SetCaretNowCol(int nCol){m_nCaretNowCol = nCol;}
	void SetScrollPosMovCaretPos();
	bool GetScrollPosToCaretShow();
	bool BaseHandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam );

	virtual void Render( float fElapsedTime );
	virtual void EditRender( float fElapsedTime );
	virtual void RenderComposition(float fElapsedTime);
	virtual void UpdateRects();
	virtual bool MsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual bool HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam );
	virtual bool HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam );

protected:

	/*//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	SetCaretPosNewLine() - bCaretNowCol: 실제로 위치한 포커스영역 Caret  | 함수 nLine: 1 한칸아래 , -1 한칸 위, n n칸만큼
	GetLineStrEnd() -  [6/19/2008 Psj] 라인의 마지막 스트링을 출력한다
	SetCaretPosNewLine() //caret의 라인위치를 결정한다. nLine 라인위치 bCaretNowCol직접적 라인위치 유무 bMode 라인바로적용 또는
	                     //라인값을 축적해 둔다.
	*//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SetCaretPosNewLine(int nLine = 1, bool bCaretNowCol = true ,bool bMode = false);
	void OtherMsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
	void SetNowLineDel();
	void SetArryLineCaretX();
	SIZE GetCharSiz(const WCHAR* pChar);
	WCHAR GetLineStrEnd(int nLine); 
	
};