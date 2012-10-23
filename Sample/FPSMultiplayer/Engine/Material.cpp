//-----------------------------------------------------------------------------
// Material.h implementation.
// Refer to the Material.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "Engine.h"

//-----------------------------------------------------------------------------
// The material class constructor.
//-----------------------------------------------------------------------------
Material::Material( char *name, char *path ) : Resource< Material >( name, path )
{
	D3DXIMAGE_INFO info;

	// Load the script for this material.
	Script *script = new Script( name, path );

	// Check if the material's texture has transparency.
	if( script->GetColourData( "transparency" )->a == 0.0f )
	{
		// Load the texture without transparency.
		D3DXCreateTextureFromFileEx( g_engine->GetDevice(), script->GetStringData( "texture" ), D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_TRIANGLE, D3DX_FILTER_TRIANGLE, 0, &info, NULL, &m_texture );
	}
	else
	{
		// Load the texture using a transparency colour value.
		D3DCOLORVALUE *colour = script->GetColourData( "transparency" );
		D3DCOLOR transparency = D3DCOLOR_COLORVALUE( colour->r, colour->g, colour->b, colour->a );
		D3DXCreateTextureFromFileEx( g_engine->GetDevice(), script->GetStringData( "texture" ), D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_TRIANGLE, D3DX_FILTER_TRIANGLE, transparency, &info, NULL, &m_texture );
	}

	// Store the width and height of the texture.
	m_width = info.Width;
	m_height = info.Height;

	// Set the material's lighting properties.
	m_lighting.Diffuse = *script->GetColourData( "diffuse" );
	m_lighting.Ambient = *script->GetColourData( "ambient" );
	m_lighting.Specular = *script->GetColourData( "specular" );
	m_lighting.Emissive = *script->GetColourData( "emissive" );
	m_lighting.Power = *script->GetFloatData( "power" );

	// Set the ignore face flag.
	m_ignoreFace = *script->GetBoolData( "ignore_face" );

	// Set the ignore fog flag.
	m_ignoreFog = *script->GetBoolData( "ignore_fog" );

	// Set the ignore ray flag.
	m_ignoreRay = *script->GetBoolData( "ignore_ray" );

	// Destory the material's script.
	SAFE_DELETE( script );
}

//-----------------------------------------------------------------------------
// The material class destructor.
//-----------------------------------------------------------------------------
Material::~Material()
{
	SAFE_RELEASE( m_texture );
}

//-----------------------------------------------------------------------------
// Returns the material's texture.
//-----------------------------------------------------------------------------
IDirect3DTexture9 *Material::GetTexture()
{
	return m_texture;
}

//-----------------------------------------------------------------------------
// Returns the material's lighting properties.
//-----------------------------------------------------------------------------
D3DMATERIAL9 *Material::GetLighting()
{
	return &m_lighting;
}

//-----------------------------------------------------------------------------
// Returns the width of the material's texture.
//-----------------------------------------------------------------------------
unsigned long Material::GetWidth()
{
	return m_width;
}

//-----------------------------------------------------------------------------
// Returns the height of the material's texture.
//-----------------------------------------------------------------------------
unsigned long Material::GetHeight()
{
	return m_height;
}

//-----------------------------------------------------------------------------
// Returns the material's ignore face flag.
//-----------------------------------------------------------------------------
bool Material::GetIgnoreFace()
{
	return m_ignoreFace;
}

//-----------------------------------------------------------------------------
// Returns the material's ignore fog flag.
//-----------------------------------------------------------------------------
bool Material::GetIgnoreFog()
{
	return m_ignoreFog;
}

//-----------------------------------------------------------------------------
// Returns the material's ignore ray flag.
//-----------------------------------------------------------------------------
bool Material::GetIgnoreRay()
{
	return m_ignoreRay;
}