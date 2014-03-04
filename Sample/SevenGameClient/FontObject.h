#pragma once
//#include "dxstdafx.h"

class CFontObject : public  CDirectXEntity
{
public:
	CFontObject(void);
	virtual ~CFontObject(void);

	BOOL Initialize(D3DXFONT_DESC* desc);
	void Print(TCHAR* szText, RECT& rc, int iFlag, D3DXCOLOR& Color );

	virtual BOOL OnRender(float fElapsedTime) override;
	virtual BOOL OnResetDevice() override;
	virtual BOOL OnFrameMove( double fTime, float fElapsedTime) override;
	virtual BOOL OnLostDevice() override;
	virtual BOOL OnDestroyDevice() override;

	void Print();//(TCHAR* szText, float fPosX, float fPosY, int R, int G, int B);

private:
	D3DXFONT_DESC m_desc;
	LPD3DXFONT m_pFont;
	RECT	m_Rect;
};
