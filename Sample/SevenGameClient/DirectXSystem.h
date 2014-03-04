#pragma once
#include "d3d9.h"
#include "d3dx9core.h"
#include "FontObject.h"
#include "Renderer.h"
#include "PictureBoxMgr.h"

class SGManager;
class CTextureMgr;
class CPictureBox;

class CDirectXSystem : public CRenderer
{
public:
	CDirectXSystem(void);
	virtual ~CDirectXSystem(void);

	BOOL Initialize();
	int Run();

	BOOL InitEnvironment();

	BOOL GetFullFilePath(TCHAR** szFileName, TCHAR* szFileExt);

	BOOL OnRender(float fElapsedTime);
	BOOL OnRender( float fElapsedTime, int iImageIndex);
	void OnMove( double fTime, float fElapsedTime);
	void LostDevice();

	BOOL ProcessMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL ResetDevice( IDirect3DDevice9* pd3dDevice);

	HRESULT Create3DSprite(LPDIRECT3DTEXTURE9 &Test_Texture,LPDIRECT3DVERTEXBUFFER9 &Test_Buffer,LPCTSTR lpstr, int alpha);
	HRESULT CreateTexture(LPDIRECT3DTEXTURE9 &Test_Texture, LPCTSTR szTextureName);
	BOOL RenderRegion(int iTextureIdentifer,int x,int y,int align, int Col, int Row, int _x, int _y, int border, BOOL Transparent);

	static CDirectXSystem* GetInstance()
	{
		if (NULL == m_pDirectXSystem)
		{
			m_pDirectXSystem = new CDirectXSystem();
		}
		return m_pDirectXSystem;
	}

	int GetWidth(){return m_iWidth;}
	int GetHeight(){return m_iHeight;}

	void SetWidth(int iWidth){m_iWidth = iWidth;}
	void SetHeight(int iHeight){m_iHeight = iHeight;}

	void SetD3DDevice(IDirect3DDevice9* pd3dDevice){m_pd3dDevice = pd3dDevice;}
	IDirect3DDevice9* GetD3DDevice(){return m_pd3dDevice;}

	BOOL DrawCard(float fElapsedTime);

	BOOL DrawCardTurn( float fElapsedTime );
	BOOL DrawWinLose( float fElapsedTime );
	BOOL DrawPassButton( float fElapsedTime );
	BOOL ResetFireEffect();

	virtual BOOL OnRenderRegion(float fElapsedTime, 
		int	  iImageIndex,	
		float fPosX, 
		float fPosY, 
		float fColRatio, 
		float fRowRatio,
		float fColUnitRatio, 
		float fRowUnitRatio,
		float fRegionWidth, 
		float fRegionHeight) override;

	virtual BOOL OnRender(float fElapsedTime, 
		int	  iImageIndex,
		float fPosX, 
		float fPosY) override;

protected:

private:
	IDirect3DDevice9* m_pd3dDevice;

	int m_iWidth;
	int m_iHeight;

	static CDirectXSystem* m_pDirectXSystem;

	ID3DXSprite*            m_pD3DXSprite;
	CTextureMgr* m_pTextureMgr;

	CFontObject m_Font;
	CPictureBoxMgr m_PictureBoxMgr;
	CPictureBox* m_pBackGround;
};
