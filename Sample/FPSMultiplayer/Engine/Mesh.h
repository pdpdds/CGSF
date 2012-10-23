//-----------------------------------------------------------------------------
// Used to load and manage static and animated meshes.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef MESH_H
#define MESH_H

//-----------------------------------------------------------------------------
// Frame Structure
//-----------------------------------------------------------------------------
struct Frame : public D3DXFRAME
{
	D3DXMATRIX finalTransformationMatrix; // Frame's final transformation after being combined with its parent.

	//-------------------------------------------------------------------------
	// Returns the frame's translation.
	//-------------------------------------------------------------------------
	D3DXVECTOR3 GetTranslation()
	{
		return D3DXVECTOR3( finalTransformationMatrix._41, finalTransformationMatrix._42, finalTransformationMatrix._43 );
	}
};

//-----------------------------------------------------------------------------
// Mesh Container Structure
//-----------------------------------------------------------------------------
struct MeshContainer : public D3DXMESHCONTAINER
{
	char **materialNames; // Temporary array of material (texture) names.
	Material **materials; // Array of materials used by the mesh container.
	ID3DXMesh *originalMesh; // Original mesh.
	D3DXATTRIBUTERANGE *attributeTable; // Mesh's attribute table.
	unsigned long totalAttributeGroups; // Total number of attribute groups.
	D3DXMATRIX **boneMatrixPointers; // Array of pointers to the bone transformation matrices.
};

//-----------------------------------------------------------------------------
// Allocate Hierarchy Class
//-----------------------------------------------------------------------------
class AllocateHierarchy : public ID3DXAllocateHierarchy
{
	STDMETHOD( CreateFrame )( THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame );
	STDMETHOD( CreateMeshContainer )( THIS_ LPCSTR Name, CONST D3DXMESHDATA *pMeshData, CONST D3DXMATERIAL *pMaterials, CONST D3DXEFFECTINSTANCE *pEffectInstances, DWORD NumMaterials, CONST DWORD *pAdjacency, LPD3DXSKININFO pSkinInfo, LPD3DXMESHCONTAINER *ppNewMeshContainer );
	STDMETHOD( DestroyFrame )( THIS_ LPD3DXFRAME pFrameToFree );
	STDMETHOD( DestroyMeshContainer )( THIS_ LPD3DXMESHCONTAINER pMeshContainerToFree );
};

//-----------------------------------------------------------------------------
// Mesh Class
//-----------------------------------------------------------------------------
class Mesh : public BoundingVolume, public Resource< Mesh >
{
public:
	Mesh( char *name, char *path = "./" );
	virtual ~Mesh();

	void Update();
	void Render();

	void CloneAnimationController( ID3DXAnimationController **animationController );

	MeshContainer *GetStaticMesh();
	Vertex *GetVertices();
	unsigned short *GetIndices();

	LinkedList< Frame > *GetFrameList();
	Frame *GetFrame( char *name );
	Frame *GetReferencePoint( char *name );

private:
	void PrepareFrame( Frame *frame );
	void UpdateFrame( Frame *frame, D3DXMATRIX *parentTransformationMatrix = NULL );
	void RenderFrame( Frame *frame );

private:
	Frame *m_firstFrame; // First frame in the mesh's frame hierarchy.
	ID3DXAnimationController *m_animationController; // Animation controller.

	D3DXMATRIX *m_boneMatrices; // Array of bone transformation matrices.
	unsigned long m_totalBoneMatrices; // Number of bones in the array.

	MeshContainer *m_staticMesh; // A static (non-animated) version of the mesh.
	Vertex *m_vertices; // Array of vertices from the static mesh.
	unsigned short *m_indices; // Array of indices into the vertex array.

	LinkedList< Frame > *m_frames; // Linked list of pointers to all the frames in the mesh.
	LinkedList< Frame > *m_refPoints; // Linked list of pointers to all the reference points in the mesh.
};

#endif