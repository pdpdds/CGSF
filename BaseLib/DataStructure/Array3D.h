// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// Array3D
// This is the 3D Array Class.
// ============================================================================
#ifndef ARRAY3D_H
#define ARRAY3D_H

namespace CGBase
{

	template <class Datatype>
	class Array3D
	{
	public:

		// -------------------------------------------------------
		// Name:        Array3D::Array3D
		// Description: This creates the 2d array using a sinle
		//              linear array.
		// -------------------------------------------------------
		Array3D( int p_width, int p_height, int p_depth )
		{
			// create a new array
			m_array = new Datatype[ p_width * p_height * p_depth ];

			// set the size variables
			m_width = p_width;
			m_height = p_height;
			m_depth = p_depth;
		}


		// -------------------------------------------------------
		// Name:        Array3D::~Array3D
		// Description: This destructs the array.
		// -------------------------------------------------------
		~Array3D()
		{
			// if the array exists, delete it.
			if( m_array != 0 )
				delete[] m_array;
			m_array = 0;
		}


		// -------------------------------------------------------
		// Name:        Array3D::Get
		// Description: This retrieves the item at the given
		//              index.
		// -------------------------------------------------------
		Datatype& Get( int p_x, int p_y, int p_z )
		{
			return m_array[ (p_z * m_width * m_height) + 
				(p_y * m_width) + 
				p_x ];
		}



		// -------------------------------------------------------
		// Name:        Array3D::Resize
		// Description: This resizes the array.
		// -------------------------------------------------------
		void Resize( int p_width, int p_height, int p_depth )
		{
			// create a new array.
			Datatype* newarray = new Datatype[ p_width * p_height * p_depth ];

			if( newarray == 0 )
				return;

			// create the three coordinate variables and the four temp 
			// variables.
			int x, y, z, t1, t2, t3, t4;

			// determine the minimum of all dimensions.
			int minx = (p_width < m_width ? p_width : m_width);
			int miny = (p_height < m_height ? p_height : m_height);
			int minz = (p_depth < m_depth ? p_depth : m_depth);

			// loop through each cell and copy everything over.
			for( z = 0; z < minz; z++ )
			{
				// precalculate the outter term (z) of the 
				// access algorithm
				t1 = z * p_width * p_height;
				t2 = z * m_width * m_height;
				for( y = 0; y < miny; y++ )
				{
					// precalculate the middle term (y) of the
					// access algorithm
					t3 = y * p_width;
					t4 = y * m_width;
					for( x = 0; x < minx; x++ )
					{
						// move the data to the new array.
						newarray[ t1 + t3 + x ] = m_array[ t2 + t4 + x ];
					}
				}
			}

			// delete the old array.        
			if( m_array != 0 )
				delete[] m_array;

			// set the new array, and the width and height.
			m_array = newarray;
			m_width = p_width;
			m_height = p_height;
			m_depth = p_depth;
		}



		// -------------------------------------------------------
		//  Name:         Array3D::Size
		//  Description:  gets the size of the array.
		//  Return Value: the size of the array.
		// -------------------------------------------------------
		int Size()
		{
			return m_width * m_height * m_depth;
		}


		// -------------------------------------------------------
		//  Name:         Array3D::Width
		//  Description:  gets the width of the array.
		//  Return Value: the width of the array.
		// -------------------------------------------------------
		int Width()
		{
			return m_width;
		}


		// -------------------------------------------------------
		//  Name:         Array3D::Height
		//  Description:  gets the height of the array.
		//  Return Value: the height of the array.
		// -------------------------------------------------------
		int Height()
		{
			return m_height;
		}


		// -------------------------------------------------------
		//  Name:         Array3D::Depth
		//  Description:  gets the depth of the array.
		//  Return Value: the depth of the array.
		// -------------------------------------------------------
		int Depth()
		{
			return m_depth;
		}

		// -------------------------------------------------------
		// Name:        Array3D::m_array
		// Description: This is a pointer to the array.
		// -------------------------------------------------------
		Datatype* m_array;

		// -------------------------------------------------------
		// Name:        Array3D::m_width
		// Description: the current width of the array.
		// -------------------------------------------------------
		int m_width;

		// -------------------------------------------------------
		// Name:        Array3D::m_height
		// Description: the current height of the array.
		// -------------------------------------------------------
		int m_height;

		// -------------------------------------------------------
		// Name:        Array3D::m_depth
		// Description: the current depth of the array.
		// -------------------------------------------------------
		int m_depth;
	};

};


#endif