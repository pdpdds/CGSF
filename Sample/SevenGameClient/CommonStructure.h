#pragma once
#include "d3d9.h"
#include "d3d9types.h"
#include "d3dx9math.h"

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

struct CUSTOMVERTEX
{

  D3DXVECTOR3 position; // The position
  D3DCOLOR    color;    // The color
  FLOAT       tu, tv;   // The texture coordinates
};

class membuf : public std::wstreambuf // <- !!!HERE!!! 
{    
public:        

	membuf(wchar_t* p, size_t n) 
	{ // <- !!!HERE!!!         
		setg(p, p, p + n);     
	} 
}; 