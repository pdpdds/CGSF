#include "StdAfx.h"
#include "FontObject.h"
#include "DirectXSystem.h"
//#include "dxstdafx.h"

CFontObject::CFontObject(void)
{
	m_pFont = NULL;
}

CFontObject::~CFontObject(void)
{
}

void CFontObject::Print(TCHAR* szText, RECT& rc, int iFlag, D3DXCOLOR& Color )
{
	m_pFont->DrawText( NULL, szText, -1, &rc, iFlag, Color);
}

BOOL CFontObject::OnRender( float fElapsedTime )
{
	return TRUE;

}

BOOL CFontObject::OnResetDevice()
{
	if(m_pFont)
		m_pFont->OnResetDevice();

	return TRUE;
}

BOOL CFontObject::OnFrameMove( double fTime, float fElapsedTime )
{

	return TRUE;
}

BOOL CFontObject::OnLostDevice()
{
	if( m_pFont )
		m_pFont->OnLostDevice();

	return TRUE;

}

BOOL CFontObject::OnDestroyDevice()
{
	SAFE_RELEASE(m_pFont);

	return TRUE;
}

BOOL CFontObject::Initialize( D3DXFONT_DESC* desc )
{
	m_desc = *desc; 
	D3DXCreateFontIndirect(CDirectXSystem::GetInstance()->GetD3DDevice(), &m_desc, &m_pFont);

	return TRUE;
}