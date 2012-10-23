//-----------------------------------------------------------------------------
// Used to manage the creation and destruction of any resource type.
// ResourceManager will ensure there is no redundancy.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef RESOURCE_MANAGEMENT_H
#define RESOURCE_MANAGEMENT_H

//-----------------------------------------------------------------------------
// Resource Class
//-----------------------------------------------------------------------------
template< class Type > class Resource
{
public:
	//-------------------------------------------------------------------------
	// The resource class constructor.
	//-------------------------------------------------------------------------
	Resource( char *name, char *path = "./" )
	{
		// Store the name.
		if( name != NULL )
		{
			m_name = new char[strlen( name ) + 1];
			memcpy( m_name, name, ( strlen( name ) + 1 ) * sizeof( char ) );
		}

		// Store the path.
		if( path != NULL )
		{
			m_path = new char[strlen( path ) + 1];
			memcpy( m_path, path, ( strlen( path ) + 1 ) * sizeof( char ) );
		}

		// Create the filename.
		if( name != NULL && path != NULL )
		{
			m_filename = new char[strlen( name ) + strlen( path ) + 1];
			sprintf( m_filename, "%s%s", path, name );
		}

		// Start the reference count.
		m_refCount = 1;
	}

	//-------------------------------------------------------------------------
	// The resource class destructor.
	//-------------------------------------------------------------------------
	virtual ~Resource()
	{
		SAFE_DELETE_ARRAY( m_name );
		SAFE_DELETE_ARRAY( m_path );
		SAFE_DELETE_ARRAY( m_filename );
	}

	//-------------------------------------------------------------------------
	// Returns the name of the resource.
	//-------------------------------------------------------------------------
	char *GetName()
	{
		return m_name;
	}

	//-------------------------------------------------------------------------
	// Returns the path to the resource.
	//-------------------------------------------------------------------------
	char *GetPath()
	{
		return m_path;
	}

	//-------------------------------------------------------------------------
	// Returns the filename of the resource.
	//-------------------------------------------------------------------------
	char *GetFilename()
	{
		return m_filename;
	}

	//-------------------------------------------------------------------------
	// Increments the resource's reference count.
	//-------------------------------------------------------------------------
	void IncRef()
	{
		m_refCount++;
	}

	//-------------------------------------------------------------------------
	// Decrements the resource's reference count.
	//-------------------------------------------------------------------------
	void DecRef()
	{
		m_refCount--;
	}

	//-------------------------------------------------------------------------
	// Returns the resource's reference count.
	//-------------------------------------------------------------------------
	unsigned long GetRefCount()
	{
		return m_refCount;
	}

private:
	char *m_name; // Name of the resource.
	char *m_path; // Path to the resource.
	char *m_filename; // Filename (name + path) of the resource.
	unsigned long m_refCount; // Reference count.
};

//-----------------------------------------------------------------------------
// Resource Manager Class
//-----------------------------------------------------------------------------
template< class Type > class ResourceManager
{
public:
	//-------------------------------------------------------------------------
	// The resource manager class constructor.
	//-------------------------------------------------------------------------
	ResourceManager( void (*CreateResourceFunction)( Type **resource, char *name, char *path ) = NULL )
	{
		m_list = new LinkedList< Type >;

		CreateResource = CreateResourceFunction;
	}

	//-------------------------------------------------------------------------
	// The resource manager class destructor.
	//-------------------------------------------------------------------------
	~ResourceManager()
	{
		SAFE_DELETE( m_list );
	}

	//-------------------------------------------------------------------------
	// Adds a new resource to the manager.
	//-------------------------------------------------------------------------
	Type *Add( char *name, char *path = "./" )
	{
		// Ensure the list, the resource name, and its path are valid.
		if( m_list == NULL || name == NULL || path == NULL )
			return NULL;

		// If the element already exists, then return a pointer to it.
		Type *element = GetElement( name, path );
		if( element != NULL )
		{
			element->IncRef();
			return element;
		}

		// Create the resource, preferably through the application specific
		// function if it is available.
		Type *resource = NULL;
		if( CreateResource != NULL )
			CreateResource( &resource, name, path );
		else
			resource = new Type( name, path );

		// Add the new resource to the manager and return a pointer to it.
		return m_list->Add( resource );
	}

	//-------------------------------------------------------------------------
	// Removes the given resource from the manager.
	//-------------------------------------------------------------------------
	void Remove( Type **resource )
	{
		// Ensure the resource to be removed and the list is valid.
		if( *resource == NULL || m_list == NULL )
			return;

		// Decrement the resource's reference count.
		(*resource)->DecRef();

		// If the resource is no long being used then destroy it.
		if( (*resource)->GetRefCount() == 0 )
			m_list->Remove( resource );
	}

	//-------------------------------------------------------------------------
	// Empties the resource list.
	//-------------------------------------------------------------------------
	void EmptyList()
	{
		if( m_list != NULL )
			m_list->Empty();
	}

	//-------------------------------------------------------------------------
	// Returns the list of resources.
	//-------------------------------------------------------------------------
	LinkedList< Type > *GetList()
	{
		return m_list;
	}

	//-------------------------------------------------------------------------
	// Returns a resource by its filename.
	//-------------------------------------------------------------------------
	Type *GetElement( char *name, char *path = "./" )
	{
		// Ensure the name and path are valid, and the list is valid and not empty.
		if( name == NULL || path == NULL || m_list == NULL )
			return NULL;
		if( m_list->GetFirst() == NULL )
			return NULL;

		// Iterate the list looking for the specified resource.
		m_list->Iterate( true );
		while( m_list->Iterate() )
			if( strcmp( m_list->GetCurrent()->GetName(), name ) == 0 )
				if( strcmp( m_list->GetCurrent()->GetPath(), path ) == 0 )
					return m_list->GetCurrent();

		// Return NULL if the resource was not found.
		return NULL;
	}

private:
	LinkedList< Type > *m_list; // Linked list of resources.

	void (*CreateResource)( Type **resource, char *name, char *path ); // Application specific resource creation.
};

#endif