#pragma once
#include "d3d9.h"

class CPictureBox : public CDirectXEntity
{
public:
	CPictureBox(void);
	virtual ~CPictureBox(void);

	virtual BOOL OnRender(float fElapsedTime) override;
	virtual BOOL OnResetDevice() override;
	virtual BOOL OnFrameMove( double fTime, float fElapsedTime) override;
	virtual BOOL OnLostDevice() override;
	virtual BOOL OnDestroyDevice() override;

	BOOL OnRenderRegion(float fElapsedTime, 
		float fPosX, 
		float fPosY, 
		float fColRatio, 
		float fRowRatio,
		float fColUnitRatio, 
		float fRowUnitRatio,
		float fRegionWidth, 
		float fRegionHeight);

	BOOL OnRender(float fElapsedTime, 
		float fPosX, 
		float fPosY);

	ComponentInfo m_Info;

protected:

private:
	LPDIRECT3DVERTEXBUFFER9 m_pVBGameTitle;
	ID3DXSprite*            m_pPictureSprite;
	LPDIRECT3DTEXTURE9  m_pTextureGameTitle;
};