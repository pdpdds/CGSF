// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// Array2D.h
// This is the 2D Aray class
// ============================================================================

#ifndef ARRAY2D_H
#define ARRAY2D_H

namespace CGBase
{

	template <class Datatype>
	class Array2D
	{
	public:

		// -------------------------------------------------------
		// Name:        Array2D::Array2D
		// Description: This creates the 2D array using a single
		//              linear array.
		// -------------------------------------------------------
		Array2D( int p_width, int p_height )
		{
			// create the new array
			m_array = new Datatype[ p_width * p_height ];

//20120516 Modification
		if(p_width <= 0 || p_height <= 0)
		{
			m_width = 0;
			m_height = 0;
			m_array = NULL;
			assert(1);
			return;
		}
//End

			// set the width and height
			m_width = p_width;
			m_height = p_height;
		}


		// -------------------------------------------------------
		// Name:        Array2D::~Array2D
		// Description: This destructs the array.
		// -------------------------------------------------------
		~Array2D()
		{
			// if the array is valid, delete it.
			if( m_array != 0 )
				delete[] m_array;
			m_array = 0;
		}


		// -------------------------------------------------------
		// Name:        Array2D::Get
		// Description: This retrieves the item at the given
		//              index.
		// -------------------------------------------------------
		Datatype* Get( int p_x, int p_y )
		{

//20120516 Modification
		if(p_x >= m_width || p_y >= m_height)
		{
			assert(1);
			return NULL;
		}
//End
			return &m_array[ p_y * m_width + p_x ];
		}



		// -------------------------------------------------------
		// Name:        Array2D::Resize
		// Description: This resizes the array.
		// -------------------------------------------------------
		void Resize( int p_width, int p_height )
		{
			// create a new array.
			Datatype* newarray = new Datatype[ p_width * p_height ];

			if( newarray == 0 )
				return;

			// create the two coordinate variables and the two temp 
			// variables.
			int x, y, t1, t2;

			// determine the minimum of both dimensions.
			int minx = (p_width < m_width ? p_width : m_width);
			int miny = (p_height < m_height ? p_height : m_height);

			// loop through each cell and copy everything over.
			for( y = 0; y < miny; y++ )
			{
				// OPTIMISATION: pre-calculate t1 and t2
				// the standard algorithm to calculate the
				// position in a 2d array is [y * w + x].
				// however, since y * w doesn't change at all
				// in the innermost loop, we calculate it out
				// here instead.
				t1 = y * p_width;
				t2 = y * m_width;
				for( x = 0; x < minx; x++ )
				{
					// move the data to the new array.
					newarray[ t1 + x ] = m_array[ t2 + x ];
				}
			}

			// delete the old array.        
			if( m_array != 0 )
				delete[] m_array;

			// set the new array, and the width and height.
			m_array = newarray;
			m_width = p_width;
			m_height = p_height;
		}



		// -------------------------------------------------------
		//  Name:         Array2D::Size
		//  Description:  gets the size of the array.
		//  Return Value: the size of the array.
		// -------------------------------------------------------
		int Size()
		{
			return m_width * m_height;
		}


		// -------------------------------------------------------
		//  Name:         Array2D::Width
		//  Description:  gets the width of the array.
		//  Return Value: the width of the array.
		// -------------------------------------------------------
		int Width()
		{
			return m_width;
		}


		// -------------------------------------------------------
		//  Name:         Array2D::Height
		//  Description:  gets the height of the array.
		//  Return Value: the height of the array.
		// -------------------------------------------------------
		int Height()
		{
			return m_height;
		}




		// -------------------------------------------------------
		// Name:        array2d::m_array
		// Description: This is a pointer to the array.
		// -------------------------------------------------------
		Datatype* m_array;

		// -------------------------------------------------------
		// Name:        array2d::m_width
		// Description: the current width of the array.
		// -------------------------------------------------------
		int m_width;

		// -------------------------------------------------------
		// Name:        array2d::m_height
		// Description: the current height of the array.
		// -------------------------------------------------------
		int m_height;
	};

};


#endif