//-----------------------------------------------------------------------------
// Mesh.h implementation.
// Refer to the Mesh.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "Engine.h"

//-----------------------------------------------------------------------------
// Creates a new frame.
//-----------------------------------------------------------------------------
HRESULT AllocateHierarchy::CreateFrame( THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame )
{
	// Create the new frame and zero its memory.
	Frame *frame = new Frame;
	ZeroMemory( frame, sizeof( Frame ) );

	// Copy the frame's name.
	if( Name == NULL )
	{
		// There is no name, so create a unique name.
		static unsigned long nameCount = 0;
		char newName[32];
		sprintf( newName, "unknown_frame_%d", nameCount );
		nameCount++;

		frame->Name = new char[strlen( newName ) + 1];
		strcpy( frame->Name, newName );
	}
	else
	{
		frame->Name = new char[strlen( Name ) + 1];
		strcpy( frame->Name, Name );
	}

	*ppNewFrame = frame;

	return S_OK;
}

//-----------------------------------------------------------------------------
// Creates a new mesh container.
//-----------------------------------------------------------------------------
HRESULT AllocateHierarchy::CreateMeshContainer( THIS_ LPCSTR Name, CONST D3DXMESHDATA *pMeshData, CONST D3DXMATERIAL *pMaterials, CONST D3DXEFFECTINSTANCE *pEffectInstances, DWORD NumMaterials, CONST DWORD *pAdjacency, LPD3DXSKININFO pSkinInfo, LPD3DXMESHCONTAINER *ppNewMeshContainer )
{
	// Create the new mesh container and zero its memory.
	MeshContainer *meshContainer = new MeshContainer;
	ZeroMemory( meshContainer, sizeof( MeshContainer ) );

	// Copy the mesh's name.
	if( Name == NULL )
	{
		// There is no name, so create a unique name.
		static unsigned long nameCount = 0;
		char newName[32];
		sprintf( newName, "unknown_mesh_%d", nameCount );
		nameCount++;

		meshContainer->Name = new char[strlen( newName ) + 1];
		strcpy( meshContainer->Name, newName );
	}
	else
	{
		meshContainer->Name = new char[strlen( Name ) + 1];
		strcpy( meshContainer->Name, Name );
	}

	// Check if the mesh has any materials.
	if( ( meshContainer->NumMaterials = NumMaterials ) > 0 )
	{
		// Allocate some memory for the mesh's materials, and their names (i.e. texture names).
		meshContainer->materials = new Material*[meshContainer->NumMaterials];
		meshContainer->materialNames = new char*[meshContainer->NumMaterials];

		// Store all the material (texture) names.
		for( unsigned long m = 0; m < NumMaterials; m++ )
		{
			if( pMaterials[m].pTextureFilename )
			{
				meshContainer->materialNames[m] = new char[strlen( pMaterials[m].pTextureFilename ) + 1];
				memcpy( meshContainer->materialNames[m], pMaterials[m].pTextureFilename, ( strlen( pMaterials[m].pTextureFilename ) + 1 ) * sizeof( char ) );
			}
			else
				meshContainer->materialNames[m] = NULL;

			meshContainer->materials[m] = NULL;
		}
	}

	// Store the mesh's adjacency information.
	meshContainer->pAdjacency = new DWORD[pMeshData->pMesh->GetNumFaces() * 3];
	memcpy( meshContainer->pAdjacency, pAdjacency, sizeof( DWORD ) * pMeshData->pMesh->GetNumFaces() * 3 );

	// Store the mesh data.
	meshContainer->MeshData.pMesh = meshContainer->originalMesh = pMeshData->pMesh;
	meshContainer->MeshData.Type = D3DXMESHTYPE_MESH;
	pMeshData->pMesh->AddRef();
	pMeshData->pMesh->AddRef();

	// Check if this mesh is a skinned mesh.
	if( pSkinInfo != NULL )
	{
		// Store the skin information and the mesh data.
		meshContainer->pSkinInfo = pSkinInfo;
		pSkinInfo->AddRef();

		// Clone the original mesh to create the skinned mesh.
		meshContainer->originalMesh->CloneMeshFVF( D3DXMESH_MANAGED, meshContainer->originalMesh->GetFVF(), g_engine->GetDevice(), &meshContainer->MeshData.pMesh );

		// Store the attribute table.
		meshContainer->MeshData.pMesh->GetAttributeTable( NULL, &meshContainer->totalAttributeGroups );
		meshContainer->attributeTable = new D3DXATTRIBUTERANGE[meshContainer->totalAttributeGroups];
		meshContainer->MeshData.pMesh->GetAttributeTable( meshContainer->attributeTable, NULL );
	}

	*ppNewMeshContainer = meshContainer;

	return S_OK;
}

//-----------------------------------------------------------------------------
// Destroys the given frame.
//-----------------------------------------------------------------------------
HRESULT AllocateHierarchy::DestroyFrame( THIS_ LPD3DXFRAME pFrameToFree )
{
	SAFE_DELETE_ARRAY( pFrameToFree->Name );
	SAFE_DELETE( pFrameToFree );

	return S_OK;
}

//-----------------------------------------------------------------------------
// Destroys the given mesh conatiner.
//-----------------------------------------------------------------------------
HRESULT AllocateHierarchy::DestroyMeshContainer( THIS_ LPD3DXMESHCONTAINER pMeshContainerToFree )
{
	MeshContainer *meshContainer = (MeshContainer*)pMeshContainerToFree;

	// Remove all of the mesh's materials from the material manager.
	for( unsigned long m = 0; m < meshContainer->NumMaterials; m++ )
		if( meshContainer->materials )
			g_engine->GetMaterialManager()->Remove( &meshContainer->materials[m] );

	// Destroy the mesh container.
	SAFE_DELETE_ARRAY( meshContainer->Name );
	SAFE_DELETE_ARRAY( meshContainer->pAdjacency );
	SAFE_DELETE_ARRAY( meshContainer->pMaterials );
	SAFE_DELETE_ARRAY( meshContainer->materialNames );
	SAFE_DELETE_ARRAY( meshContainer->materials );
	SAFE_DELETE_ARRAY( meshContainer->boneMatrixPointers );
	SAFE_DELETE_ARRAY( meshContainer->attributeTable );
	SAFE_RELEASE( meshContainer->MeshData.pMesh );
	SAFE_RELEASE( meshContainer->pSkinInfo );
	SAFE_RELEASE( meshContainer->originalMesh );
	SAFE_DELETE( meshContainer );

	return S_OK;
}

//-----------------------------------------------------------------------------
// The mesh class constructor.
//-----------------------------------------------------------------------------
Mesh::Mesh( char *name, char *path ) : Resource< Mesh >( name, path )
{
	// Create the list of reference points.
	m_frames = new LinkedList< Frame >;
	m_refPoints = new LinkedList< Frame >;

	// Load the mesh's frame hierarchy.
	AllocateHierarchy ah;
	D3DXLoadMeshHierarchyFromX( GetFilename(), D3DXMESH_MANAGED, g_engine->GetDevice(), &ah, NULL, (D3DXFRAME**)&m_firstFrame, &m_animationController );

	// Disable all the animation tracks initially.
	if( m_animationController != NULL )
		for( unsigned long t = 0; t < m_animationController->GetMaxNumTracks(); ++t )
			m_animationController->SetTrackEnable( t, false );

	// Invalidate the bone transformation matrices array.
	m_boneMatrices = NULL;
	m_totalBoneMatrices = 0;

	// Prepare the frame hierarchy.
	PrepareFrame( m_firstFrame );

	// Allocate memory for the bone matrices.
	m_boneMatrices = new D3DXMATRIX[m_totalBoneMatrices];

	// Create a static (non-animated) version of the mesh.
	m_staticMesh = new MeshContainer;
	ZeroMemory( m_staticMesh, sizeof( MeshContainer ) );

	// Load the mesh.
	ID3DXBuffer *materialBuffer, *adjacencyBuffer;
	D3DXLoadMeshFromX( GetFilename(), D3DXMESH_MANAGED, g_engine->GetDevice(), &adjacencyBuffer, &materialBuffer, NULL, &m_staticMesh->NumMaterials, &m_staticMesh->originalMesh );

	// Optimise the mesh for better rendering performance.
	m_staticMesh->originalMesh->OptimizeInplace( D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, (DWORD*)adjacencyBuffer->GetBufferPointer(), NULL, NULL, NULL );

	// Finished with the adjacency buffer, so destroy it.
	SAFE_RELEASE( adjacencyBuffer );

	// Check if the mesh has any materials.
	if( m_staticMesh->NumMaterials > 0 )
	{
		// Create the array of materials.
		m_staticMesh->materials = new Material*[m_staticMesh->NumMaterials];

		// Get the list of materials from the material buffer.
		D3DXMATERIAL *materials = (D3DXMATERIAL*)materialBuffer->GetBufferPointer();

		// Load each material into the array via the material manager.
		for( unsigned long m = 0; m < m_staticMesh->NumMaterials; m++ )
		{
			// Ensure the material has a texture.
			if( materials[m].pTextureFilename )
			{
				// Get the name of the material's script and load it.
				char *name = new char[strlen( materials[m].pTextureFilename ) + 5];
				sprintf( name, "%s.txt", materials[m].pTextureFilename );
				m_staticMesh->materials[m] = g_engine->GetMaterialManager()->Add( name, GetPath() );
				SAFE_DELETE_ARRAY( name );
			}
			else
				m_staticMesh->materials[m] = NULL;
		}
	}

	// Create the bounding volume around the mesh.
	BoundingVolumeFromMesh( m_staticMesh->originalMesh );

	// Destroy the material buffer.
	SAFE_RELEASE( materialBuffer );

	// Create a vertex array and an array of indices into the vertex array.
	m_vertices = new Vertex[m_staticMesh->originalMesh->GetNumVertices()];
	m_indices = new unsigned short[m_staticMesh->originalMesh->GetNumFaces() * 3];

	// Use the arrays to store a local copy of the static mesh's vertices and
	// indices so that they can be used by the scene manager on the fly.
	Vertex* verticesPtr;
	m_staticMesh->originalMesh->LockVertexBuffer( 0, (void**)&verticesPtr );
	unsigned short *indicesPtr;
	m_staticMesh->originalMesh->LockIndexBuffer( 0, (void**)&indicesPtr );

	memcpy( m_vertices, verticesPtr, VERTEX_FVF_SIZE * m_staticMesh->originalMesh->GetNumVertices() );
	memcpy( m_indices, indicesPtr, sizeof( unsigned short ) * m_staticMesh->originalMesh->GetNumFaces() * 3 );

	m_staticMesh->originalMesh->UnlockVertexBuffer();
	m_staticMesh->originalMesh->UnlockIndexBuffer();
}

//-----------------------------------------------------------------------------
// The mesh class destructor.
//-----------------------------------------------------------------------------
Mesh::~Mesh()
{
	// Destroy the frame hierarchy.
	AllocateHierarchy ah;
	D3DXFrameDestroy( m_firstFrame, &ah );

	// Destroy the frames list and reference points list.
	m_frames->ClearPointers();
	SAFE_DELETE( m_frames );
	m_refPoints->ClearPointers();
	SAFE_DELETE( m_refPoints );

	// Release the animation controller.
	SAFE_RELEASE( m_animationController );

	// Destroy the bone matrices.
	SAFE_DELETE_ARRAY( m_boneMatrices );

	// Destroy the static mesh.
	if( m_staticMesh )
	{
		// Remove all the static mesh's textures.
		for( unsigned long m = 0; m < m_staticMesh->NumMaterials; m++ )
			if( m_staticMesh->materials )
				g_engine->GetMaterialManager()->Remove( &m_staticMesh->materials[m] );

		// Clean up the rest of it.
		SAFE_DELETE_ARRAY( m_staticMesh->materials );
		SAFE_RELEASE( m_staticMesh->originalMesh );
		SAFE_DELETE( m_staticMesh );
	}

	// Destroy the vertex and index arrays.
	SAFE_DELETE_ARRAY( m_vertices );
	SAFE_DELETE_ARRAY( m_indices );
}

//-----------------------------------------------------------------------------
// Updates the mesh.
//-----------------------------------------------------------------------------
void Mesh::Update()
{
	UpdateFrame( m_firstFrame );
}

//-----------------------------------------------------------------------------
// Renders the mesh.
//-----------------------------------------------------------------------------
void Mesh::Render()
{
	RenderFrame( m_firstFrame );
}

//-----------------------------------------------------------------------------
// Create a clone of the mesh's animation controller.
//-----------------------------------------------------------------------------
void Mesh::CloneAnimationController( ID3DXAnimationController **animationController )
{
	if( m_animationController )
		m_animationController->CloneAnimationController( m_animationController->GetMaxNumAnimationOutputs(), m_animationController->GetMaxNumAnimationSets(), m_animationController->GetMaxNumTracks(), m_animationController->GetMaxNumEvents(), &*animationController );
	else
		*animationController = NULL;
}

//-----------------------------------------------------------------------------
// Returns the static (non-animated) version of the mesh.
//-----------------------------------------------------------------------------
MeshContainer *Mesh::GetStaticMesh()
{
	return m_staticMesh;
}

//-----------------------------------------------------------------------------
// Returns the mesh's static (non-animated) vertices.
//-----------------------------------------------------------------------------
Vertex *Mesh::GetVertices()
{
	return m_vertices;
}

//-----------------------------------------------------------------------------
// Returns the mesh's face indices for the vertices.
//-----------------------------------------------------------------------------
unsigned short *Mesh::GetIndices()
{
	return m_indices;
}

//-----------------------------------------------------------------------------
// Returns the list of frames in the mesh.
//-----------------------------------------------------------------------------
LinkedList< Frame > *Mesh::GetFrameList()
{
	return m_frames;
}

//-----------------------------------------------------------------------------
// Returns the frame with the given name.
//-----------------------------------------------------------------------------
Frame *Mesh::GetFrame( char *name )
{
	m_frames->Iterate( true );
	while( m_frames->Iterate() )
		if( strcmp( m_frames->GetCurrent()->Name, name ) == 0 )
			return m_frames->GetCurrent();

	return NULL;
}

//-----------------------------------------------------------------------------
// Returns the reference point wth the given name.
//-----------------------------------------------------------------------------
Frame *Mesh::GetReferencePoint( char *name )
{
	m_refPoints->Iterate( true );
	while( m_refPoints->Iterate() )
		if( strcmp( m_refPoints->GetCurrent()->Name, name ) == 0 )
			return m_refPoints->GetCurrent();

	return NULL;
}

//-----------------------------------------------------------------------------
// Prepares the given frame.
//-----------------------------------------------------------------------------
void Mesh::PrepareFrame( Frame *frame )
{
	m_frames->Add( frame );

	// Check if this frame is actually a reference point.
	if( strncmp( "rp_", frame->Name, 3 ) == 0 )
		m_refPoints->Add( frame );

	// Set the initial final transformation.
	frame->finalTransformationMatrix = frame->TransformationMatrix;

	// Prepare the frame's mesh container, if it has one.
	if( frame->pMeshContainer != NULL )
	{
		MeshContainer *meshContainer = (MeshContainer*)frame->pMeshContainer;

		// Check if this mesh is a skinned mesh.
		if( meshContainer->pSkinInfo != NULL )
		{
			// Create the array of bone matrix pointers.
			meshContainer->boneMatrixPointers = new D3DXMATRIX*[meshContainer->pSkinInfo->GetNumBones()];

			// Set up the pointers to the mesh's bone transformation matrices.
			for( unsigned long b = 0; b < meshContainer->pSkinInfo->GetNumBones(); b++ )
			{
				Frame *bone = (Frame*)D3DXFrameFind( m_firstFrame, meshContainer->pSkinInfo->GetBoneName( b ) );
				if( bone == NULL )
					continue;

				meshContainer->boneMatrixPointers[b] = &bone->finalTransformationMatrix;
			}

			// Keep track of the maximum bones out of all the mesh containers.
			if( m_totalBoneMatrices < meshContainer->pSkinInfo->GetNumBones() )
				m_totalBoneMatrices = meshContainer->pSkinInfo->GetNumBones();
		}

		// Check if the mesh has any materials.
		if( meshContainer->NumMaterials > 0 )
		{
			// Load all the materials in via the material manager.
			for( unsigned long m = 0; m < meshContainer->NumMaterials; m++ )
			{
				// Ensure the material has a texture.
				if( meshContainer->materialNames[m] != NULL )
				{
					// Get the name of the material's script and load it.
					char *name = new char[strlen( meshContainer->materialNames[m] ) + 5];
					sprintf( name, "%s.txt", meshContainer->materialNames[m] );
					meshContainer->materials[m] = g_engine->GetMaterialManager()->Add( name, GetPath() );
					SAFE_DELETE_ARRAY( name );
				}
			}
		}
	}

	// Prepare the frame's siblings.
	if( frame->pFrameSibling != NULL )
		PrepareFrame( (Frame*)frame->pFrameSibling );

	// Prepare the frame's children.
	if( frame->pFrameFirstChild != NULL )
		PrepareFrame( (Frame*)frame->pFrameFirstChild );
}

//-----------------------------------------------------------------------------
// Updates the given frame's transformation matrices.
//-----------------------------------------------------------------------------
void Mesh::UpdateFrame( Frame *frame, D3DXMATRIX *parentTransformationMatrix )
{
	if( parentTransformationMatrix != NULL )
		D3DXMatrixMultiply( &frame->finalTransformationMatrix, &frame->TransformationMatrix, parentTransformationMatrix );
	else
		frame->finalTransformationMatrix = frame->TransformationMatrix;

	// Update the frame's siblings.
	if( frame->pFrameSibling != NULL )
		UpdateFrame( (Frame*)frame->pFrameSibling, parentTransformationMatrix );

	// Update the frame's children.
	if( frame->pFrameFirstChild != NULL )
		UpdateFrame( (Frame*)frame->pFrameFirstChild, &frame->finalTransformationMatrix );
}

//-----------------------------------------------------------------------------
// Renders the given frame's mesh containers, if it has any.
//-----------------------------------------------------------------------------
void Mesh::RenderFrame( Frame *frame )
{
	MeshContainer *meshContainer = (MeshContainer*)frame->pMeshContainer;

	// Render this frame's mesh, if it has one.
	if( frame->pMeshContainer != NULL )
	{
		// Check if this mesh is a skinned mesh.
		if( meshContainer->pSkinInfo != NULL )
		{
			// Create the bone transformations using the mesh's transformation matrices.
			for( unsigned long b = 0; b < meshContainer->pSkinInfo->GetNumBones(); ++b )
				D3DXMatrixMultiply( &m_boneMatrices[b], meshContainer->pSkinInfo->GetBoneOffsetMatrix( b ), meshContainer->boneMatrixPointers[b] );

			// Update the meshes vertices with the new bone transformation matrices.
			PBYTE sourceVertices, destinationVertices;
			meshContainer->originalMesh->LockVertexBuffer( D3DLOCK_READONLY, (void**)&sourceVertices );
			meshContainer->MeshData.pMesh->LockVertexBuffer( 0, (void**)&destinationVertices );
			meshContainer->pSkinInfo->UpdateSkinnedMesh( m_boneMatrices, NULL, sourceVertices, destinationVertices );
			meshContainer->originalMesh->UnlockVertexBuffer();
			meshContainer->MeshData.pMesh->UnlockVertexBuffer();

			// Render the mesh by atrtribute group.
			for( unsigned long a = 0; a < meshContainer->totalAttributeGroups; a++ )
			{
				g_engine->GetDevice()->SetMaterial( meshContainer->materials[meshContainer->attributeTable[a].AttribId]->GetLighting() );
				g_engine->GetDevice()->SetTexture( 0, meshContainer->materials[meshContainer->attributeTable[a].AttribId]->GetTexture() );
				meshContainer->MeshData.pMesh->DrawSubset( meshContainer->attributeTable[a].AttribId );
			}
		}
		else
		{
			// This is not a skinned mesh, so render it like a static mesh.
			for( unsigned long m = 0; m < meshContainer->NumMaterials; m++)
			{
				if( meshContainer->materials[m] )
				{
					g_engine->GetDevice()->SetMaterial( meshContainer->materials[m]->GetLighting() );
					g_engine->GetDevice()->SetTexture( 0, meshContainer->materials[m]->GetTexture() );
				}
				else
					g_engine->GetDevice()->SetTexture( 0, NULL );

				meshContainer->MeshData.pMesh->DrawSubset( m );
			}
		}
	}

	// Render the frame's siblings.
	if( frame->pFrameSibling != NULL )
		RenderFrame( (Frame*)frame->pFrameSibling );

	// Render the frame's children.
	if( frame->pFrameFirstChild != NULL )
		RenderFrame( (Frame*)frame->pFrameFirstChild );
}