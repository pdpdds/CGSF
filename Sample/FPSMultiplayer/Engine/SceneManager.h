//-----------------------------------------------------------------------------
// Manages and renders a scene and the objects in it using frustum and
// occlusion culling, and performing collision detection.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

//-----------------------------------------------------------------------------
// Scene Occluder Structure
//-----------------------------------------------------------------------------
struct SceneOccluder : public BoundingVolume
{
	unsigned long visibleStamp; // Stamp indicating if the occluder is visible in the current frame.
	D3DXVECTOR3 translation; // Translation of the occluder.
	unsigned long totalFaces; // Total number of faces in the occluder's mesh.
	Vertex *vertices; // Array containing the occluder's vertices transformed into world space.
	unsigned short *indices; // Array of indices into the vertex array.
	LinkedList< D3DXPLANE > *planes; // List of planes that define the occluded volume.
	float distance; // Distance between the viewer and the occluder.

	//-------------------------------------------------------------------------
	// The scene occluder structure constructor.
	//-------------------------------------------------------------------------
	SceneOccluder( D3DXVECTOR3 t, ID3DXMesh *mesh, D3DXMATRIX *world )
	{
		// Clear the visible stamp.
		visibleStamp = -1;

		// Set the translation.
		translation = t;

		// Set the total faces and create the vertx and idex arrays.
		totalFaces = mesh->GetNumFaces();
		vertices = new Vertex[mesh->GetNumVertices()];
		indices = new unsigned short[totalFaces * 3];

		// Lock the given mesh's vertex and index buffers.
		Vertex* verticesPtr;
		mesh->LockVertexBuffer( 0, (void**)&verticesPtr );
		unsigned short *indicesPtr;
		mesh->LockIndexBuffer( 0, (void**)&indicesPtr );

		// Copy the vertices and the indices.
		memcpy( vertices, verticesPtr, VERTEX_FVF_SIZE * mesh->GetNumVertices() );
		memcpy( indices, indicesPtr, sizeof( unsigned short ) * totalFaces * 3 );

		// Unlock the vertex and index buffers.
		mesh->UnlockVertexBuffer();
		mesh->UnlockIndexBuffer();

		// Transform the vertices into world space.
		for( unsigned long v = 0; v < mesh->GetNumVertices(); v++ )
			D3DXVec3TransformCoord( &vertices[v].translation, &vertices[v].translation, world );

		// Create a list of planes for building the occlusion volume.
		planes = new LinkedList< D3DXPLANE >;

		// Create a bounding volume from the occluder's mesh.
		BoundingVolumeFromMesh( mesh );

		// Resposition the bounding volume into world space.
		D3DXMATRIX location;
		D3DXMatrixTranslation( &location, t.x, t.y, t.z );
		RepositionBoundingVolume( &location );
	}

	//-------------------------------------------------------------------------
	// The scene occluder structure destructor.
	//-------------------------------------------------------------------------
	virtual ~SceneOccluder()
	{
		SAFE_DELETE_ARRAY( vertices );
		SAFE_DELETE_ARRAY( indices );

		SAFE_DELETE( planes );
	}
};

//-----------------------------------------------------------------------------
// Scene Leaf Structure
//-----------------------------------------------------------------------------
struct SceneLeaf : public BoundingVolume
{
	SceneLeaf *children[8]; // Array of child scene leaf pointers.
	unsigned long visibleStamp; // Indicates if the scene leaf is visible in the current frame.
	LinkedList< SceneOccluder > *occluders; // list of scene occluders in the scene leaf.
	unsigned long totalFaces; // Total number of faces in the scene leaf.
	unsigned long *faces; // Array of indices pointing to the faces in the scene leaf.

	//-------------------------------------------------------------------------
	// The scene leaf structure constructor.
	//-------------------------------------------------------------------------
	SceneLeaf()
	{
		for( char l = 0; l < 8; l++ )
			children[l] = NULL;
		occluders = new LinkedList< SceneOccluder >;
		totalFaces = 0;
		faces = NULL;
	}

	//-------------------------------------------------------------------------
	// The scene leaf structure destructor.
	//-------------------------------------------------------------------------
	virtual ~SceneLeaf()
	{
		for( char l = 0; l < 8; l++ )
			SAFE_DELETE( children[l] );
		occluders->ClearPointers();
		SAFE_DELETE( occluders );
		SAFE_DELETE_ARRAY( faces );
	}
};

//-----------------------------------------------------------------------------
// Scene Face Structure
//-----------------------------------------------------------------------------
struct SceneFace : public IndexedFace
{
	RenderCache *renderCache; // Pointer to the render cache this face belongs to.
	unsigned long renderStamp; // Indicates when the face was last rendered.
};

//-----------------------------------------------------------------------------
// Ray Intersection Result Struction
//-----------------------------------------------------------------------------
struct RayIntersectionResult
{
	Material *material; // Pointer to the material of the intersected face.
	float distance; // Distance the ray can travel until intersection occurs.
	D3DXVECTOR3 point; // Intersection point in 3D space.
	SceneObject *hitObject; // Pointer to the hit scene object (if one was hit).

	//-------------------------------------------------------------------------
	// The ray intersection result structure constructor.
	//-------------------------------------------------------------------------
	RayIntersectionResult()
	{
		material = NULL;
		distance = 0.0f;
		point = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		hitObject = NULL;
	}
};

//-----------------------------------------------------------------------------
// Scene Manager Class
//-----------------------------------------------------------------------------
class SceneManager
{
public:
	SceneManager( float scale, char *spawnerPath );
	virtual ~SceneManager();

	void LoadScene( char *name, char *path = "./" );
	void DestroyScene();
	bool IsLoaded();

	void Update( float elapsed, D3DXMATRIX *view = NULL );
	void Render( float elapsed, D3DXVECTOR3 viewer );

	SceneObject *AddObject( SceneObject *object );
	void RemoveObject( SceneObject **object );

	SceneObject *GetRandomPlayerSpawnPoint();
	SceneObject *GetSpawnPointByID( long id );
	long GetSpawnPointID( SceneObject *point );

	LinkedList< SpawnerObject > *GetSpawnerObjectList();

	bool RayIntersectScene( RayIntersectionResult *result, D3DXVECTOR3 rayPosition, D3DXVECTOR3 rayDirection, bool checkScene = true, SceneObject *thisObject = NULL, bool checkObjects = false );

private:
	void BuildOcclusionVolume( SceneOccluder *occluder, D3DXVECTOR3 viewer );

	void RecursiveSceneBuild( SceneLeaf *leaf, D3DXVECTOR3 translation, float halfSize );
	bool RecursiveSceneFrustumCheck( SceneLeaf *leaf, D3DXVECTOR3 viewer );
	void RecursiveSceneOcclusionCheck( SceneLeaf *leaf );

private:
	char *m_name; // Name of the scene.
	float m_scale; // Scene scale in meters/unit.
	ViewFrustum m_viewFrustum; // View frustum for visiblity culling.
	D3DXVECTOR3 m_gravity; // Constant gravity pull.
	bool m_loaded; // Indicates if the scene has been loaded or not.
	Mesh *m_mesh; // Mesh for the scene.
	unsigned long m_maxFaces; // Maximum number of faces per scene leaf.
	float m_maxHalfSize; // Maximum half size of a scene leaf.
	unsigned long m_frameStamp; // Current frame time stamp.

	LinkedList< SceneObject > *m_dynamicObjects; // Linked list of dynamic objects.
	LinkedList< SceneOccluder > *m_occludingObjects; // Linked list of occluding objects.
	LinkedList< SceneOccluder > *m_visibleOccluders; // Linked list of visible occluders each frame.
	LinkedList< SceneObject > *m_playerSpawnPoints; // Linked list of player spawn points.
	LinkedList< SpawnerObject > *m_objectSpawners; // Linked list of object spawners.
	char *m_spawnerPath; // Path used for loading the spawner object scripts.

	SceneLeaf *m_firstLeaf; // The first scene leaf in the scene hierarchy.

	IDirect3DVertexBuffer9 *m_sceneVertexBuffer; // Vertex buffer for all the vertices in the scene.
	Vertex *m_vertices; // Pointer for accessing the vertices in the vertex buffer.
	unsigned long m_totalVertices; // Total number of vertices in the scene.

	LinkedList< RenderCache > *m_renderCaches; // Linked list of render caches.

	unsigned long m_totalFaces; // Total number of faces in the scene.
	SceneFace *m_faces; // Array of faces in the scene.
};

#endif