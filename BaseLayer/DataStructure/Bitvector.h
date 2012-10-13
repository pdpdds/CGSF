// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// Bitvector.h
// This is the Bitvector class
// ============================================================================

namespace CGBase
{


	class Bitvector
	{
	public:

		// ----------------------------------------------------------------
		//  Name:           Bitvector
		//  Description:    Default Constructor. 
		//  Arguments:      - p_size: The size in BITS of the vector.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		Bitvector( int p_size )
		{
			m_array = 0;
			m_size = 0;
			Resize( p_size );
		}

		// ----------------------------------------------------------------
		//  Name:           ~Bitvector
		//  Description:    Destructor, destroys the vector 
		//  Arguments:      None.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		~Bitvector()
		{
			// if the array exists, delete it.
			if( m_array != 0 )
				delete[] m_array;
			m_array = 0;
		}


		// ----------------------------------------------------------------
		//  Name:           Resize
		//  Description:    Resizes the bitvector
		//  Arguments:      - p_size: The size in BITS of the vector.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		void Resize( int p_size )
		{
			// declare a new vector
			unsigned long int* newvector = 0;

			// convert the bit-size into an integer-size
			if( p_size % 32 == 0 )
				p_size = p_size / 32;
			else
				p_size = (p_size / 32) + 1;

			// create the new vector using the converted size.
			newvector = new unsigned long int[p_size];

			// if it wasn't allocated properly, exit out without
			// changing anything.
			if( newvector == 0 )
				return;

			// find the minimum size of the two different sizes.
			int min;
			if( p_size < m_size )
				min = p_size;
			else
				min = m_size;

			// copy everything that can be copied over.
			int index;
			for( index = 0; index < min; index++ )
				newvector[index] = m_array[index];

			// set the new size
			m_size = p_size;

			// delete the old array
			if( m_array != 0 )
				delete[] m_array;

			// make the array point to the new array.
			m_array = newvector;
		}


		// this is the access operator. It retrieves
		// the given bit from the vector.
		bool operator[] ( int p_index )
		{
			// figure out which cell the bit is supposed to be in.
			int cell = p_index / 32;

			// figure out which index the bit is, within the cell.
			int bit = p_index % 32;

			// create a bitpattern with a 1 in the position
			// of the bit we want, then 'and' that with the
			// current cell, then shift the result back down
			// and return it.
			return (m_array[cell] & (1 << bit)) >> bit;
		}


		// this is the set function, which will set a given
		// bit within the vector.
		void Set( int p_index, bool p_value )
		{
			// figure out which cell the bit is supposed to be in.
			int cell = p_index / 32;

			// figure out which index the bit is, within the cell.
			int bit = p_index % 32;


			if( p_value == true )
				// if the value we are setting is 1 (true), 
				// then create a bitpattern with a 1 in the
				// place where we want the bit, and 'or' that
				// with the correct cell.
				m_array[cell] = (m_array[cell] | (1 << bit));
			else
				// if the value we are setting is 0 (false),
				// then create a bitpattern with a 1 in the
				// place where we want the bit, negate that so
				// that it becomes a 0, and everything else is
				// a 1, and 'and' that with the correct cell.
				m_array[cell] = (m_array[cell] & (~(1 << bit)));
		}



		// ----------------------------------------------------------------
		//  Name:           ClearAll
		//  Description:    Clears every bit to 0 in the entire vector
		//  Arguments:      None.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		void ClearAll()
		{
			int index;

			// loop through each cell and set it to 0.
			for( index = 0; index < m_size; index++ )
				m_array[index] = 0;
		}


		// ----------------------------------------------------------------
		//  Name:           SetAll
		//  Description:    Sets every bit to 1.
		//  Arguments:      None.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		void SetAll()
		{
			int index;

			// loop through each cell and set it to 0xFFFFFFFF
			for( index = 0; index < m_size; index++ )
				m_array[index] = 0xFFFFFFFF;
		}


		// ----------------------------------------------------------------
		//  Name:           Size
		//  Description:    Returns the size of the vector in bits.
		//  Arguments:      None.
		//  Return Value:   Size of the vector in bits.
		// ----------------------------------------------------------------
		int Size()
		{
			return m_size * 32;
		}



		// ----------------------------------------------------------------
		//  Name:           GetCell
		//  Description:    Gets a cell of 32 bits in the vector
		//  Arguments:      - p_index: The cell number
		//  Return Value:   The cell.
		// ----------------------------------------------------------------
		unsigned long int GetCell( int p_index )
		{
			return m_array[p_index];
		}



		// ----------------------------------------------------------------
		//  Name:           m_array
		//  Description:    a pointer to the array of bits
		// ----------------------------------------------------------------
		unsigned long int* m_array;


		// ----------------------------------------------------------------
		//  Name:           m_size
		//  Description:    the size, in cells (bits / 32) of the vector.
		// ----------------------------------------------------------------
		int m_size;
	};

};
