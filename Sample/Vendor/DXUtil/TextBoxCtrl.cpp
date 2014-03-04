#include "stdafx.h"
#include "dxut.h"
#include "TextBoxCtrl.h"
#include <string>

#define DXUT_NEAR_BUTTON_DEPTH 0.6f
#define DXUT_FAR_BUTTON_DEPTH 0.8f

#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN 0x020B // (not always defined)R
#endif
#ifndef WM_XBUTTONUP
#define WM_XBUTTONUP 0x020C // (not always defined)
#endif
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A // (not always defined)
#endif
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120 // (not always defined)
#endif

#define SPI_GETWHEELSCROLLLINES 0x0068


#define CHT_IMEFILENAME1    "TINTLGNT.IME" // New Phonetic
#define CHT_IMEFILENAME2    "CINTLGNT.IME" // New Chang Jie
#define CHT_IMEFILENAME3    "MSTCIPHA.IME" // Phonetic 5.1
#define CHS_IMEFILENAME1    "PINTLGNT.IME" // MSPY1.5/2/3
#define CHS_IMEFILENAME2    "MSSCIPYA.IME" // MSPY3 for OfficeXP

#define LANG_CHT            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)
#define LANG_CHS            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)
#define _CHT_HKL            ( (HKL)(INT_PTR)0xE0080404 ) // New Phonetic
#define _CHT_HKL2           ( (HKL)(INT_PTR)0xE0090404 ) // New Chang Jie
#define _CHS_HKL            ( (HKL)(INT_PTR)0xE00E0804 ) // MSPY
#define MAKEIMEVERSION( major, minor )      ( (DWORD)( ( (BYTE)( major ) << 24 ) | ( (BYTE)( minor ) << 16 ) ) )

#define IMEID_CHT_VER42 ( LANG_CHT | MAKEIMEVERSION( 4, 2 ) )   // New(Phonetic/ChanJie)IME98  : 4.2.x.x // Win98
#define IMEID_CHT_VER43 ( LANG_CHT | MAKEIMEVERSION( 4, 3 ) )   // New(Phonetic/ChanJie)IME98a : 4.3.x.x // Win2k
#define IMEID_CHT_VER44 ( LANG_CHT | MAKEIMEVERSION( 4, 4 ) )   // New ChanJie IME98b          : 4.4.x.x // WinXP
#define IMEID_CHT_VER50 ( LANG_CHT | MAKEIMEVERSION( 5, 0 ) )   // New(Phonetic/ChanJie)IME5.0 : 5.0.x.x // WinME
#define IMEID_CHT_VER51 ( LANG_CHT | MAKEIMEVERSION( 5, 1 ) )   // New(Phonetic/ChanJie)IME5.1 : 5.1.x.x // IME2002(w/OfficeXP)
#define IMEID_CHT_VER52 ( LANG_CHT | MAKEIMEVERSION( 5, 2 ) )   // New(Phonetic/ChanJie)IME5.2 : 5.2.x.x // IME2002a(w/Whistler)
#define IMEID_CHT_VER60 ( LANG_CHT | MAKEIMEVERSION( 6, 0 ) )   // New(Phonetic/ChanJie)IME6.0 : 6.0.x.x // IME XP(w/WinXP SP1)
#define IMEID_CHS_VER41 ( LANG_CHS | MAKEIMEVERSION( 4, 1 ) )   // MSPY1.5  // SCIME97 or MSPY1.5 (w/Win98, Office97)
#define IMEID_CHS_VER42 ( LANG_CHS | MAKEIMEVERSION( 4, 2 ) )   // MSPY2    // Win2k/WinME
#define IMEID_CHS_VER53 ( LANG_CHS | MAKEIMEVERSION( 5, 3 ) )   // MSPY3    // WinXP


inline int RectWidth( RECT &rc ) { return ( (rc).right - (rc).left ); }
inline int RectHeight( RECT &rc ) { return ( (rc).bottom - (rc).top ); }

const int DEFSTR_WIDTH = 12;

CTextBoxCtrl::CTextBoxCtrl(CDXUTDialog *pDialog) : CDXUTIMEEditBox(pDialog) ,m_ScrollBar(pDialog)
{
	m_RcCaret.left = 0;
	m_RcCaret.right = 0;
	m_RcCaret.top = 0;
	m_RcCaret.bottom = 0;
	m_nHeightPassCaret = 0;
	m_nCaretNowCol = 0;
	m_nbuffWordSiz = 0;
	m_nCaretX = 0;
	m_nCaretLine = 0;

	m_bSBInit = false;
	m_nSBWidth = 16;
	m_nSelected = -1;
	
	m_vStrLineInfo.push_back(TStrLineInfo());

	m_nShadow = 0;
	m_nSelStartSizX = 0;
	m_nBeforeStrNumSav = m_Buffer.GetTextSize();

}

CTextBoxCtrl::~CTextBoxCtrl()
{

}


void CTextBoxCtrl::Init(LPCWSTR pwszText, int nX, int nY, int nW, int nH, UINT nHotKey, bool bDefault)
{
	SetText(pwszText);
	SetLocation(nX, nY);
	SetSize( nW, nH);
	SetHotkey( nHotKey );
	m_bIsDefault = bDefault;
	m_Elements.GetAt( 0 )->dwTextFormat = DT_LEFT;
	m_pDialog->InitControl(&m_ScrollBar);
	//m_mEnterWidth = GetCharSiz(L"\r\n").cx;
	
}


void CTextBoxCtrl::Init(LPCWSTR pwszText, RECT &rcCtrl, UINT nHotKey, bool bDefault)
{
	int nW = rcCtrl.right - rcCtrl.left;
	int nH = rcCtrl.bottom - rcCtrl.top;
	Init(pwszText,rcCtrl.left,rcCtrl.top,nW,nH,nHotKey,bDefault);
}


void CTextBoxCtrl::Render( float fElapsedTime )
{
	if( m_bVisible == false )
		return;
	// If we have not computed the indicator symbol width,
	// do it.
	if( !m_nIndicatorWidth )
	{
		RECT rc = { 0, 0, 0, 0 };
		m_pDialog->CalcTextRect( L"En", m_Elements.GetAt( 9 ), &rc );
		m_nIndicatorWidth = rc.right - rc.left;

		// Update the rectangles now that we have the indicator's width
		UpdateRects();
	}


	//int nSideW = m_rcBoundingBox.right - m_rcText.right;
	//m_ScrollBar.SetLocation( m_rcRender->right , m_rcRender->top );

	//m_ScrollBar.SetSize( m_nSBWidth, m_rcRender->bottom- m_rcRender->top);
	//DXUTFontNode* pFontNode = m_pDialog->GetManager()->GetFontNode( m_Elements.GetAt( 0 )->iFont );
	//if( pFontNode && pFontNode->nHeight )
	//{
	//	m_ScrollBar.SetPageSize( RectHeight( m_rcText ) / pFontNode->nHeight );

	//	// The selected item may have been scrolled off the page.
	//	// Ensure that it is in page again.
	//	m_ScrollBar.ShowItem( m_nSelected );



	//  [7/4/2008 Psj] fix 스크롤바
	//if( !m_bSBInit )
	//{
	//	// Update the page size of the scroll bar
	//	if( m_nTextHeight )
	//		m_ScrollBar.SetPageSize( RectHeight( m_rcText ) / m_nTextHeight );
	//	else
	//		m_ScrollBar.SetPageSize( RectHeight( m_rcText ) );
	//	m_bSBInit = true;
	//}


	// Let the parent render first (edit control)
	//CDXUTEditBox::Render( fElapsedTime );

	// Let the parent render first (edit control)
	EditRender( fElapsedTime ); //이쪽부분만 따로 BaseDlg 를 랜더링 할 수 있게한다.

	CDXUTElement* pElement = GetElement( 1 );
	if( pElement )
	{
		s_CompString.SetFontNode( m_pDialog->GetFont( pElement->iFont ) );
		s_CandList.HoriCand.SetFontNode( m_pDialog->GetFont( pElement->iFont ) );
	}

	//
	// Now render the IME elements
	//

	ImeUi_RenderUI();
	

	if( m_bHasFocus )	
	{
		// Render the input locale indicator
		RenderIndicator( fElapsedTime );

		// Display the composition string.
		// This method should also update s_ptCompString
		// for RenderCandidateReadingWindow.
		RenderComposition( fElapsedTime );

		// Display the reading/candidate window. RenderCandidateReadingWindow()
		// uses s_ptCompString to position itself.  s_ptCompString must have
		// been filled in by RenderComposition().
		if( ImeUi_IsShowReadingWindow() )
			// Reading window
			RenderCandidateReadingWindow( fElapsedTime, true );
		else if( ImeUi_IsShowCandListWindow() )
			// Candidate list window
			RenderCandidateReadingWindow( fElapsedTime, false );
	}

	//Render the scroll bar
	m_ScrollBar.Render( fElapsedTime );

}

void CTextBoxCtrl::EditRender( float fElapsedTime )
{
	if( m_bVisible == false )
		return;

	HRESULT hr;
	int nSelStartX = 0, nCaretX = 0;  // Left and right X cordinates of the selection region
	DXUTFontNode* pFont = m_pDialog->GetFont( m_Elements.GetAt( 0 )->iFont );

	CDXUTElement* pElement = GetElement( 0 );
	if( pElement )
	{
		m_Buffer.SetFontNode( m_pDialog->GetFont( pElement->iFont ) );
		PlaceCaret( m_nCaret );  // Call PlaceCaret now that we have the font info (node),
		// so that scrolling can be handled.
	}



	// Render the control graphics
	for( int e = 0; e < 9; ++e )
	{
		pElement = m_Elements.GetAt( e );
		pElement->TextureColor.Blend( DXUT_STATE_NORMAL, fElapsedTime );

		m_pDialog->DrawSprite( pElement, &m_rcRender[e], DXUT_FAR_BUTTON_DEPTH );
	}



	//
	// Compute the X coordinates of the first visible character.
	//
	int nXFirst;
	m_Buffer.CPtoX( m_nFirstVisible, FALSE, &nXFirst );

	//
	// Compute the X coordinates of the selection rectangle //  [6/19/2008 Psj] 문자열 셀렉트 마킹 Start길이 구한다.
	// 
	hr = m_Buffer.CPtoX( m_nCaret, FALSE, &nCaretX );
	if( m_nCaret != m_nSelStart )
	{
		hr = m_Buffer.CPtoX( m_nSelStart, FALSE, &nSelStartX );
		//m_nSelStartSizX = m_vCaretX[m_nCaretNowCol];
	
	}
	else
		nSelStartX = nCaretX;

	//
	// Render the selection rectangle
	//
	RECT rcSelection;  // Make this available for rendering selected text
	static RECT rcSelectionSiz;
	if( m_nCaret != m_nSelStart )
	{


		int nSelLeftX = nCaretX, nSelRightX = nSelStartX;
		// Swap if left is bigger than right
		if( nSelLeftX > nSelRightX )
		{ int nTemp = nSelLeftX; nSelLeftX = nSelRightX; nSelRightX = nTemp; }

		SetRect( &rcSelection, nSelLeftX, m_rcText.top, nSelRightX, m_rcText.bottom );
		OffsetRect( &rcSelection, m_rcText.left - nXFirst, 0 );
		IntersectRect( &rcSelection, &m_rcText, &rcSelection );


		//  [6/19/2008 Psj] 중간에 RECT를 변경 마킹사이즈를 조절 해준다.
		int nStartX=0,nEndX=0;
		int nwidth = rcSelection.right-rcSelection.left;
		if(m_nSelStartSizX != m_nCaretX) //  [6/19/2008 Psj] 스타트 셀렉점과 캐럿의 위치가 같으면 위치설정을 안한다.
		{
			nStartX = m_rcText.left+m_nSelStartSizX;
			nEndX = nStartX+nwidth;
		}
		if( m_nSelStartSizX > m_nCaretX)
		{
			nEndX = nStartX;
			nStartX = nStartX - nwidth;
		}
		SetRect( &rcSelectionSiz, nStartX, rcSelection.top+m_nHeightPassCaret,nEndX, rcSelection.top+m_nHeightPassCaret+pFont->nHeight);

		m_pDialog->DrawRect( &rcSelectionSiz, m_SelBkColor);

		/*
		IDirect3DDevice9* pd3dDevice = m_pDialog->GetManager()->GetD3D9Device();
		if( pd3dDevice )
		pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );\
		if( pd3dDevice )
		pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
		*/
	}

	//
	// Render the text
	//
	// Element 0 for text
	m_Elements.GetAt( 0 )->FontColor.Current = m_TextColor;
	
	//  [6/13/2008] 글자 간격,line 조정
	int nTemTextW = 0,nStrWidth=0;
	int nNum=0, nNumSum=0;
	RECT rcTem = m_rcText;
	

	int nStrSum = 0;
	for(int i=0;i<(int)m_vStrLineInfo.size();i++)
	{

		rcTem.right  = rcTem.left + m_vStrLineInfo[i].nTextW;
		rcTem.top = (pFont->nHeight*i)+m_rcText.top - (m_ScrollBar.GetTrackPos() * pFont->nHeight);;
		rcTem.bottom = rcTem.top+pFont->nHeight;

		if(i==0)
		{
			if(m_rcRender->bottom<rcTem.bottom || rcTem.top < m_rcRender->top ) continue;// 문자열 라인이 범위를 넘었을때 그려주지 않는다.
			m_pDialog->DrawText(m_Buffer.GetBuffer(), m_Elements.GetAt( 0 ), &rcTem );
		}
		else
		{
			nStrSum = nStrSum + m_vStrLineInfo[i-1].nTextNum;
			if(m_rcRender->bottom<rcTem.bottom || rcTem.top < m_rcRender->top ) continue;// 문자열 라인이 범위를 넘었을때 그려주지 않는다.
			m_pDialog->DrawText(m_Buffer.GetBuffer()+nStrSum, m_Elements.GetAt( 0 ), &rcTem );
		}
	}

	// Render the selected text //  [6/19/2008 Psj] 선택한 글자 마킹되는 코드부분
	if( m_nCaret != m_nSelStart )
	{
		int nFirstToRender = __max( m_nFirstVisible, __min( m_nSelStart, m_nCaret ) );
		int nNumChatToRender = __max( m_nSelStart, m_nCaret ) - nFirstToRender;
		m_Elements.GetAt( 0 )->FontColor.Current = m_SelTextColor;
		
		m_pDialog->DrawText( m_Buffer.GetBuffer() + nFirstToRender,
			m_Elements.GetAt( 0 ), &rcSelectionSiz, false, nNumChatToRender );
	}


	//
	// Blink the caret
	//
	if( DXUTGetGlobalTimer()->GetAbsoluteTime() - m_dfLastBlink >= m_dfBlink )
	{
		m_bCaretOn = !m_bCaretOn;
		m_dfLastBlink = DXUTGetGlobalTimer()->GetAbsoluteTime();
	}

	//
	// Render the caret if this control has the focus
	//
	if( m_bHasFocus && m_bCaretOn && !s_bHideCaret )
	{
		// Start the rectangle with insert mode caret

		//hr = m_Buffer.CPtoX( m_nCaret, FALSE, &nCaretX );
		//m_vCaretX[m_nCaretCol]
		m_RcCaret.left = m_rcText.left;
		m_RcCaret.right = m_rcText.right;
		m_RcCaret.top = m_rcText.top+m_nHeightPassCaret;
		m_RcCaret.bottom = m_RcCaret.top+pFont->nHeight;

		RECT rcCaret = { m_RcCaret.left  + m_nCaretX - 1, m_RcCaret.top,
			m_RcCaret.left + m_nCaretX + 1, m_RcCaret.bottom };

		// If we are in overwrite mode, adjust the caret rectangle
		// to fill the entire character.
		if( !m_bInsertMode )
		{
			// Obtain the right edge X coord of the current character
			int nRightEdgeX;
			m_Buffer.CPtoX( m_nCaret, TRUE, &nRightEdgeX );
			rcCaret.right = m_rcText.left - nXFirst + nRightEdgeX;
		}

		if(GetScrollPosToCaretShow())
		{
			m_pDialog->DrawRect( &rcCaret, m_CaretColor );
		}
		
	}
}

bool CTextBoxCtrl::GetScrollPosToCaretShow()
{
	if(m_ScrollBar.GetTrackPos()<= m_nCaretNowCol && m_nCaretNowCol < m_ScrollBar.GetTrackPos()+m_ScrollBar.GetPageSize()-1)
	{
		return true;
	}

	return false;
}


bool CTextBoxCtrl::MsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !m_bEnabled || !m_bVisible )
		return false;

#if defined(DEBUG) || defined(_DEBUG)
	// DXUT.cpp used to call CDXUTIMEEditBox::StaticMsgProc() so that, but now
	// this is the application's responsiblity.  To do this, call 
	// CDXUTDialogResourceManager::MsgProc() before calling this function.
	assert( m_bIMEStaticMsgProcCalled && L"To fix, call CDXUTDialogResourceManager::MsgProc() first" );
#endif
	switch( uMsg )
	{
	case WM_DESTROY:
		ImeUi_Uninitialize();
		break;	
	
	}	

	bool trappedData;
	bool* trapped = &trappedData;

	*trapped = false;
	if( !ImeUi_IsEnabled() )
		return CDXUTEditBox::MsgProc( uMsg, wParam, lParam );

	ImeUi_ProcessMessage( DXUTGetHWND(), uMsg, wParam, lParam, trapped );
	if( *trapped == false )
	{
		CDXUTEditBox::MsgProc( uMsg, wParam, lParam );

			int nCaretX;
			m_Buffer.CPtoX( m_nCaret, FALSE, &nCaretX );
			m_nCaretX = nCaretX - m_nbuffWordSiz;



			static int nSavCaretX = nCaretX;
			int nSavCaretCol = m_nCaretNowCol;
			int nSavHeightPassCaret =  m_nHeightPassCaret;



			if((m_nBeforeStrNumSav!=m_Buffer.GetTextSize())) //  [6/24/2008 Psj] 문자가 변할때 마다 길이정보저장
			{
				CDXUTElement* pElement = m_Elements.GetAt( 0 );
				DXUTFontNode* pFontNode = m_pDialog->GetFont(pElement->iFont);
				int nTextH = m_rcText.bottom - m_rcText.top;

				//  [6/16/2008 Psj] 그전 문자의 길이와 지금 문자길이를 빼서 문자길이를 알아낸다.
				int nStrSiz = 0;
				nStrSiz = nCaretX - nSavCaretX;


				// fix 크기에 맞겠금 잘라 주어야 한다.
				int nWidth = m_rcText.right-m_rcText.left;

				if( (m_vStrLineInfo[m_nCaretNowCol].nTextW+nStrSiz) > nWidth )
				{
					if( wParam!=VK_BACK  && nWidth<m_nCaretX)
					{

						if( (m_vStrLineInfo[m_nCaretNowCol].nTextW+nStrSiz) > nWidth && (m_vStrLineInfo[m_nCaretNowCol].nEnterW) )
						{
							m_vStrLineInfo.insert(m_vStrLineInfo.begin()+m_nCaretNowCol+1,1,TStrLineInfo(0,2,m_vStrLineInfo[m_nCaretNowCol].nEnterW));
							m_vStrLineInfo[m_nCaretNowCol].nEnterW = 0;
							m_vStrLineInfo[m_nCaretNowCol].nTextNum = m_vStrLineInfo[m_nCaretNowCol].nTextNum-2; 
						}


						SetCaretPosNewLine(); 
						m_nbuffWordSiz = m_nbuffWordSiz+(nCaretX-m_nbuffWordSiz-nStrSiz);


					}

				}




				for(int i=m_nCaretNowCol;i<=(int)m_vStrLineInfo.size();i++)
				{
					if(i==m_vStrLineInfo.size())
					{
						m_vStrLineInfo.push_back(TStrLineInfo());
					}



					// 문자열 꽉찬상태에서 지우고 다시 쓰고 엔터쳤을때 TextW크기 꽉찬상태 그대로 버그
					if( (m_vStrLineInfo[i].nTextW+nStrSiz) < nWidth/* || (m_vStrLineInfo[i].nEnterW) */)
					{

						//  [7/7/2008 Psj] 문자열이 아래 끝까지 문자열 끝에 있을때 스크롤바 위치를 지정한다.
						if((nSavHeightPassCaret+(pFontNode->nHeight) > (nTextH-pFontNode->nHeight)) && m_nCaretX > nWidth)
						{
							SetScrollRange();
							m_ScrollBar.SetTrackPos(m_ScrollBar.GetTrackPos()+1);
						}

						//  [7/1/2008 Psj] 밑에까지 이어지는 문자열을 뺄때 아래 값이 있으면 맨 아래부터 빼준다.
						if( ((int)(m_vStrLineInfo[i].nTextW+(nStrSiz<0? -nStrSiz : +nStrSiz)) > nWidth) 
							&& i<((int)m_vStrLineInfo.size())-1 &&(!m_vStrLineInfo[i].nEnterW) )
						{
							continue;
						}




						m_vStrLineInfo[i].nTextW = m_vStrLineInfo[i].nTextW + nStrSiz;	
						m_vStrLineInfo[i].nTextNum = m_vStrLineInfo[i].nTextNum + (nStrSiz>0 ? 1 :-1);

						if(m_vStrLineInfo[i].nTextW==0)
						{
							if( /*i!=0 &&*/ m_vStrLineInfo[i].nEnterW && i!=m_nCaretNowCol /*&& !m_vStrLineInfo[i-1].nEnterW*/)
							{
								int qweqw = m_nCaretNowCol;
								if(i>0)
								{
									m_vStrLineInfo[i-1].nTextNum =m_vStrLineInfo[i-1].nTextNum+ m_vStrLineInfo[i].nTextNum;
									m_vStrLineInfo[i-1].nEnterW = m_vStrLineInfo[i].nEnterW;
									m_vStrLineInfo.erase(m_vStrLineInfo.begin()+i);
								}

							}

							if(i>0 && m_vStrLineInfo.size()>1 && (!m_vStrLineInfo[i-1].nEnterW) )
							{
								m_vStrLineInfo.erase(m_vStrLineInfo.begin()+i);
							}

						}
						break;
					}
					else if(m_vStrLineInfo[i].nEnterW)
					{
						m_vStrLineInfo.insert(m_vStrLineInfo.begin()+i+1,1,TStrLineInfo(nStrSiz,3,m_vStrLineInfo[i].nEnterW));
						m_vStrLineInfo[i].nEnterW = 0;
						m_vStrLineInfo[i].nTextNum = m_vStrLineInfo[i].nTextNum-2; 
						break;
					}



				}

				SetScrollRange();
				SetScrollPosMovCaretPos();

			}

			nSavCaretX = nCaretX;
			nSavCaretCol = m_nCaretNowCol;
			m_nBeforeStrNumSav = m_Buffer.GetTextSize();

			switch( uMsg )
			{
				// Make sure that while editing, the keyup and keydown messages associated with 
				// WM_CHAR messages don't go to any non-focused controls or cameras
			case WM_KEYUP:
			case WM_KEYDOWN:
				{
					OtherMsgProc(  uMsg,  wParam,  lParam ); //  [6/17/2008 Psj] 엔터, 백스페이스
				}
				break;
			}
	}

	return *trapped;	
}


void CTextBoxCtrl::OtherMsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	switch( uMsg )
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			switch ( wParam )
			{
			case VK_UP:
				{
					if( 0>=(m_nCaretNowCol) ) 
						return;

					int nStrSum=0;
					for(int i=0;i<m_nCaretNowCol;i++)
					{
						nStrSum = nStrSum+m_vStrLineInfo[i].nTextNum;
					}

					int nLineCaret=0;
					nLineCaret = m_nCaret - nStrSum; // m_nCaret는 엔터값이 추가되어있는 상태
					m_nbuffWordSiz =m_nbuffWordSiz - m_vStrLineInfo[m_nCaretNowCol-1].nTextW-m_vStrLineInfo[m_nCaretNowCol-1].nEnterW;
					
					int nUpLineNum = m_vStrLineInfo[m_nCaretNowCol-1].nEnterW ? m_vStrLineInfo[m_nCaretNowCol-1].nTextNum -2 : m_vStrLineInfo[m_nCaretNowCol-1].nTextNum;
					if(nUpLineNum <nLineCaret)
					{
						//위로 올릴때 윗라인이 현재라인 크기보다 작을때.
						//m_nCaret =m_nCaret- (m_vStrLineInfo[m_nCaretNowCol].nTextNum)-(m_vStrLineInfo[m_nCaretNowCol-1].nEnterW ? 2:0 ); //m_vStrLineInfo[m_nCaretNowCol-1].nEnterW ? 2:0 엔터값 빼준다.
					
						m_nCaret =m_nCaret-nLineCaret-(m_vStrLineInfo[m_nCaretNowCol-1].nEnterW ? 2:0 );
					
					}
					else
					{
						m_nCaret =m_nCaret- m_vStrLineInfo[m_nCaretNowCol-1].nTextNum;
					}
					
					m_nSelStart = m_nCaret;

					

					if(m_ScrollBar.GetTrackPos()!=0 && m_nHeightPassCaret==0 )
					{
						m_ScrollBar.SetTrackPos(m_ScrollBar.GetTrackPos()-1);
						m_nCaretNowCol--;
					}
					else
					{
						SetCaretPosNewLine(-1);
					}



				}
				break;
			
			case VK_DOWN:
				{
					CDXUTElement* pElement = m_Elements.GetAt( 0 );
					DXUTFontNode* pFontNode = m_pDialog->GetFont(pElement->iFont);
					if( ((int)m_vStrLineInfo.size())<=(m_nCaretNowCol+1) ) 
						return;

					int nStrSum=0;
					for(int i=0;i<m_nCaretNowCol;i++)
					{
						nStrSum = nStrSum+m_vStrLineInfo[i].nTextNum;
					}

					int nLineCaret=0;
					nLineCaret = m_nCaret - nStrSum; // m_nCaret는 엔터값이 추가되어있는 상태
					m_nbuffWordSiz =m_nbuffWordSiz + m_vStrLineInfo[m_nCaretNowCol].nTextW+m_vStrLineInfo[m_nCaretNowCol].nEnterW;

					int nDownLineNum = m_vStrLineInfo[m_nCaretNowCol+1].nEnterW ? m_vStrLineInfo[m_nCaretNowCol+1].nTextNum -2 : m_vStrLineInfo[m_nCaretNowCol+1].nTextNum;
					if(nDownLineNum <nLineCaret)
					{
						m_nCaret =m_nCaret+ (m_vStrLineInfo[m_nCaretNowCol].nTextNum-nLineCaret)+nDownLineNum ;

					}
					else
					{
						m_nCaret =m_nCaret+ m_vStrLineInfo[m_nCaretNowCol].nTextNum;
					}
					m_nSelStart = m_nCaret;


					//84

					int nTextH = m_rcText.bottom - m_rcText.top;
					if((m_nHeightPassCaret+(pFontNode->nHeight) > (nTextH-pFontNode->nHeight)))
					{
						m_ScrollBar.SetTrackPos(m_ScrollBar.GetTrackPos()+1);
						m_nCaretNowCol++;
					}
					else
					{
						SetCaretPosNewLine();
					}
					
				}
				break;
			case VK_LEFT: //  [6/17/2008 Psj] fix LEFT로 텍스트 위로 올라갈때
				{
					if(m_nCaretNowCol>0)
					{
						SIZE fSiz;
						// [6/19/2008 Psj] 캐럿 위치가 위로 올라갈때 위치를 설정 해준다,
						// 리턴값이 없을때 이분기로 넘어온다.,

						if((!m_vStrLineInfo[m_nCaretNowCol-1].nEnterW) && (m_nCaretX)==0)
						{
							fSiz = GetCharSiz(&m_Buffer[m_Buffer.GetTextSize()-1]);
						
							if(m_ScrollBar.GetTrackPos()!=0 && m_nHeightPassCaret==0 )
							{
								m_ScrollBar.SetTrackPos(m_ScrollBar.GetTrackPos()-1);
								m_nCaretNowCol--;
							}
							else
							{
								SetCaretPosNewLine(-1);
							}


							m_nbuffWordSiz = m_nbuffWordSiz - m_vStrLineInfo[m_nCaretNowCol].nTextW;
							m_nCaretX = m_vStrLineInfo[m_nCaretNowCol].nTextW;
						} // 리턴값이 있을때(엔터) 이분기로 넘어온다.
						else if(m_nCaretX==0) 
						{
							PlaceCaret( m_nCaret - 1 );
							m_nSelStart = m_nCaret;

							if(m_ScrollBar.GetTrackPos()!=0 && m_nHeightPassCaret==0 )
							{
								m_ScrollBar.SetTrackPos(m_ScrollBar.GetTrackPos()-1);
								m_nCaretNowCol--;
							}
							else
							{
								SetCaretPosNewLine(-1);
							}


							m_nbuffWordSiz =m_nbuffWordSiz - m_vStrLineInfo[m_nCaretNowCol].nTextW - m_vStrLineInfo[m_nCaretNowCol].nEnterW; 
							m_nCaretX = m_vStrLineInfo[m_nCaretNowCol].nTextW;
						}
					}

				}
				break;
			case VK_RIGHT:
				{
					CDXUTElement* pElement = m_Elements.GetAt( 0 );
					DXUTFontNode* pFontNode = m_pDialog->GetFont(pElement->iFont);
					int nTextH = m_rcText.bottom - m_rcText.top;
			
					if(m_nCaretNowCol<(int)(m_vStrLineInfo.size()-1))
					{

						//  [6/26/2008 Psj] 이어진 문자 끝에 Caret 현재 위치 알아본다. (미리 다음라인으로 가기위해) 
						int nStrNum,nPass;
						m_Buffer.XtoCP(m_nCaretX,&nStrNum,&nPass);
						
						SIZE fSiz;
						fSiz = GetCharSiz(L"\r\n");
						// [6/19/2008 Psj] 캐럿 위치가 위로 올라갈때 위치를 설정 해준다,
						// 리턴값이 없을때 이분기로 넘어온다.,
						if((!m_vStrLineInfo[m_nCaretNowCol].nEnterW) && nStrNum+1>=(m_vStrLineInfo[m_nCaretNowCol].nTextNum))
						{
							fSiz = GetCharSiz(&m_Buffer[m_Buffer.GetTextSize()-1]);
							if((m_nHeightPassCaret+(pFontNode->nHeight) > (nTextH-pFontNode->nHeight)))
							{
								m_ScrollBar.SetTrackPos(m_ScrollBar.GetTrackPos()+1);
								m_nCaretNowCol++;
							}
							else
							{
								SetCaretPosNewLine();
							}
							m_nbuffWordSiz = m_nbuffWordSiz + m_vStrLineInfo[m_nCaretNowCol-1].nTextW;
							m_nCaretX = m_vStrLineInfo[m_nCaretNowCol].nTextW;

						} // 리턴값이 있을때(엔터) 이분기로 넘어온다.
						else if((m_vStrLineInfo[m_nCaretNowCol].nEnterW) && m_nCaretX>=(m_vStrLineInfo[m_nCaretNowCol].nTextW)) 
						{
							PlaceCaret( m_nCaret +1 );
							m_nSelStart = m_nCaret;
							fSiz = GetCharSiz(L"\r\n");
							m_nbuffWordSiz =m_nbuffWordSiz + m_vStrLineInfo[m_nCaretNowCol].nTextW + m_vStrLineInfo[m_nCaretNowCol].nEnterW; 
							if((m_nHeightPassCaret+(pFontNode->nHeight) > (nTextH-pFontNode->nHeight)))
							{
								m_ScrollBar.SetTrackPos(m_ScrollBar.GetTrackPos()+1);
								m_nCaretNowCol++;
							}
							else
							{
								SetCaretPosNewLine();
							}
							m_nCaretX = 0;
						}
					}
				}
				break;

			case VK_SHIFT:
				{

					if( !(m_nCaret != m_nSelStart))
					{
						m_nSelStartSizX = m_nCaretX;
					}
				}
				break;
		
			case VK_BACK:
				{
					SIZE fSiz;
					WCHAR tmpStr[2];
					memcpy(&tmpStr[0],&m_Buffer[m_Buffer.GetTextSize()-1],sizeof(WCHAR));
					tmpStr[1] = NULL;
					fSiz = GetCharSiz(tmpStr);

					if(m_nCaretNowCol>0)
					{
						// [6/19/2008 Psj] 캐럿 위치가 위로 올라갈때 위치를 설정 해준다,
						// 리턴값이 없을때 이분기로 넘어온다.,
						if((!m_vStrLineInfo[m_nCaretNowCol-1].nEnterW) && ((m_nCaretX-fSiz.cx)==0 || m_nCaretX==0) ) // 문자열이 남았는데 CaretX의 위치가 '0' 일때 모두 지워지는 버그,
						{
							if(m_vStrLineInfo[m_nCaretNowCol].nTextNum==1)
							{
								SetNowLineDel();
							}
							else if(m_vStrLineInfo[m_nCaretNowCol].nEnterW && m_vStrLineInfo[m_nCaretNowCol].nTextNum ==3)
							{
								m_vStrLineInfo[m_nCaretNowCol-1].nEnterW =  m_vStrLineInfo[m_nCaretNowCol].nEnterW;
								SetNowLineDel();
								m_vStrLineInfo[m_nCaretNowCol-1].nTextNum = m_vStrLineInfo[m_nCaretNowCol-1].nTextNum+2;
							}
							else
							{
								m_vStrLineInfo[m_nCaretNowCol].nTextW -= fSiz.cx;
								m_vStrLineInfo[m_nCaretNowCol].nTextNum--;
							}

							if(m_ScrollBar.GetTrackPos()!=0 &&m_ScrollBar.GetTrackPos() != m_ScrollBar.GetEndSiz()  )
							{

								if(m_nHeightPassCaret==0)	
								{
									m_ScrollBar.SetTrackPos(m_ScrollBar.GetTrackPos()-1);
								}
								SetCaretPosNewLine(-1,false);
							}

							
							if(m_ScrollBar.GetTrackPos()==0)
							{

								SetCaretPosNewLine(-1);
							}
							else
							{
								m_nCaretNowCol--;
							}
							
							m_nbuffWordSiz =m_nbuffWordSiz - m_vStrLineInfo[m_nCaretNowCol].nTextW; 
							m_nBeforeStrNumSav = m_Buffer.GetTextSize()-1;
						

						} // 리턴값이 있을때(엔터) 이분기로 넘어온다.
						else if(m_nCaretX==0 && m_vStrLineInfo[m_nCaretNowCol-1].nEnterW)  // 문자열이 남았는데 CaretX의 위치가 '0' 일때 모두 지워지는 버그,
						{
							PlaceCaret( m_nCaret - 1 );
							m_nSelStart = m_nCaret;
							m_Buffer.RemoveChar( m_nCaret );
							fSiz = GetCharSiz(L"\r\n");

							int nSavLineW = m_vStrLineInfo[m_nCaretNowCol-1].nTextW;
								
							if(m_vStrLineInfo[m_nCaretNowCol].nTextNum>0)
							{
								int nSum=0;
								for(int i=0;i<m_nCaretNowCol;i++)
								{
									nSum = nSum+m_vStrLineInfo[i].nTextNum;
								}

								//  [7/1/2008 Psj] 현재라인이 엔터값과 아래라인문자열이 있을때 아래라인에서 지우면서 문자열이 남아있고
								//  현재라인으로 올라올때 아래라인의 문자열을 크기에 맞게 가져온다.

								int nTmpW=0;
								int nTextWidth = m_rcText.right-m_rcText.left;
								int MaxStr = m_vStrLineInfo[m_nCaretNowCol].nTextNum;
								
								wstring tmpStr;
								for(int n=0;n<MaxStr;n++)
								{

									tmpStr += m_Buffer[(nSum-1)+n];
									nTmpW =  GetCharSiz(tmpStr.data()).cx;

									if( (m_vStrLineInfo[m_nCaretNowCol-1].nTextW+nTmpW) > nTextWidth )
									{
										break;
									}

									if( !(m_vStrLineInfo[m_nCaretNowCol].nEnterW && m_vStrLineInfo[m_nCaretNowCol].nTextNum<=2) )
									{
										m_vStrLineInfo[m_nCaretNowCol-1].nTextW = m_vStrLineInfo[m_nCaretNowCol-1].nTextW+nTmpW;
										m_vStrLineInfo[m_nCaretNowCol].nTextW = m_vStrLineInfo[m_nCaretNowCol].nTextW-nTmpW;
									}
									

									m_vStrLineInfo[m_nCaretNowCol-1].nTextNum++;
									m_vStrLineInfo[m_nCaretNowCol].nTextNum--;	

									tmpStr.clear();
								}
							}

							m_nbuffWordSiz =m_nbuffWordSiz-m_vStrLineInfo[m_nCaretNowCol-1].nEnterW; // 윗 라인의 엔터값을 빼준다.

							if(m_vStrLineInfo[m_nCaretNowCol].nTextNum==0)
							{										

								m_vStrLineInfo[m_nCaretNowCol-1].nEnterW = m_vStrLineInfo[m_nCaretNowCol].nEnterW;
								SetNowLineDel();

								if(m_ScrollBar.GetTrackPos()!=0 &&m_ScrollBar.GetTrackPos() != m_ScrollBar.GetEndSiz() )
								{

									if(m_nHeightPassCaret==0)	
									{
										m_ScrollBar.SetTrackPos(m_ScrollBar.GetTrackPos()-1);
									}
									SetCaretPosNewLine(-1,false);
								}
							}
						
							
							if(m_ScrollBar.GetTrackPos()==0)
							{
								
								SetCaretPosNewLine(-1);
							}
							else
							{
								m_nCaretNowCol--;
							}
								
							m_nbuffWordSiz =m_nbuffWordSiz-nSavLineW;
							m_vStrLineInfo[m_nCaretNowCol].nTextNum = m_vStrLineInfo[m_nCaretNowCol].nTextNum-2;//  [6/30/2008 Psj] 미리 하나 빼준다.
							m_nBeforeStrNumSav = m_Buffer.GetTextSize()-1; 
							m_nCaretX = m_vStrLineInfo[m_nCaretNowCol].nTextW;

						}


						SetScrollRange();
					}
				}
				break;
			case VK_RETURN:
				{	
					DXUTFontNode* pFont = m_pDialog->GetFont( m_Elements.GetAt( 0 )->iFont );
					
					if( m_Buffer.InsertChar(m_nCaret,L'\r') )
					{
						PlaceCaret( m_nCaret + 1 );
						m_nSelStart = m_nCaret;
					}

					if( m_Buffer.InsertChar(m_nCaret,L'\n') )
					{
						PlaceCaret( m_nCaret + 1 );
						m_nSelStart = m_nCaret;
					}

					int nCaretX;
					m_Buffer.CPtoX( m_nCaret, FALSE, &nCaretX );
					m_nbuffWordSiz = m_nbuffWordSiz +(nCaretX-m_nbuffWordSiz);
					m_nCaretX = 0;
			
					int nStrSum=0;
					for(int i=0;i<m_nCaretNowCol;i++)
					{
						nStrSum = nStrSum+m_vStrLineInfo[i].nTextNum;
					}

					// m_nCaret는 엔터값이 추가되어있는 상태
					int nLineCaret = m_nCaret - nStrSum; 
					
					int nTmpW=0;
					int nNaxtCol = 1;
					int nTextW = m_rcText.right-m_rcText.left;
					int nTextH = m_rcText.bottom - m_rcText.top;
					int MaxStr = m_vStrLineInfo[m_nCaretNowCol].nTextNum - (nLineCaret-2);
					wstring tmpStr;

					//  [7/2/2008 Psj] 엔터 치면서 caret뒤에 남은 문자열 만큼 루프를 돌고 문자열을 알맞게 넣어준다,
					for(int n=0;n<MaxStr;n++)
					{
						tmpStr.clear();
						tmpStr = tmpStr+m_Buffer[m_nCaret+n];
						nTmpW  = GetCharSiz(tmpStr.data()).cx;

						if(m_vStrLineInfo[m_nCaretNowCol].nEnterW) //현재 문자열이 엔터값이 있는경우 
						{
							// 현재라인의 바로 다음 라인에 또 다른 라인을 삽입시켜준다.
							m_vStrLineInfo.insert(m_vStrLineInfo.begin()+m_nCaretNowCol+1,1,TStrLineInfo(0,2,m_vStrLineInfo[m_nCaretNowCol].nEnterW));
							
							m_vStrLineInfo[m_nCaretNowCol].nTextNum -=2;
						
							if(MaxStr==2) break; //MaxStr==2 엔터값만 있을경우 바로 넘어간다.
						
							m_vStrLineInfo[m_nCaretNowCol].nEnterW = 0;
							MaxStr -=2;// 엔터값 뺀 총 글자수
							

							
						}


						if(m_nCaretNowCol+nNaxtCol>=(int)m_vStrLineInfo.size()) //현재 넣어줄 라인이 더이상 없으면 넣어준다,
						{
							m_vStrLineInfo.push_back(TStrLineInfo());
						}


						if(m_vStrLineInfo[m_nCaretNowCol+nNaxtCol].nTextW+nTmpW>nTextW)
						{
							if(m_vStrLineInfo[m_nCaretNowCol+nNaxtCol].nEnterW) //엔터친 후 다음 문자열에 엔터값이 있는경우 처리
							{
								m_vStrLineInfo.insert(m_vStrLineInfo.begin()+m_nCaretNowCol+nNaxtCol+1,1,TStrLineInfo());
								m_vStrLineInfo[m_nCaretNowCol+nNaxtCol+1].nEnterW =  m_vStrLineInfo[m_nCaretNowCol+nNaxtCol].nEnterW;
								m_vStrLineInfo[m_nCaretNowCol+nNaxtCol].nEnterW = 0;
								m_vStrLineInfo[m_nCaretNowCol+nNaxtCol].nTextNum -= 2;
								m_vStrLineInfo[m_nCaretNowCol+nNaxtCol+1].nTextNum +=2;
								 
							}
							n -=1; // 컨티뉴 되므로 다시 되돌린다.
							nNaxtCol++;
							continue;
						}

						if(nTmpW!=6)
						{
							int qwdq =0;
						}

						m_vStrLineInfo[m_nCaretNowCol].nTextW = m_vStrLineInfo[m_nCaretNowCol].nTextW - nTmpW;
						m_vStrLineInfo[m_nCaretNowCol+nNaxtCol].nTextW = m_vStrLineInfo[m_nCaretNowCol+nNaxtCol].nTextW + nTmpW;
						
						m_vStrLineInfo[m_nCaretNowCol].nTextNum--;
						m_vStrLineInfo[m_nCaretNowCol+nNaxtCol].nTextNum++;

					}

					if(MaxStr==0 ) 
					{
						m_vStrLineInfo.insert(m_vStrLineInfo.begin()+m_nCaretNowCol+1,1,TStrLineInfo());
						
					}
					m_vStrLineInfo[m_nCaretNowCol].nTextNum = m_vStrLineInfo[m_nCaretNowCol].nTextNum+2;
					m_vStrLineInfo[m_nCaretNowCol].nEnterW = GetCharSiz(L"\r\n").cx;
				
					m_nBeforeStrNumSav = m_Buffer.GetTextSize();
					
					//  [7/4/2008 Psj] 스크롤바 트랙크기,위치 설정
					SetScrollRange();
					if(m_nCaretNowCol<(int)m_vStrLineInfo.size() &&  (m_nHeightPassCaret+(pFont->nHeight) > (nTextH-pFont->nHeight)))
					{
						m_ScrollBar.SetTrackPos(m_ScrollBar.GetTrackPos()+1);
					}
				
						
					SetCaretPosNewLine();
					ResetCaretBlink();

				}
				break;
			} //switch ( wParam )
			break;
		} //case WM_KEYDOWN: case WM_SYSKEYDOWN:
	default:
		break;
	} 

	switch( uMsg )
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			switch ( wParam )
			{
			case VK_UP:
			case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
			case VK_RETURN:
			case VK_BACK:
				{
					SetScrollPosMovCaretPos();
				}
				break;
			};
		}
		break;
	};

}
void CTextBoxCtrl::SetNowLineDel()
{
	vector<TStrLineInfo>::iterator it = m_vStrLineInfo.begin()+m_nCaretNowCol;
	m_vStrLineInfo.erase(it);
}


void CTextBoxCtrl::SetArryLineCaretX()
{
	int nCaretX;
	m_Buffer.CPtoX( m_nCaret, FALSE, &nCaretX );
	m_nCaretX = nCaretX - m_nbuffWordSiz;

}





SIZE CTextBoxCtrl::GetCharSiz(const WCHAR* pChar)
{
	SIZE fSiz;
	CDXUTElement* pElement = m_Elements.GetAt( 0 );
	DXUTFontNode* pFontNode = m_pDialog->GetFont(pElement->iFont);
	HDC dc = ((HDC)pFontNode->pFont9->GetDC());
	GetTextExtentPoint32(dc,pChar, (int)wcslen(pChar), &fSiz);
	return fSiz;
}



void CTextBoxCtrl::SetCaretPosNewLine(int nLine, bool bCaretNowCol, bool bMode)
{

	SIZE fSiz;
	CDXUTElement* pElement = m_Elements.GetAt( 0 );
	DXUTFontNode* pFontNode = m_pDialog->GetFont(pElement->iFont);

	int nTextH = m_rcText.bottom-m_rcText.top;
	if(bMode) // 라인 수 만큼 커서가 이동한다.
	{
		HDC dc = ((HDC)pFontNode->pFont9->GetDC());
		GetTextExtentPoint32(dc,L"\r\n", (int)wcslen(L"\r\n"), &fSiz);
		
		if(!(pFontNode->nHeight*nLine>(nTextH-pFontNode->nHeight)) &&
			!(pFontNode->nHeight*nLine<0))		
			m_nHeightPassCaret = (pFontNode->nHeight*nLine);
		
		if(bCaretNowCol)
			m_nCaretNowCol = nLine;
	}
	else // 라인 수 만큼 커서가 쌓여 이동한다.
	{
		HDC dc = ((HDC)pFontNode->pFont9->GetDC());
		GetTextExtentPoint32(dc,L"\r\n", (int)wcslen(L"\r\n"), &fSiz);
		
		if(!(m_nHeightPassCaret+(pFontNode->nHeight*nLine) > (nTextH-pFontNode->nHeight)) && 
			!(m_nHeightPassCaret+(pFontNode->nHeight*nLine)<0))
			m_nHeightPassCaret = m_nHeightPassCaret+(pFontNode->nHeight*nLine);
		
		if(bCaretNowCol)
			m_nCaretNowCol = m_nCaretNowCol+nLine;
	}
}

void CTextBoxCtrl::RenderComposition(float fElapsedTime)
{
	s_CompString.SetText( ImeUi_GetCompositionString() );

	RECT rcCaret = { 0, 0, 0, 0 };
	int nX, nXFirst;
	m_Buffer.CPtoX( m_nCaret, FALSE, &nX );
	m_Buffer.CPtoX( m_nFirstVisible, FALSE, &nXFirst );
	CDXUTElement* pElement = m_Elements.GetAt( 1 );

	// Get the required width
	RECT rc = { m_rcText.left + nX - nXFirst, m_rcText.top,
		m_rcText.left + nX - nXFirst, m_rcText.bottom };
	m_pDialog->CalcTextRect( s_CompString.GetBuffer(), pElement, &rc );

	// If the composition string is too long to fit within
	// the text area, move it to below the current line.
	// This matches the behavior of the default IME.
	if( rc.right > m_rcText.right )
		OffsetRect( &rc, m_rcText.left - rc.left, rc.bottom - rc.top );

	// Save the rectangle position for processing highlighted text.
	RECT rcFirst = rc;

	// Update s_ptCompString for RenderCandidateReadingWindow().
	s_ptCompString.x = rc.left; s_ptCompString.y = rc.top;


	D3DCOLOR TextColor = m_CompColor;
	// Render the window and string.
	// If the string is too long, we must wrap the line.
	pElement->FontColor.Current = TextColor;
	const WCHAR* pwszComp = s_CompString.GetBuffer();
	int nCharLeft = s_CompString.GetTextSize();
	for(; ; )
	{
		// Find the last character that can be drawn on the same line.
		int nLastInLine;
		int bTrail;
		s_CompString.XtoCP( m_rcText.right - rc.left, &nLastInLine, &bTrail );
		int nNumCharToDraw = __min( nCharLeft, nLastInLine );
		m_pDialog->CalcTextRect( pwszComp, pElement, &rc, nNumCharToDraw );

		// Draw the background
		// For Korean IME, blink the composition window background as if it
		// is a cursor.


		//  [6/16/2008 Psj] ime조합 마킹과 텍스트의 크기를 설정
		RECT rcSetTxt = rc;
		DXUTFontNode* pFont = m_pDialog->GetFont( m_Elements.GetAt( 0 )->iFont );
		rcSetTxt.left =  m_RcCaret.left  + m_nCaretX + 1;
		rcSetTxt.right = rcSetTxt.left + (rc.right-rc.left);
		rcSetTxt.top = m_rcText.top+m_nHeightPassCaret;
		rcSetTxt.bottom = rcSetTxt.top+(rc.bottom-rc.top);


		if( GetPrimaryLanguage() == LANG_KOREAN )
		{
			if( m_bCaretOn )
			{
				//  [6/16/2008 Psj] Ime 조합마킹부분
				m_pDialog->DrawRect( &rcSetTxt, m_CompWinColor );
			}
			else
			{
				// Not drawing composition string background. We
				// use the editbox's text color for composition
				// string text.
				TextColor = m_Elements.GetAt(0)->FontColor.States[DXUT_STATE_NORMAL];
			}
		} else
		{
			// Non-Korean IME. Always draw composition background.
			m_pDialog->DrawRect( &rc, m_CompWinColor );
		}

		// Draw the text 
		//  [6/16/2008 Psj] Ime  글자조합설정
		
		pElement->FontColor.Current = TextColor;
		m_pDialog->DrawText( pwszComp, pElement, &rcSetTxt, false, nNumCharToDraw );
		
		// Advance pointer and counter
		nCharLeft -= nNumCharToDraw;
		pwszComp += nNumCharToDraw;
		if( nCharLeft <= 0 )
			break;

		// Advance rectangle coordinates to beginning of next line
		OffsetRect( &rc, m_rcText.left - rc.left, rc.bottom - rc.top );
	}

	// Load the rect for the first line again.
	rc = rcFirst;

	// Inspect each character in the comp string.
	// For target-converted and target-non-converted characters,
	// we display a different background color so they appear highlighted.
	int nCharFirst = 0;
	nXFirst = 0;
	s_nFirstTargetConv = -1;
	BYTE* pAttr;
	const WCHAR* pcComp;
	for( pcComp = s_CompString.GetBuffer(), pAttr = ImeUi_GetCompStringAttr();
		*pcComp != L'\0'; ++pcComp, ++pAttr )
	{
		D3DCOLOR bkColor;

		// Render a different background for this character
		int nXLeft, nXRight;
		s_CompString.CPtoX( int( pcComp - s_CompString.GetBuffer() ), FALSE, &nXLeft );
		s_CompString.CPtoX( int( pcComp - s_CompString.GetBuffer() ), TRUE, &nXRight );

		// Check if this character is off the right edge and should
		// be wrapped to the next line.
		if( nXRight - nXFirst > m_rcText.right - rc.left )
		{
			// Advance rectangle coordinates to beginning of next line
			OffsetRect( &rc, m_rcText.left - rc.left, rc.bottom - rc.top );

			// Update the line's first character information
			nCharFirst = int( pcComp - s_CompString.GetBuffer() );
			s_CompString.CPtoX( nCharFirst, FALSE, &nXFirst );
		}

		// If the caret is on this character, save the coordinates
		// for drawing the caret later.
		if( ImeUi_GetImeCursorChars() == ( DWORD )( pcComp - s_CompString.GetBuffer() ) )
		{
			rcCaret = rc;
			rcCaret.left += nXLeft - nXFirst - 1;
			rcCaret.right = rcCaret.left + 2;
		}

		// Set up color based on the character attribute
		if( *pAttr == ATTR_TARGET_CONVERTED )
		{
			pElement->FontColor.Current = m_CompTargetColor;
			bkColor = m_CompTargetBkColor;
		}
		else if( *pAttr == ATTR_TARGET_NOTCONVERTED )
		{
			pElement->FontColor.Current = m_CompTargetNonColor;
			bkColor = m_CompTargetNonBkColor;
		}
		else
		{
			continue;
		}

		RECT rcTarget = { rc.left + nXLeft - nXFirst, rc.top, rc.left + nXRight - nXFirst, rc.bottom };
		m_pDialog->DrawRect( &rcTarget, bkColor );
		m_pDialog->DrawText( pcComp, pElement, &rcTarget, false, 1 );

		// Record the first target converted character's index
		if( -1 == s_nFirstTargetConv )
			s_nFirstTargetConv = int( pAttr - ImeUi_GetCompStringAttr() );
	}


	// Render the composition caret
	if( m_bCaretOn )
	{
		// If the caret is at the very end, its position would not have
		// been computed in the above loop.  We compute it here.
		if( ImeUi_GetImeCursorChars() == ( DWORD )s_CompString.GetTextSize() )
		{
			s_CompString.CPtoX( ImeUi_GetImeCursorChars(), FALSE, &nX );
			rcCaret = rc;
			rcCaret.left += nX - nXFirst - 1;
			rcCaret.right = rcCaret.left + 2;
		}
		else
		{
			//  [6/16/2008 Psj] ime 조합캐럿부분
			DXUTFontNode* pFont = m_pDialog->GetFont( m_Elements.GetAt( 0 )->iFont );
			rcCaret.left = m_RcCaret.left  + m_nCaretX - 1;
			rcCaret.right = m_RcCaret.left + m_nCaretX + 1;
			rcCaret.top = m_rcText.top+m_nHeightPassCaret;
			rcCaret.bottom =rcCaret.top+pFont->nHeight;
		}
		m_pDialog->DrawRect( &rcCaret, m_CompCaretColor );
	}
}


bool CTextBoxCtrl::HandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam )
{
	if( !m_bEnabled || !m_bVisible )
		return false;

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		{
			DXUTFontNode* pFont = m_pDialog->GetFont( m_Elements.GetAt( 9 )->iFont );

			// Check if this click is on top of the composition string
			int nCompStrWidth;
			s_CompString.CPtoX( s_CompString.GetTextSize(), FALSE, &nCompStrWidth );

			if( s_ptCompString.x <= pt.x &&
				s_ptCompString.y <= pt.y &&
				s_ptCompString.x + nCompStrWidth > pt.x &&
				s_ptCompString.y + pFont->nHeight > pt.y )
			{
				int nCharBodyHit, nCharHit;
				int nTrail;

				// Determine the character clicked on.
				s_CompString.XtoCP( pt.x - s_ptCompString.x, &nCharBodyHit, &nTrail );
				if( nTrail && nCharBodyHit < s_CompString.GetTextSize() )
					nCharHit = nCharBodyHit + 1;
				else
					nCharHit = nCharBodyHit;


				switch( GetPrimaryLanguage() )
				{
				case LANG_JAPANESE:
					// For Japanese, there are two cases.  If s_nFirstTargetConv is
					// -1, the comp string hasn't been converted yet, and we use
					// s_nCompCaret.  For any other value of s_nFirstTargetConv,
					// the string has been converted, so we use clause information.

					if( s_nFirstTargetConv != -1 )
					{
						int nClauseClicked = 0;
						while( ( int )s_adwCompStringClause[nClauseClicked + 1] <= nCharBodyHit )
							++nClauseClicked;

						int nClauseSelected = 0;
						while( ( int )s_adwCompStringClause[nClauseSelected + 1] <= s_nFirstTargetConv )
							++nClauseSelected;

						BYTE nVirtKey = nClauseClicked > nClauseSelected ? VK_RIGHT : VK_LEFT;
						int nSendCount = abs( nClauseClicked - nClauseSelected );
						while( nSendCount-- > 0 )
							SendKey( nVirtKey );

						return true;
					}

					// Not converted case. Fall thru to Chinese case.

				case LANG_CHINESE:
					{
						// For Chinese, use s_nCompCaret.
						BYTE nVirtKey = nCharHit > ( int )ImeUi_GetImeCursorChars() ? VK_RIGHT : VK_LEFT;
						int nSendCount = abs( nCharHit - ( int )ImeUi_GetImeCursorChars() );
						while( nSendCount-- > 0 )
							SendKey( nVirtKey );
						break;
					}
				}

				return true;
			}

			// Check if the click is on top of the candidate window
			if( ImeUi_IsShowCandListWindow() && PtInRect( &s_CandList.rcCandidate, pt ) )
			{
				if( ImeUi_IsVerticalCand() )
				{
					// Vertical candidate window

					// Compute the row the click is on
					int nRow = ( pt.y - s_CandList.rcCandidate.top ) / pFont->nHeight;

					if( nRow < ( int )ImeUi_GetCandidateCount() )
					{
						// nRow is a valid entry.
						// Now emulate keystrokes to select the candidate at this row.
						switch( GetPrimaryLanguage() )
						{
						case LANG_CHINESE:
						case LANG_KOREAN:
							// For Chinese and Korean, simply send the number keystroke.
							SendKey( ( BYTE )( '0' + nRow + 1 ) );
							break;

						case LANG_JAPANESE:
							// For Japanese, move the selection to the target row,
							// then send Right, then send Left.

							BYTE nVirtKey;
							if( nRow > ( int )ImeUi_GetCandidateSelection() )
								nVirtKey = VK_DOWN;
							else
								nVirtKey = VK_UP;
							int nNumToHit = abs( int( nRow - ImeUi_GetCandidateSelection() ) );
							for( int nStrike = 0; nStrike < nNumToHit; ++nStrike )
								SendKey( nVirtKey );

							// Do this to close the candidate window without ending composition.
							SendKey( VK_RIGHT );
							SendKey( VK_LEFT );

							break;
						}
					}
				}
				else
				{
					// Horizontal candidate window

					// Determine which the character the click has hit.
					int nCharHit;
					int nTrail;
					s_CandList.HoriCand.XtoCP( pt.x - s_CandList.rcCandidate.left, &nCharHit, &nTrail );

					// Determine which candidate string the character belongs to.
					int nCandidate = ImeUi_GetCandidateCount() - 1;

					int nEntryStart = 0;
					for( UINT i = 0; i < ImeUi_GetCandidateCount(); ++i )
					{
						if( nCharHit >= nEntryStart )
						{
							// Haven't found it.
							nEntryStart += lstrlenW( ImeUi_GetCandidate( i ) ) + 1;  // plus space separator
						}
						else
						{
							// Found it.  This entry starts at the right side of the click point,
							// so the char belongs to the previous entry.
							nCandidate = i - 1;
							break;
						}
					}

					// Now emulate keystrokes to select the candidate entry.
					switch( GetPrimaryLanguage() )
					{
					case LANG_CHINESE:
					case LANG_KOREAN:
						// For Chinese and Korean, simply send the number keystroke.
						SendKey( ( BYTE )( '0' + nCandidate + 1 ) );
						break;
					}
				}

				return true;
			}
		}
	}


	// If we didn't care for the msg, let the parent process it.
	//  [6/16/2008 Psj] 멀티라인에 맞게 바꾸어준다.
	return BaseHandleMouse(  uMsg,  pt,  wParam,  lParam );
}

bool CTextBoxCtrl::BaseHandleMouse( UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam )
{
	if( !m_bEnabled || !m_bVisible )
		return false;

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		{
			if( !m_bHasFocus )
				m_pDialog->RequestFocus( this );

			if( !ContainsPoint( pt ) )
				return false;

			m_bMouseDrag = true;
			SetCapture( DXUTGetHWND() );
			// Determine the character corresponding to the coordinates.
			int nCP, nTrail, nX1st;
			m_Buffer.CPtoX( m_nFirstVisible, FALSE, &nX1st );  // X offset of the 1st visible char

			DXUTFontNode* pFont = m_pDialog->GetFont( m_Elements.GetAt( 0 )->iFont );
			if( SUCCEEDED( m_Buffer.XtoCP( pt.x - m_rcText.left , &nCP, &nTrail ) ) )
			{
				
				
				//  [6/17/2008 Psj] NewLine과 글자 부분이 형성된  라인 부분만 클릭할수 있게한다.
				int nLine = 0;
				if((pt.y-m_rcText.top)/pFont->nHeight > (int)(m_vStrLineInfo.size()-1))
				{
					nLine = (int)m_vStrLineInfo.size()-1;
					SetCaretPosNewLine(nLine,false,true);
				}
				else
				{
					nLine = ((pt.y-m_rcText.top)/pFont->nHeight);
					SetCaretPosNewLine(nLine,false,true);
					nLine = nLine +m_ScrollBar.GetTrackPos();
				}

				//  [6/19/2008 Psj] 클릭할때 커서위치에 캐럿이 있게한다.
				//SetCaretPosNewLine(nLine,false,true);
				m_nCaretNowCol = nLine; // 지금 캐럿위치를 넘겨줌

				//  [6/17/2008 Psj] 길이만큼의 들어갈 문자 수를 알려줌
				//  fix 마지막 문자가 무엇이 들어가는지 알아내어  caret 위치 선정
				m_Buffer.XtoCP(pt.x - m_rcText.left ,&nCP, &nTrail);
				
				if(nLine==0)
				{
					m_nbuffWordSiz = 0;
				}

				//  [6/19/2008 Psj] 마우스를 클릭할때 캐럿위치를 계산해준다.
				//if(nLine>0)
				//{	
					
				if(pt.y>( m_rcText.top  ))
				{
					HDC dc = pFont->pFont9->GetDC();
					int nPass; //그냥 넘어가는 값

					m_nbuffWordSiz=0;
					for(int i=0;i<nLine;i++)
					{
						m_nbuffWordSiz = m_nbuffWordSiz+m_vStrLineInfo[i].nTextW+m_vStrLineInfo[i].nEnterW;
						int nLineMaxW=0;
						m_Buffer.XtoCP(m_nbuffWordSiz,&nLineMaxW,&nPass);
					}

					if(pt.x- m_rcText.left>m_vStrLineInfo[nLine].nTextW)
					{
						m_Buffer.XtoCP(m_nbuffWordSiz+m_vStrLineInfo[nLine].nTextW ,&nCP, &nTrail);
					}
					else
					{
						m_Buffer.XtoCP(m_nbuffWordSiz+pt.x- m_rcText.left ,&nCP, &nTrail);
					}

					//// Cap at the NULL character.
					if( nTrail && nCP < m_Buffer.GetTextSize() )
						PlaceCaret( nCP + 1 );
					else
						PlaceCaret( nCP );
					m_nSelStart = m_nCaret;
					ResetCaretBlink();


				}
				
				
			}
			return true;
		}

	case WM_LBUTTONUP:
		ReleaseCapture();
		m_bMouseDrag = false;
		break;

	case WM_RBUTTONDOWN:
		{
			int qwdqw =0;

		}
		break;

	case WM_MOUSEMOVE:
		if( m_bMouseDrag )
		{
			//// Determine the character corresponding to the coordinates.
			//int nCP, nTrail, nX1st;
			//m_Buffer.CPtoX( m_nFirstVisible, FALSE, &nX1st );  // X offset of the 1st visible char
			//if( SUCCEEDED( m_Buffer.XtoCP( pt.x - m_rcText.left + nX1st, &nCP, &nTrail ) ) )
			//{
			//	// Cap at the NULL character.
			//	if( nTrail && nCP < m_Buffer.GetTextSize() )
			//		PlaceCaret( nCP + 1 );
			//	else
			//		PlaceCaret( nCP );
			//}
		}
		break;
	}
	return false;

}

WCHAR CTextBoxCtrl::GetLineStrEnd(int nLine)
{
	if(nLine<0 || m_vStrLineInfo[nLine].nTextW==0) return 0;



	int nPass;
	int nLineMaxW=0;
	int nbuffSiz=0;
	for(int i=0;i<=nLine;i++)
	{
		nbuffSiz = nbuffSiz+m_vStrLineInfo[i].nTextW;
	}
	m_Buffer.XtoCP(nbuffSiz,&nLineMaxW,&nPass);
	return m_Buffer[nLineMaxW-1];
}


void CTextBoxCtrl::UpdateRects()
{
	//기본 업데이트 Rect 불러준다.
	CDXUTIMEEditBox::UpdateRects();


	//m_rcText
	// Update the scrollbar's rects
	int nSideW = m_rcBoundingBox.right - m_rcText.right;
	m_ScrollBar.SetLocation( m_rcRender->right , m_rcRender->top );
	

	m_ScrollBar.SetSize( m_nSBWidth, m_rcRender->bottom- m_rcRender->top);

	DXUTFontNode* pFont = m_pDialog->GetFont( m_Elements.GetAt( 0 )->iFont );
	if( pFont && pFont->nHeight )
	{
	
		m_ScrollBar.SetPageSize( RectHeight( m_rcRender[0] ) / pFont->nHeight );

		// The selected item may have been scrolled off the page.
		// Ensure that it is in page again.
		m_ScrollBar.ShowItem( m_nCaretNowCol );
	}


}


bool CTextBoxCtrl::HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// Let the scroll bar have a chance to handle it first
	if( m_ScrollBar.HandleKeyboard( uMsg, wParam, lParam ) )
		return true;

	return CDXUTIMEEditBox::HandleKeyboard(uMsg,wParam,lParam);
}


void CTextBoxCtrl::SetScrollRange()
{
	m_ScrollBar.SetTrackRange( 0,(int)m_vStrLineInfo.size()+1 );
}

void CTextBoxCtrl::SetScrollPosMovCaretPos()
{
	if(!GetScrollPosToCaretShow())
	{
		if(m_ScrollBar.GetTrackPos() > m_nCaretNowCol)
		{
			
			m_ScrollBar.SetTrackPos(m_nCaretNowCol);
		}
		else
		{
			m_ScrollBar.SetTrackPos(m_nCaretNowCol-m_ScrollBar.GetPageSize()+2);

		}
		SetCaretPosNewLine(m_nCaretNowCol - m_ScrollBar.GetTrackPos(),false,true);
	}
}