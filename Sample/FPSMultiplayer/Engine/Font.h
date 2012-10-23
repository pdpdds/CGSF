//-----------------------------------------------------------------------------
// A font render that draws the characters using textured quads.
//
// Note: This is just a cutdown, modified version of the CD3DFont class. 
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef FONT_H
#define FONT_H

//-----------------------------------------------------------------------------
// Font Class
//-----------------------------------------------------------------------------
class Font
{
public:
	Font( char *name = "Arial", short size = 10, unsigned long bold = FW_NORMAL, bool italic = false );
	virtual ~Font();

	void Render( char *text, float x, float y, D3DCOLOR colour = D3DCOLOR_COLORVALUE( 1.0f, 1.0f, 1.0f, 1.0f ) );

private:
	bool PrepareFont( HDC hDC, bool measure = false );

private:
	IDirect3DStateBlock9 *m_states; // State block used for restoring the render states.
	IDirect3DVertexBuffer9 *m_vb; // Vertex buffer for rendering the text.
	IDirect3DTexture9 *m_texture; // Direct3D texture for the font.
	unsigned long m_textureWidth; // Width of the texture.
	unsigned long m_textureHeight; // Height of the texture.
	float m_textureCoords[96][4]; // Character texture coordinates.
	short m_spacing; // Character pixel spacing per side.
};

#endif