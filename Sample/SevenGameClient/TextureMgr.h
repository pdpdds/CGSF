#pragma once

class CTextureMgr
{
	typedef std::map<int, LPDIRECT3DTEXTURE9> mapTexture;
public:
	CTextureMgr(void);
	virtual ~CTextureMgr(void);

	BOOL AddTexture(int iIdentifier, TCHAR* szTextureName);
	LPDIRECT3DTEXTURE9 GetTexture(int iIdentifier);

protected:

private:
	mapTexture m_mapTexture;
};
