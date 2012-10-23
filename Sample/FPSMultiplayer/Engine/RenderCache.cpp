//-----------------------------------------------------------------------------
// RenderCache.h implementation.
// Refer to the RenderCache.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "Engine.h"

//-----------------------------------------------------------------------------
// The render cache class constructor.
//-----------------------------------------------------------------------------
RenderCache::RenderCache( IDirect3DDevice9 *device, Material *material )
{
	m_device = device;

	m_material = material;

	m_indexBuffer = NULL;
	m_totalIndices = 0;
}

//-----------------------------------------------------------------------------
// The render cache class destructor.
//-----------------------------------------------------------------------------
RenderCache::~RenderCache()
{
	SAFE_RELEASE( m_indexBuffer );
}

//-----------------------------------------------------------------------------
// Increases the size of the render cache to manage another face.
//-----------------------------------------------------------------------------
void RenderCache::AddFace()
{
	m_totalIndices += 3;
}

//-----------------------------------------------------------------------------
// Prepares the render cache for use. Must be called after setting the size.
//-----------------------------------------------------------------------------
void RenderCache::Prepare( unsigned long totalVertices )
{
	// Set the total vertices that are rendered.
	m_totalVertices = totalVertices;

	// Create the render cache's index buffer.
	m_device->CreateIndexBuffer( m_totalIndices * sizeof( unsigned short ), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_indexBuffer, NULL );
}

//-----------------------------------------------------------------------------
// Informs the render cache that rendering is about to begin.
//-----------------------------------------------------------------------------
void RenderCache::Begin()
{
	m_indexBuffer->Lock( 0, 0, (void**)&m_indexPointer, 0 );

	m_faces = 0;
}

//-----------------------------------------------------------------------------
// Adds the given face indicies to be rendered.
//-----------------------------------------------------------------------------
void RenderCache::RenderFace( unsigned short vertex0, unsigned short vertex1, unsigned short vertex2 )
{
	*m_indexPointer++ = vertex0;
	*m_indexPointer++ = vertex1;
	*m_indexPointer++ = vertex2;

	m_faces++;
}

//-----------------------------------------------------------------------------
// Informs the render cache that rendering is finished, so render the faces.
//-----------------------------------------------------------------------------
void RenderCache::End()
{
	// Unlock the index buffer.
	m_indexBuffer->Unlock();

	// Check if there are any faces to render.
	if( m_faces == 0 )
		return;

	// Check if the material should ignore fog.
	if( m_material->GetIgnoreFog() == true )
		m_device->SetRenderState( D3DRS_FOGENABLE, false );

	// Set the material and texture.
	m_device->SetMaterial( m_material->GetLighting() );
	m_device->SetTexture( 0, m_material->GetTexture() );

	// Set the indicies to render the correct faces.
	m_device->SetIndices( m_indexBuffer );

	// Render all the faces.
	m_device->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, m_totalVertices, 0, m_faces );

	// Restore the fog setting if it was changed.
	if( m_material->GetIgnoreFog() == true )
		m_device->SetRenderState( D3DRS_FOGENABLE, true );
}

//-----------------------------------------------------------------------------
// Returns a pointer to the material being used by this render cache.
//-----------------------------------------------------------------------------
Material *RenderCache::GetMaterial()
{
	return m_material;
}