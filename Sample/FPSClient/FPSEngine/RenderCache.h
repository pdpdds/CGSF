//-----------------------------------------------------------------------------
// Manages the rendering of indexed faces from a set of vertices.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef RENDER_CACHE_H
#define RENDER_CACHE_H

//-----------------------------------------------------------------------------
// Render Cache Class
//-----------------------------------------------------------------------------
class RenderCache
{
public:
	RenderCache( IDirect3DDevice9 *device, Material *material );
	virtual ~RenderCache();

	void AddFace();
	void Prepare( unsigned long totalVertices );

	void Begin();
	void RenderFace( unsigned short vertex0, unsigned short vertex1, unsigned short vertex2 );
	void End();

	Material *GetMaterial();

private:
	IDirect3DDevice9 *m_device; // Pointer to a Direct3D device.
	Material *m_material; // Pointer to the material used by this render cache.

	IDirect3DIndexBuffer9 *m_indexBuffer; // Index buffer pointing to the vertices to render.
	unsigned short *m_indexPointer; // Pointer for accessing the index buffer.
	unsigned long m_totalIndices; // Total number of indices this render cache can handle.
	unsigned long m_faces; // Total number of faces to be rendered.

	unsigned long m_totalVertices; // Total number of vertices.
};

#endif