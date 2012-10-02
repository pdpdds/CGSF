// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// RLE.h
// This file holds the RLE compression class
// ============================================================================
#ifndef RLE_H
#define RLE_H


#include "Array.h"

namespace CGBase
{

	// ----------------------------------------------------------------
	//  Name:           RLEPair
	//  Description:    Class that stores a data and run-length pair.
	// ----------------------------------------------------------------
	template<class DataType>
	class RLEPair
	{
	public:
		DataType m_data;
		unsigned char m_length;
	};


	// ----------------------------------------------------------------
	//  Name:           RLE
	//  Description:    RLE compressor and decompressor class.
	// ----------------------------------------------------------------
	template<class DataType>
	class RLE
	{
	public:
		// typedef the pair class, to make it easier to work with
		typedef RLEPair<DataType> Pair;

		// ----------------------------------------------------------------
		//  Name:           m_RLE
		//  Description:    an array of pairs; this stores all the
		//                  compressed run data.
		// ----------------------------------------------------------------
		Array<Pair> m_RLE;

		// ----------------------------------------------------------------
		//  Name:           m_runs
		//  Description:    the number of runs in the RLE
		// ----------------------------------------------------------------
		int m_runs;

		// ----------------------------------------------------------------
		//  Name:           m_size
		//  Description:    The size of the uncompressed data.
		// ----------------------------------------------------------------
		int m_size;


		// ----------------------------------------------------------------
		//  Name:           RLE
		//  Description:    Constructor, creates an empty RLE.
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		RLE()
			: m_RLE( 1 )
		{
			m_runs = 0;
			m_size = 0;
		}


		// ----------------------------------------------------------------
		//  Name:           Compress
		//  Description:    Compresses an array of data into an RLE
		//  Arguments:      p_array: the array to compress
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Compress( Array<DataType>& p_array )
		{
			int currentrun = 0;
			int index;

			// set up the first run
			m_RLE[0].m_data = p_array[0];
			m_RLE[0].m_length = 1;

			// record the number of items in the array.
			m_size = p_array.Size();

			// start calculating the runs
			for( index = 1; index < p_array.m_size; index++ )
			{
				if( p_array[index] != m_RLE[currentrun].m_data )
				{
					// a different value was found, start a new run.
					currentrun++;

					// if the run array isn't big enough, double the size.
					if( m_RLE.m_size == currentrun )
						m_RLE.Resize( currentrun * 2 );

					// set the new run's data and length.
					m_RLE[currentrun].m_data = p_array[index];
					m_RLE[currentrun].m_length = 1;
				}
				else
				{
					// check if you need to split up a run due to length.
					if( m_RLE[currentrun].m_length == 255 )
					{
						// create a new run
						currentrun++;

						// if the run array isn't big enough, double the size.
						if( m_RLE.m_size == currentrun )
							m_RLE.Resize( currentrun * 2 );

						// set up the new runs data and length
						m_RLE[currentrun].m_data = p_array[index];
						m_RLE[currentrun].m_length = 1;
					}
					else
					{
						// increase the current runs length by 1.
						m_RLE[currentrun].m_length++;
					}
				}
			}

			// set up the number of runs
			m_runs = currentrun + 1;
		}


		// ----------------------------------------------------------------
		//  Name:           Decompress
		//  Description:    decompresses the RLE into a given array
		//  Arguments:      p_array: the array to decompress into.
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Decompress( Array<DataType>& p_array )
		{
			// make sure there's enough room for the entire RLE
			if( p_array.Size() < m_size )
				p_array.Resize( m_size );

			int currentrun;
			int index;
			int offset = 0;

			for( currentrun = 0; currentrun < m_runs; currentrun++ )
			{
				for( index = 0; index < m_RLE[currentrun].m_length; index++ )
				{
					p_array[offset + index] = m_RLE[currentrun].m_data;
				}
				offset += m_RLE[currentrun].m_length;
			}
		}


		// ----------------------------------------------------------------
		//  Name:           SaveData
		//  Description:    saves the RLE data to disk
		//  Arguments:      p_name: name of the file to save to
		//  Return Value:   None
		// ----------------------------------------------------------------
		void SaveData( char* p_name )
		{
			FILE* file = fopen( p_name, "wb" );

			// write the size of the data stored
			fwrite( &m_size, sizeof(int), 1, file );

			// write the number of runs
			fwrite( &m_runs, sizeof(int), 1, file );

			// write the actual compressed data
			fwrite( m_RLE.m_array, sizeof(Pair), m_runs, file );

			fclose( file );
		}



		// ----------------------------------------------------------------
		//  Name:           LoadData
		//  Description:    loads RLE data from disk
		//  Arguments:      p_name: name of the file to load from
		//  Return Value:   None
		// ----------------------------------------------------------------
		void LoadData( char* p_name )
		{
			FILE* file = fopen( p_name, "rb" );

			// read the size of the data stored
			fread( &m_size, sizeof(int), 1, file );

			// read the number of runs
			fread( &m_runs, sizeof(int), 1, file );

			// resize the run array if it's not large enough
			if( m_RLE.Size() < m_runs )
				m_RLE.Resize( m_runs );

			// read the actual compressed data
			fread( m_RLE.m_array, sizeof(Pair), m_runs, file );

			fclose( file );
		}
	};
};

#endif