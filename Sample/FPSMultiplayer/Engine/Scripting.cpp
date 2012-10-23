//-----------------------------------------------------------------------------
// Scripting.h implementation.
// Refer to the Scripting.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "Engine.h"

//-----------------------------------------------------------------------------
// The variable class constructor.
//-----------------------------------------------------------------------------
Variable::Variable( char *name, FILE *file )
{
	// Store the name of the variable
	m_name = new char[strlen( name ) + 1];
	strcpy( m_name, name );

	// Ensure the file pointer is valid.
	if( file == NULL )
		return;

	// Read the variable's type.
	char buffer[MAX_PATH];
	fscanf( file, "%s", buffer );
	if( strcmp( buffer, "bool" ) == 0 )
	{
		// The variable is a boolean.
		m_type = VARIABLE_BOOL;

		// Read and set the bool for the variable.
		bool value;
		fscanf( file, "%s", buffer );
		if( strcmp( buffer, "true" ) == 0 )
			value = true;
		else
			value = false;
		m_data = new bool;
		memcpy( m_data, &value, sizeof( bool ) );
	}
	else if( strcmp( buffer, "colour" ) == 0 )
	{
		// The variable is a colour.
		m_type = VARIABLE_COLOUR;

		// Read and set the colour for the variable.
		D3DCOLORVALUE colour;
		fscanf( file, "%s", buffer );
		colour.r = (float)atof( buffer );
		fscanf( file, "%s", buffer );
		colour.g = (float)atof( buffer );
		fscanf( file, "%s", buffer );
		colour.b = (float)atof( buffer );
		fscanf( file, "%s", buffer );
		colour.a = (float)atof( buffer );
		m_data = new D3DCOLORVALUE;
		memcpy( m_data, &colour, sizeof( D3DCOLORVALUE ) );
	}
	else if( strcmp( buffer, "float" ) == 0 )
	{
		// The variable is a float.
		m_type = VARIABLE_FLOAT;

		// Read and set the float for the variable.
		float value;
		fscanf( file, "%s", buffer );
		value = (float)atof( buffer );
		m_data = new float;
		memcpy( m_data, &value, sizeof( float ) );
	}
	else if( strcmp( buffer, "number" ) == 0 )
	{
		// The variable is a number.
		m_type = VARIABLE_NUMBER;

		// Read and set the number for the variable.
		long value;
		fscanf( file, "%s", buffer );
		value = atol( buffer );
		m_data = new long;
		memcpy( m_data, &value, sizeof( long ) );
	}
	else if( strcmp( buffer, "string" ) == 0 )
	{
		// The variable is a string.
		m_type = VARIABLE_STRING;

		// Find the opening inverted commas.
		bool commasFound = false;
		ZeroMemory( buffer, MAX_PATH * sizeof( char ) );
		fscanf( file, "%c", buffer );
		while( true )
		{
			if( strcmp( buffer, "\"" ) == 0 )
			{
				commasFound = true;
				break;
			}

			if( strcmp( buffer, " " ) != 0 )
			{
				fpos_t pos;
				fgetpos( file, &pos );
				fsetpos( file, &--pos );
				break;
			}

			fscanf( file, "%c", buffer );
		}

		// Read and set the string for the variable.
		char completeString[MAX_PATH];
		ZeroMemory( completeString, MAX_PATH * sizeof( char ) );
		bool addSpacing = false;
		do
		{
			fscanf( file, "%s", buffer );
			if( strcmp( &buffer[strlen( buffer ) - 1], "\"" ) == 0 )
			{
				buffer[strlen( buffer ) - 1] = 0;
				commasFound = false;
			}

			if( addSpacing == false )
				addSpacing = true;
			else
				strcat( completeString, " " );

			strcat( completeString, buffer );
		} while( commasFound == true );

		m_data = new char[strlen( completeString ) + 1];
		strcpy( (char*)m_data, completeString );
	}
	else if( strcmp( buffer, "vector" ) == 0 )
	{
		// The variable is a vector.
		m_type = VARIABLE_VECTOR;

		// Read and set the vector for the variable.
		D3DXVECTOR3 vector;
		fscanf( file, "%s", buffer );
		vector.x = (float)atof( buffer );
		fscanf( file, "%s", buffer );
		vector.y = (float)atof( buffer );
		fscanf( file, "%s", buffer );
		vector.z = (float)atof( buffer );
		m_data = new D3DXVECTOR3;
		memcpy( m_data, &vector, sizeof( D3DXVECTOR3 ) );
	}
	else
	{
		// The variable has an unknown type.
		m_type = VARIABLE_UNKNOWN;

		// Read and set the data (same as a string) for the variable.
		fscanf( file, "%s", buffer );
		m_data = new char[strlen( buffer ) + 1];
		strcpy( (char*)m_data, buffer );
	}
}

//-----------------------------------------------------------------------------
// The variable class constructor.
//-----------------------------------------------------------------------------
Variable::Variable( char *name, char type, void *value )
{
	// Store the name of the variable.
	m_name = new char[strlen( name ) + 1];
	strcpy( m_name, name );

	// Store the type of the variable.
	m_type = type;

	// Set the variable's data based on its type.
	switch( m_type )
	{
		case VARIABLE_BOOL:
			m_data = new bool;
			memcpy( m_data, (bool*)value, sizeof( bool ) );
			return;

		case VARIABLE_COLOUR:
			m_data = new D3DCOLORVALUE;
			memcpy( m_data, (D3DCOLORVALUE*)value, sizeof( D3DCOLORVALUE ) );
			return;

		case VARIABLE_FLOAT:
			m_data = new float;
			memcpy( m_data, (float*)value, sizeof( float ) );
			return;

		case VARIABLE_NUMBER:
			m_data = new long;
			memcpy( m_data, (long*)value, sizeof( long ) );
			return;

		case VARIABLE_STRING:
			m_data = new char[strlen( (char*)value ) + 1];
			strcpy( (char*)m_data, (char*)value );
			return;

		case VARIABLE_VECTOR:
			m_data = new D3DXVECTOR3;
			memcpy( m_data, (D3DXVECTOR3*)value, sizeof( D3DXVECTOR3 ) );
			return;

		default:
			m_data = new char[strlen( (char*)value ) + 1];
			strcpy( (char*)m_data, (char*)value );
			return;
	}
}

//-----------------------------------------------------------------------------
// The variable class destructor.
//-----------------------------------------------------------------------------
Variable::~Variable()
{
	SAFE_DELETE_ARRAY( m_name );
	SAFE_DELETE( m_data );
}

//-----------------------------------------------------------------------------
// Returns the type of the variable.
//-----------------------------------------------------------------------------
char Variable::GetType()
{
	return m_type;
}

//-----------------------------------------------------------------------------
// Returns the name of the variable.
//-----------------------------------------------------------------------------
char *Variable::GetName()
{
	return m_name;
}

//-----------------------------------------------------------------------------
// Returns the data in the variable.
//-----------------------------------------------------------------------------
void *Variable::GetData()
{
	switch( m_type )
	{
		case VARIABLE_BOOL:
			return (bool*)m_data;

		case VARIABLE_COLOUR:
			return (D3DCOLORVALUE*)m_data;

		case VARIABLE_FLOAT:
			return (float*)m_data;

		case VARIABLE_NUMBER:
			return (long*)m_data;

		case VARIABLE_STRING:
			return (char*)m_data;

		case VARIABLE_VECTOR:
			return (D3DXVECTOR3*)m_data;

		default:
			return m_data;
	}
}

//-----------------------------------------------------------------------------
// The script class constructor.
//-----------------------------------------------------------------------------
Script::Script( char *name, char *path ) : Resource< Script >( name, path )
{
	// Create the linked list that will store all of the script's variables.
	m_variables = new LinkedList< Variable >;

	// Open the script file using the filename.
	FILE *file = NULL;
	if( ( file = fopen( GetFilename(), "r" ) ) == NULL )
		return;

	// Continue reading from the file until the eof is reached.
	bool read = false;
	char buffer[MAX_PATH];
	fscanf( file, "%s", buffer );
	while( feof( file ) == 0 )
	{
		// Check if the file position indicator is between a #begin and #end
		// statement. If so then read the data into the variable linked list.
		if( read == true )
		{
			// Stop reading data if an #end statement has been reached.
			if( strcmp( buffer, "#end" ) == 0 )
				read = false;
			else
				m_variables->Add( new Variable( buffer, file ) );
		}
		else if( strcmp( buffer, "#begin" ) == 0 )
			read = true;

		// Read the next string.
		fscanf( file, "%s", buffer );
	}

	// Close the script file.
	fclose( file );
}

//-----------------------------------------------------------------------------
// The script class destructor.
//-----------------------------------------------------------------------------
Script::~Script()
{
	SAFE_DELETE( m_variables );
}

//-----------------------------------------------------------------------------
// Adds a new variable to the script.
//-----------------------------------------------------------------------------
void Script::AddVariable( char *name, char type, void *value )
{
	m_variables->Add( new Variable( name, type, value ) );
}

//-----------------------------------------------------------------------------
// Sets the value of an existing variable in the script.
//-----------------------------------------------------------------------------
void Script::SetVariable( char *name, void *value )
{
	// Find the variable.
	Variable *variable = NULL;
	m_variables->Iterate( true );
	while( m_variables->Iterate() != NULL )
	{
		if( strcmp( m_variables->GetCurrent()->GetName(), name ) == 0 )
		{
			variable = m_variables->GetCurrent();
			break;
		}
	}

	// Ensure the variable was found.
	if( variable == NULL )
		return;

	// Get the variable's type.
	char type = variable->GetType();

	// Destroy the variable.
	m_variables->Remove( &variable );

	// Add the variable back in with the new value.
	AddVariable( name, type, value );
}

//-----------------------------------------------------------------------------
// Saves the script to file.
//-----------------------------------------------------------------------------
void Script::SaveScript( char *filename )
{
	FILE *file = NULL;
	char output[MAX_PATH];

	// Open the given filename if available, otherwise the internal filename.
	if( filename != NULL )
	{
		if( ( file = fopen( filename, "w" ) ) == NULL )
			return;
	}
	else
	{
		if( ( file = fopen( GetFilename(), "w" ) ) == NULL )
			return;
	}

	// Write the #begin statement to the file.
	fputs( "#begin\n", file );

	// Write each variable to the file.
	m_variables->Iterate( true );
	while( m_variables->Iterate() != NULL )
	{
		switch( m_variables->GetCurrent()->GetType() )
		{
			case VARIABLE_BOOL:
				if( *((bool*)m_variables->GetCurrent()->GetData()) == true )
					sprintf( output, "%s bool true", m_variables->GetCurrent()->GetName() );
				else
					sprintf( output, "%s bool false", m_variables->GetCurrent()->GetName() );
				fputs( output, file );
				fputs( "\n", file );
				continue;

			case VARIABLE_COLOUR:
				sprintf( output, "%s colour %f %f %f %f", m_variables->GetCurrent()->GetName(), ( (D3DCOLORVALUE*)m_variables->GetCurrent()->GetData() )->r, ( (D3DCOLORVALUE*)m_variables->GetCurrent()->GetData() )->g, ( (D3DCOLORVALUE*)m_variables->GetCurrent()->GetData() )->b, ( (D3DCOLORVALUE*)m_variables->GetCurrent()->GetData() )->a );
				fputs( output, file );
				fputs( "\n", file );
				continue;

			case VARIABLE_FLOAT:
				sprintf( output, "%s float %f", m_variables->GetCurrent()->GetName(), *(float*)m_variables->GetCurrent()->GetData() );
				fputs( output, file );
				fputs( "\n", file );
				continue;

			case VARIABLE_NUMBER:
				sprintf( output, "%s number %d", m_variables->GetCurrent()->GetName(), *(long*)m_variables->GetCurrent()->GetData() );
				fputs( output, file );
				fputs( "\n", file );
				continue;

			case VARIABLE_STRING:
				sprintf( output, "%s string \"%s\"", m_variables->GetCurrent()->GetName(), (char*)m_variables->GetCurrent()->GetData() );
				fputs( output, file );
				fputs( "\n", file );
				continue;

			case VARIABLE_VECTOR:
				sprintf( output, "%s vector %f %f %f", m_variables->GetCurrent()->GetName(), ( (D3DXVECTOR3*)m_variables->GetCurrent()->GetData() )->x, ( (D3DXVECTOR3*)m_variables->GetCurrent()->GetData() )->y, ( (D3DXVECTOR3*)m_variables->GetCurrent()->GetData() )->z );
				fputs( output, file );
				fputs( "\n", file );
				continue;

			default:
				sprintf( output, "%s unknown %s", m_variables->GetCurrent()->GetName(), (char*)m_variables->GetCurrent()->GetData() );
				fputs( output, file );
				fputs( "\n", file );
				continue;
		}
	}

	// Write the #end statement to the file.
	fputs( "#end", file );

	// Close the script file.
	fclose( file );
}

//-----------------------------------------------------------------------------
// Returns boolean data from the named variable.
//-----------------------------------------------------------------------------
bool *Script::GetBoolData( char *variable )
{
	m_variables->Iterate( true );
	while( m_variables->Iterate() != NULL )
		if( strcmp( m_variables->GetCurrent()->GetName(), variable ) == 0 )
			return (bool*)m_variables->GetCurrent()->GetData();

	return NULL;
}

//-----------------------------------------------------------------------------
// Returns colour data from the named variable.
//-----------------------------------------------------------------------------
D3DCOLORVALUE *Script::GetColourData( char *variable )
{
	m_variables->Iterate( true );
	while( m_variables->Iterate() != NULL )
		if( strcmp( m_variables->GetCurrent()->GetName(), variable ) == 0 )
			return (D3DCOLORVALUE*)m_variables->GetCurrent()->GetData();

	return NULL;
}

//-----------------------------------------------------------------------------
// Returns float data from the named variable.
//-----------------------------------------------------------------------------
float *Script::GetFloatData( char *variable )
{
	m_variables->Iterate( true );
	while( m_variables->Iterate() != NULL )
		if( strcmp( m_variables->GetCurrent()->GetName(), variable ) == 0 )
			return (float*)m_variables->GetCurrent()->GetData();

	return NULL;
}

//-----------------------------------------------------------------------------
// Returns number data from the named variable.
//-----------------------------------------------------------------------------
long *Script::GetNumberData( char *variable )
{
	m_variables->Iterate( true );
	while( m_variables->Iterate() != NULL )
		if( strcmp( m_variables->GetCurrent()->GetName(), variable ) == 0 )
			return (long*)m_variables->GetCurrent()->GetData();

	return NULL;
}

//-----------------------------------------------------------------------------
// Returns string data from the named variable.
//-----------------------------------------------------------------------------
char *Script::GetStringData( char *variable )
{
	m_variables->Iterate( true );
	while( m_variables->Iterate() != NULL )
		if( strcmp( m_variables->GetCurrent()->GetName(), variable ) == 0 )
			return (char*)m_variables->GetCurrent()->GetData();

	return NULL;
}

//-----------------------------------------------------------------------------
// Returns colour data from the named variable.
//-----------------------------------------------------------------------------
D3DXVECTOR3 *Script::GetVectorData( char *variable )
{
	m_variables->Iterate( true );
	while( m_variables->Iterate() != NULL )
		if( strcmp( m_variables->GetCurrent()->GetName(), variable ) == 0 )
			return (D3DXVECTOR3*)m_variables->GetCurrent()->GetData();

	return NULL;
}

//-----------------------------------------------------------------------------
// Returns unknown data from the named variable.
//-----------------------------------------------------------------------------
void *Script::GetUnknownData( char *variable )
{
	m_variables->Iterate( true );
	while( m_variables->Iterate() != NULL )
		if( strcmp( m_variables->GetCurrent()->GetName(), variable ) == 0 )
			return m_variables->GetCurrent()->GetData();

	return NULL;
}