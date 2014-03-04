//-----------------------------------------------------------------------------
// A simple scripting system. The scripts are nothing more than a collection of
// variables stored in a text file.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef SCRIPTING_H
#define SCRIPTING_H

//-----------------------------------------------------------------------------
// Variable Type Enumeration
//-----------------------------------------------------------------------------
enum{ VARIABLE_BOOL, VARIABLE_COLOUR, VARIABLE_FLOAT, VARIABLE_NUMBER, VARIABLE_STRING, VARIABLE_VECTOR, VARIABLE_UNKNOWN };

//-----------------------------------------------------------------------------
// Variable Class
//-----------------------------------------------------------------------------
class Variable
{
public:
	Variable( char *name, FILE *file );
	Variable( char *name, char type, void *value );
	virtual ~Variable();

	char GetType();
	char *GetName();
	void *GetData();

private:
	char m_type; // Type of data stored in the variable.
	char *m_name; // Name of the variable.
	void *m_data; // Data stored in the variable.
};

//-----------------------------------------------------------------------------
// Script Class
//-----------------------------------------------------------------------------
class Script : public Resource< Script >
{
public:
	Script( char *name, char *path = "./" );
	virtual ~Script();

	void AddVariable( char *name, char type, void *value );
	void SetVariable( char *name, void *value );

	void SaveScript( char *filename = NULL );

	bool *GetBoolData( char *variable );
	D3DCOLORVALUE *GetColourData( char *variable );
	float *GetFloatData( char *variable );
	long *GetNumberData( char *variable );
	char *GetStringData( char *variable );
	D3DXVECTOR3 *GetVectorData( char *variable );
	void *GetUnknownData( char *variable );

private:
	LinkedList< Variable > *m_variables; // Linked list of variables in the script.
};

#endif