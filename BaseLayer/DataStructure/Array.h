// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// Array.h
// This is the basic 1D array class.
// ============================================================================
#ifndef ARRAY_H
#define ARRAY_H

#include <stdio.h>

namespace CGBase
{


	template<class Datatype>
	class Array
	{
	public:

		// -------------------------------------------------------
		// Name:        Array::Array
		// Description: This constructs the array.
		// Arguments:   - p_size: the size of the array.
		// -------------------------------------------------------
		Array( int p_size )
		{
			// allocate enough memory for the array.
			m_array = new Datatype[p_size];

			// set the size variable
			m_size = p_size;
		}


		// -------------------------------------------------------
		// Name:        Array::~Array
		// Description: This destructs the array.
		// -------------------------------------------------------
		~Array()
		{
			// if the array is not null, delete it.
			if( m_array != 0 )
				delete[] m_array;

			// clear the pointer, just in case we ever modify this.
			m_array = 0;
		}


		// -------------------------------------------------------
		// Name:        Array::Resize
		// Description: This resizes the array to a new size.
		// Arguments:   - p_size: the new size of the array.
		// -------------------------------------------------------
		void Resize( int p_size )
		{
			// create a new array with the new size
			Datatype* newarray = new Datatype[p_size];

			// if the new array wasn't allocated, then just return
			// and don't change anything.
			if( newarray == 0 )
				return;

			// determine which size is smaller.
			int min;
			if( p_size < m_size )
				min = p_size;
			else
				min = m_size;

			// loop through and copy everything possible over.
			int index;
			for( index = 0; index < min; index++ )
				newarray[index] = m_array[index];

			// set the size of the new array
			m_size = p_size;

			// delete the old array.
			if( m_array != 0 )
				delete[] m_array;

			// copy the pointer over.
			m_array = newarray;
		}


		// -------------------------------------------------------
		//  Name:         Array::operator[]
		//  Description:  gets a reference to the item at given 
		//                index.
		//  Arguments:    - p_index: index of the item to get.
		//  Return Value: reference to the item at the index.
		// -------------------------------------------------------
		Datatype& operator[] ( int p_index )
		{
			return m_array[p_index];
		}



		// -------------------------------------------------------
		//  Name:         Array::Insert
		//  Description:  Inserts a cell inbetween two others.
		//  Arguments:    - p_item: item to insert
		//                - p_index: index to insert at.
		// -------------------------------------------------------
		void Insert( Datatype p_item, int p_index )
		{
			int index;

			// move everything after p_index up by one cell.
			for( index = m_size - 1; index > p_index; index-- )
				m_array[index] = m_array[index - 1];

			// insert the item.
			m_array[p_index] = p_item;
		}



		// -------------------------------------------------------
		//  Name:         Array::Remove
		//  Description:  Removes a cell
		//  Arguments:    - p_index: index to remove.
		// -------------------------------------------------------
		void Remove( int p_index )
		{
			int index;

			// move everything after p_index down by one cell.
			for( index = p_index + 1; index < m_size; index++ )
				m_array[index - 1] = m_array[index];
		}



		// -------------------------------------------------------
		//  Name:         Array::Size
		//  Description:  gets the size of the array.
		//  Arguments:    None.
		//  Return Value: the size of the array.
		// -------------------------------------------------------
		int Size()
		{
			return m_size;
		}


		// -------------------------------------------------------
		//  Name:         Array::operator DataType*
		//  Description:  conversion operator, converts array
		//                into a pointer, for use in C-functions
		//                and other normal array functions
		//  Arguments:    None.
		//  Return Value: a pointer to the array.
		// -------------------------------------------------------
		operator Datatype* ()
		{
			return m_array;
		}


		// -------------------------------------------------------
		//  Name:         Array::WriteFile
		//  Description:  Writes an array to disk
		//  Arguments:    The Filename
		//  Return Value: true on success, false on failure
		// -------------------------------------------------------
		bool WriteFile( const char* p_filename )
		{
			FILE* outfile = 0;
			int written = 0;

			// open the file
			outfile = fopen( p_filename, "wb" );

			// return if it couldn't be opened
			if( outfile == 0 )
				return false;

			// write the array and close thef ile
			written = fwrite( m_array, sizeof( Datatype ), m_size, outfile );
			fclose( outfile );

			// if we didn't write the number of items we expected,
			// return failure
			if( written != m_size )
				return false;

			// return success.
			return true;
		}


		// -------------------------------------------------------
		//  Name:         Array::ReadFile
		//  Description:  reads an array from disk
		//  Arguments:    The Filename
		//  Return Value: true on success, false on failure
		// -------------------------------------------------------
		bool ReadFile( const char* p_filename )
		{
			FILE* infile = 0;
			int read = 0;

			// open the file
			infile = fopen( p_filename, "rb" );

			// return if it couldn't be opened
			if( infile == 0 )
				return false;

			// read the array and close the file
			read = fread( m_array, sizeof( Datatype ), m_size, infile );
			fclose( infile );

			// if we didn't read the number of items we expected,
			// return failure
			if( read != m_size )
				return false;

			// return success
			return true;
		}



		// -------------------------------------------------------
		// Name:        Array::m_array
		// Description: This is a pointer to the array.
		// -------------------------------------------------------
		Datatype* m_array;


		// -------------------------------------------------------
		// Name:        Array::m_size
		// Description: the current size of the array.
		// -------------------------------------------------------
		int m_size;
	};

};


#endif