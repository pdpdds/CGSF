//-----------------------------------------------------------------------------
// A material which consists of a texture and lighting properties.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef MATERIAL_H
#define MATERIAL_H

//-----------------------------------------------------------------------------
// Material Class
//-----------------------------------------------------------------------------
class Material : public Resource< Material >
{
public:
	Material( char *name, char *path = "./" );
	virtual ~Material();

	IDirect3DTexture9 *GetTexture();
	D3DMATERIAL9 *GetLighting();
	unsigned long GetWidth();
	unsigned long GetHeight();
	bool GetIgnoreFace();
	bool GetIgnoreFog();
	bool GetIgnoreRay();

private:
	IDirect3DTexture9 *m_texture; // Direct3D texture.
	D3DMATERIAL9 m_lighting; // Lighting properties.
	unsigned long m_width; // Width of the texture.
	unsigned long m_height; // Height of the texture.
	bool m_ignoreFace; // Indicates if faces with this material should be ignored.
	bool m_ignoreFog; // Indicates if this material is to ignore fog when rendered.
	bool m_ignoreRay; // Indicates if this material is to ignore ray intersections.
};

#endif