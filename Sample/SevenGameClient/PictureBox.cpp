#include "StdAfx.h"
//#include "dxstdafx.h"
#include "PictureBox.h"
#include "DirectXSystem.h"

CPictureBox::CPictureBox(void)
{
	m_pVBGameTitle = NULL;
	m_pTextureGameTitle = NULL;
}

CPictureBox::~CPictureBox(void)
{

}

BOOL CPictureBox::OnRender(float fElapsedTime)
{
	D3DXMATRIX mat;  

	if(m_pTextureGameTitle == NULL)
		return FALSE;

	IDirect3DDevice9* pd3dDevice = CDirectXSystem::GetInstance()->GetD3DDevice();
	float fWidth = (float)CDirectXSystem::GetInstance()->GetWidth();
	float fHeight = (float)CDirectXSystem::GetInstance()->GetHeight();

	D3DXMatrixOrthoLH(&mat, fWidth, fHeight, 0.0, 1000.0);  

	pd3dDevice->SetTransform( D3DTS_PROJECTION, &mat );    

	D3DXMATRIX matWorld,matTrans,matScale, matScale2;

	D3DSURFACE_DESC Desc;
	m_pTextureGameTitle->GetLevelDesc(0, &Desc);

	D3DXMatrixScaling(&matScale, (float)(m_Info.iWidth/2.0f), (float)(m_Info.iHeight/2.0f), 1.0);


	//D3DXMatrixMultiply(&matScale, &matScale2, matScale2);  
	D3DXMatrixMultiply(&matWorld, &matScale, D3DXMatrixTranslation(&matTrans,  
		(float)(m_Info.iWidth/2.0f) - (float)(fWidth/2.0f) + (float)m_Info.iPosX, -(float)(m_Info.iHeight/2.0f) + (float)(fHeight/2.0f) - m_Info.iPosY,0));  

	pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );    

	pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

	pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);     

	pd3dDevice->SetTexture( 0, m_pTextureGameTitle );

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );    

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );    

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );    

	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );		

	pd3dDevice->SetStreamSource( 0, m_pVBGameTitle,0, sizeof(CUSTOMVERTEX) );    

	pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );

	pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 2 );
 	

	return TRUE;
}

BOOL CPictureBox::OnRender( float fElapsedTime, float fPosX, float fPosY )
{
	D3DXMATRIX mat;  

	if(m_pTextureGameTitle == NULL)
		return FALSE;

	IDirect3DDevice9* pd3dDevice = CDirectXSystem::GetInstance()->GetD3DDevice();
	float fWidth = (float)CDirectXSystem::GetInstance()->GetWidth();
	float fHeight = (float)CDirectXSystem::GetInstance()->GetHeight();

	D3DXMatrixOrthoLH(&mat, fWidth, fHeight, 0.0, 1000.0);  

	pd3dDevice->SetTransform( D3DTS_PROJECTION, &mat );    

	D3DXMATRIX matWorld,matTrans,matScale, matScale2;

	D3DSURFACE_DESC Desc;
	m_pTextureGameTitle->GetLevelDesc(0, &Desc);

	D3DXMatrixScaling(&matScale, (float)(m_Info.iWidth/2.0f), (float)(m_Info.iHeight/2.0f), 1.0);


	//D3DXMatrixMultiply(&matScale, &matScale2, matScale2);  
	D3DXMatrixMultiply(&matWorld, &matScale, D3DXMatrixTranslation(&matTrans,  
		(float)(m_Info.iWidth/2.0f) - (float)(fWidth/2.0f) + fPosX, -(float)(m_Info.iHeight/2.0f) + (float)(fHeight/2.0f) - fPosY,0));  

	pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );    

	pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

	pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);     

	pd3dDevice->SetTexture( 0, m_pTextureGameTitle );

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );    

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );    

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );    

	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );		

	pd3dDevice->SetStreamSource( 0, m_pVBGameTitle,0, sizeof(CUSTOMVERTEX) );    

	pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );

	pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 2 );	

	return TRUE;
}

BOOL CPictureBox::OnRenderRegion(float fElapsedTime, 
								 float fPosX, 
								 float fPosY, 
								 float fColRatio, 
								 float fRowRatio,
								 float fColUnitRatio, 
								 float fRowUnitRatio,
								 float fRegionWidth, 
								 float fRegionHeight)
{
	HRESULT hr;
	D3DXMATRIX mat;  

	if(m_pTextureGameTitle == NULL)
		return FALSE;

	IDirect3DDevice9* pd3dDevice = CDirectXSystem::GetInstance()->GetD3DDevice();
	float fWidth = (float)CDirectXSystem::GetInstance()->GetWidth();
	float fHeight = (float)CDirectXSystem::GetInstance()->GetHeight();

	D3DXMatrixOrthoLH(&mat, fWidth, fHeight, 0.0, 1000.0);  

	pd3dDevice->SetTransform( D3DTS_PROJECTION, &mat );    

	D3DXMATRIX matWorld,matTrans,matScale, matScale2;

	D3DSURFACE_DESC Desc;
	m_pTextureGameTitle->GetLevelDesc(0, &Desc);

	CUSTOMVERTEX* pVertices;  

	// got it got it got it got it  
	if( FAILED( hr = m_pVBGameTitle->Lock( 0, 6*sizeof(CUSTOMVERTEX), (void**)&pVertices, 0 ) ) )
		return false;

	pVertices[0].tu       = fColRatio;
	pVertices[0].tv       = fRowRatio;
	pVertices[1].tu       = fColRatio + fColUnitRatio;
	pVertices[1].tv       = fRowRatio;
	pVertices[2].tu       = fColRatio + fColUnitRatio;
	pVertices[2].tv       = fRowRatio + fRowUnitRatio;

	pVertices[3].tu       = fColRatio;
	pVertices[3].tv       = fRowRatio;
	pVertices[4].tu       = fColRatio + fColUnitRatio;
	pVertices[4].tv       = fRowRatio + fRowUnitRatio;

	pVertices[5].tu       = fColRatio;
	pVertices[5].tv       = fRowRatio + fRowUnitRatio;

	if( FAILED( hr = m_pVBGameTitle->Unlock() ) ) return FALSE;



	D3DXMatrixScaling(&matScale, (float)(fRegionWidth/2.0f), (float)(fRegionHeight/2.0f), 1.0);


	//D3DXMatrixMultiply(&matScale, &matScale2, matScale2);  
	D3DXMatrixMultiply(&matWorld, &matScale, D3DXMatrixTranslation(&matTrans,  
		(float)(fRegionWidth/2.0f) - (float)(fWidth/2.0f) + fPosX, -(float)(fRegionHeight/2.0f) + (float)(fHeight/2.0f) - fPosY, 0));  

	pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );    

	pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

	pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);     

	pd3dDevice->SetTexture( 0, m_pTextureGameTitle );

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );    

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );    

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );    

	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );		

	pd3dDevice->SetStreamSource( 0, m_pVBGameTitle,0, sizeof(CUSTOMVERTEX) );    

	pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );

	pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 2 );			
 	
	return TRUE;
}

BOOL CPictureBox::OnResetDevice()
{
	SAFE_RELEASE(m_pVBGameTitle);	
	SAFE_RELEASE(m_pTextureGameTitle);
	CDirectXSystem::GetInstance()->Create3DSprite(m_pTextureGameTitle,m_pVBGameTitle,m_Info.szGUICaption, 0);

	return TRUE;
}

BOOL CPictureBox::OnFrameMove( double fTime, float fElapsedTime )
{
	return TRUE;
}

BOOL CPictureBox::OnLostDevice()
{
	return TRUE;
}

BOOL CPictureBox::OnDestroyDevice()
{
	return TRUE;
}