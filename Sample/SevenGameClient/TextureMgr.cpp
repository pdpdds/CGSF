#include "StdAfx.h"
#include "TextureMgr.h"
#include "DirectXSystem.h"

CTextureMgr::CTextureMgr(void)
{
}

CTextureMgr::~CTextureMgr(void)
{
}

BOOL CTextureMgr::AddTexture(int iIdentifier, TCHAR* szTextureName)
{	
	mapTexture::iterator iter = m_mapTexture.find(iIdentifier);

	if(iter != m_mapTexture.end())
		return FALSE;

	LPDIRECT3DTEXTURE9 pTexture;
	HRESULT hr;
	if (FAILED(hr =CDirectXSystem::GetInstance()->CreateTexture(pTexture, szTextureName)))
		return FALSE;

	
	m_mapTexture.insert(mapTexture::value_type(iIdentifier, pTexture));

	return TRUE;
}

LPDIRECT3DTEXTURE9 CTextureMgr::GetTexture( int iIdentifier )
{
	mapTexture::iterator iter = m_mapTexture.find(iIdentifier);

	if(iter == m_mapTexture.end())
		return NULL;

	return iter->second;
}