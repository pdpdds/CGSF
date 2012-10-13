// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// Huffman.h
// This file holds the Huffman Tree compression class
// ============================================================================
#ifndef HUFFMAN_H
#define HUFFMAN_H


#include "Array.h"
#include "Bitvector.h"
#include "BinaryTree.h"
#include "Heap.h"

namespace CGBase
{

	// ----------------------------------------------------------------
	//  Name:           HuffmanNode
	//  Description:    This is the node class that is stored in
	//                  the huffman tree.
	// ----------------------------------------------------------------
	template<class DataType>
	class HuffmanNode
	{
	public:

		// ----------------------------------------------------------------
		//  Name:           m_data
		//  Description:    The data to store in the node
		// ----------------------------------------------------------------
		DataType m_data;

		// ----------------------------------------------------------------
		//  Name:           m_code
		//  Description:    The huffman code that represents this data.
		// ----------------------------------------------------------------
		unsigned long int m_code;

		// ----------------------------------------------------------------
		//  Name:           m_codeLength
		//  Description:    The length of the code, in bits
		// ----------------------------------------------------------------
		int m_codeLength;
	};


	// ----------------------------------------------------------------
	//  Name:           HuffmanFrequency
	//  Description:    This class stores frequency information
	//                  about a piece of data.
	// ----------------------------------------------------------------
	template<class DataType>
	class HuffmanFrequency
	{
	public:
		DataType m_data;
		int m_frequency;
	};


	// ----------------------------------------------------------------
	//  Name:           CompareNodes
	//  Description:    This is a comparison function, for use in the
	//                  tree creation algorithm. Nodes with a higher
	//                  frequency are "lower", becaue items with the
	//                  lowest frequencies need to be used first.
	//  Arguments:      left, right: the nodes
	//  Return Value:   the comparison value.
	// ----------------------------------------------------------------
	template<class DataType>
	int CompareNodes( BinaryTree< HuffmanFrequency<DataType> >* left,
		BinaryTree< HuffmanFrequency<DataType> >* right )
	{
		// return right-left; smaller frequencies are "Higher"
		return right->m_data.m_frequency - left->m_data.m_frequency;
	}



	// ----------------------------------------------------------------
	//  Name:           Huffman
	//  Description:    The huffman encoding/decoding class.
	// ----------------------------------------------------------------
	template<class DataType, unsigned long int MaxValue>
	class Huffman
	{
	public:

		// typedef the classes to make them easier to use.
		typedef HuffmanNode<DataType> Node;
		typedef HuffmanFrequency<DataType> Frequency;
		typedef BinaryTree<Frequency> TreeNode;


		// ----------------------------------------------------------------
		//  Name:           m_compressedData
		//  Description:    This is the compressed data.
		// ----------------------------------------------------------------
		Bitvector m_compressedData;

		// ----------------------------------------------------------------
		//  Name:           m_dataLength
		//  Description:    The length of the data stored.
		// ----------------------------------------------------------------
		int m_dataLength;

		// ----------------------------------------------------------------
		//  Name:           m_compressedLength
		//  Description:    the length, in bits, of the compressed data
		// ----------------------------------------------------------------
		int m_compressedLength;

		// ----------------------------------------------------------------
		//  Name:           m_huffmanTree
		//  Description:    The huffman tree
		// ----------------------------------------------------------------
		Array<Node> m_huffmanTree;

		// ----------------------------------------------------------------
		//  Name:           m_maxEntry
		//  Description:    This is the maximum entry in the arrayed
		//                  representation of the huffman tree.
		// ----------------------------------------------------------------
		int m_maxEntry;

		// ----------------------------------------------------------------
		//  Name:           m_lookupTable
		//  Description:    This is the lookup table of nodes
		// ----------------------------------------------------------------
		Array<Node> m_lookupTable;


		// ----------------------------------------------------------------
		//  Name:           Huffman
		//  Description:    Constructor, creates an empty huffman tree.
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		Huffman()
			:   m_compressedData( 1 ),
			m_huffmanTree( 1 ),
			m_lookupTable( MaxValue + 1 )
		{
			m_dataLength = 0;
			m_compressedLength = 0;
			m_maxEntry = 0;
		}



		// ----------------------------------------------------------------
		//  Name:           CalculateTree
		//  Description:    this calculates a huffman tree for a given
		//                  array of data
		//  Arguments:      p_array: the array of data.
		//  Return Value:   None
		// ----------------------------------------------------------------
		void CalculateTree( Array<DataType>& p_array )
		{
			// create an array to hold the frequency table.
			Array<int> frequencyTable( MaxValue + 1 );

			int index;

			// clear the frequency table
			for( index = 0; index <= MaxValue; index++ )
			{
				frequencyTable[index] = 0;
			}

			// calculate the frequency table
			for( index = 0; index < p_array.Size(); index++ )
			{
				frequencyTable[ p_array[index] ]++;
			}

			// create a heap that can hold every entry in the table
			Heap<TreeNode*> heap( frequencyTable.Size(), CompareNodes );

			TreeNode* parent;
			TreeNode* left;
			TreeNode* right;

			// first go though the freqency table and create a node for them,
			// and add them into the heap.
			for( index = 0; index <= MaxValue; index++ )
			{
				// only add the item if it has a freqency
				if( frequencyTable[index] != 0 )
				{
					parent = new TreeNode;
					parent->m_data.m_data = index; 
					parent->m_data.m_frequency = frequencyTable[index];
					heap.Enqueue( parent );
				}
			}


			// now that all of the nodes are in the queue,
			// perform the huffman tree creation.
			while( heap.m_count > 1 )
			{
				// remove the first two items from the heap
				left = heap.Item();
				heap.Dequeue();
				right = heap.Item();
				heap.Dequeue();

				// create the new parent node
				parent = new TreeNode;
				parent->m_left = left;
				parent->m_right = right;
				parent->m_data.m_frequency = left->m_data.m_frequency + 
					right->m_data.m_frequency;

				// add the new node into the queue again.
				heap.Enqueue( parent );
			}


			// calculation of the tree is complete, now convert the tree into the
			// binary tree array.
			ConvertTreeToArray( heap.m_array[1] );

			// create the lookup table now
			CreateLookupTable();


			// delete the tree once you are done with it.
			delete heap.m_array[1];
		}


		// ----------------------------------------------------------------
		//  Name:           Compress
		//  Description:    Compresses an array of data into the
		//                  m_compressedData bitvector
		//  Arguments:      p_array: the array to compress.
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Compress( Array<DataType>& p_array )
		{
			int index;
			int vectorindex = 0;
			int codeindex;
			int codelength;
			unsigned long int code;
			bool value;

			// this routine goes through the entire array.
			//  - looks up each item in the huffman tree to find out its code
			//  - adds the code to the bitvector.

			for( index = 0; index < p_array.Size(); index++ )
			{
				// look up the code and its length
				code = m_lookupTable[ p_array[index] ].m_code;
				codelength = m_lookupTable[ p_array[index] ].m_codeLength;

				// resize vector if needed.
				if( m_compressedData.Size() < vectorindex + codelength )
					m_compressedData.Resize( m_compressedData.Size() * 2 );

				for( codeindex = 0; codeindex < codelength; codeindex++ )
				{
					value = (1 << codeindex) & code;
					m_compressedData.Set( vectorindex, value );
					vectorindex++;
				}
			}

			// record the size of the data in the bitvector
			m_compressedLength = vectorindex;
			m_dataLength = p_array.Size();
		}


		// ----------------------------------------------------------------
		//  Name:           Decompress
		//  Description:    This decompresses the data in the bitvector
		//                  into p_array.
		//  Arguments:      p_array
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Decompress( Array<DataType>& p_array )
		{
			int vectorindex;
			int arrayindex = 0;
			int treeindex = 1;
			int value;

			// resize the array to make room
			if( p_array.Size() < m_dataLength )
				p_array.Resize( m_dataLength );

			// go through each bit in the vector
			for( vectorindex = 0; vectorindex < m_compressedLength; vectorindex++ )
			{
				// calculate the value of the current bit, then
				// travel down the tree.
				value = m_compressedData[vectorindex];
				treeindex = treeindex * 2 + value;

				// if the node's current code length is not zero, then you've
				// found a leaf node
				if( m_huffmanTree[treeindex-1].m_codeLength != 0 )
				{
					// we found a leaf node, so copy it into the array
					p_array[arrayindex] = m_huffmanTree[treeindex-1].m_data;

					// increment the array index
					arrayindex++;

					// reset the tree index.
					treeindex = 1;
				}
			}
		}


		// ----------------------------------------------------------------
		//  Name:           SaveTree
		//  Description:    This saves the tree to disk
		//  Arguments:      p_name: the name of the file.
		//  Return Value:   None
		// ----------------------------------------------------------------
		void SaveTree( char* p_name )
		{
			// open the file
			FILE* file = fopen( p_name, "wb" );

			// write the tree.
			fwrite( &m_maxEntry, sizeof(int), 1, file );
			fwrite( m_huffmanTree.m_array, sizeof(Node), m_maxEntry, file );

			fclose( file );
		}


		// ----------------------------------------------------------------
		//  Name:           LoadTree
		//  Description:    Loads a tree from disk.
		//  Arguments:      p_name: name of the file.
		//  Return Value:   None
		// ----------------------------------------------------------------
		void LoadTree( char* p_name )
		{
			FILE* file = fopen( p_name, "rb" );

			// read the tree size
			fread( &m_maxEntry, sizeof(int), 1, file );

			// resize the array to hold the tree
			m_huffmanTree.Resize( m_maxEntry );

			// read the tree
			fread( m_huffmanTree.m_array, sizeof(Node), m_maxEntry, file );

			fclose( file );

			CreateLookupTable();
		}


		// ----------------------------------------------------------------
		//  Name:           SaveData
		//  Description:    Saves the compressed data to disk
		//  Arguments:      p_name: name of the file
		//  Return Value:   None
		// ----------------------------------------------------------------
		void SaveData( char* p_name )
		{
			FILE* file = fopen( p_name, "wb" );

			// write the size of the data stored
			fwrite( &m_dataLength, sizeof(int), 1, file );

			// write the size of the binary data stored
			fwrite( &m_compressedLength, sizeof(int), 1, file );

			// write the actual compressed data
			fwrite( m_compressedData.m_array, 
				sizeof(unsigned long int),
				(m_compressedLength / 32) + 1,
				file );

			fclose( file );
		}



		// ----------------------------------------------------------------
		//  Name:           LoadData
		//  Description:    Loads compressed data from disk
		//  Arguments:      p_name: name of the file
		//  Return Value:   None
		// ----------------------------------------------------------------
		void LoadData( char* p_name )
		{
			FILE* file = fopen( p_name, "rb" );

			// read the size of the data stored
			fread( &m_dataLength, sizeof(int), 1, file );

			// read the size of the binary data stored
			fread( &m_compressedLength, sizeof(int), 1, file );

			// resize the bitvector to store the data
			m_compressedData.Resize( m_compressedLength );

			// read the actual compressed data
			fread( m_compressedData.m_array, 
				sizeof(unsigned long int),
				m_compressedData.m_size,
				file );

			fclose( file );
		}


		// ----------------------------------------------------------------
		//  Name:           ConvertTreeToArray
		//  Description:    This converts the linked tree produced by
		//                  the huffman algorithm into an arrayed tree.
		//  Arguments:      p_tree: the root of the linked tree.
		//  Return Value:   None
		// ----------------------------------------------------------------
		void ConvertTreeToArray( TreeNode* p_tree )
		{
			int index;

			// resize the tree to hold the right amount of nodes
			index = ( 2 << GetDepth( p_tree ) ) - 1;

			if( m_huffmanTree.Size() < index )
				m_huffmanTree.Resize( index );

			// clear the existing tree
			m_maxEntry = 0;
			for( index = 0; index < m_huffmanTree.Size(); index++ )
			{
				m_huffmanTree[index].m_code = 0;
				m_huffmanTree[index].m_codeLength = 0;
			}

			Convert( p_tree, 1, 0, 0 );

		}



		// ----------------------------------------------------------------
		//  Name:           Convert
		//  Description:    This is a recursive helper function that
		//                  does the actual tree->array converting.
		//  Arguments:      p_tree: the current node to convert
		//                  p_index: the index of the node in the array
		//                  p_length: the length of the node's code
		//                  p_code: the code for the node.
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Convert( TreeNode* p_tree, 
			int p_index, 
			int p_length, 
			unsigned long int p_code )
		{
			// chack to see if the node is a leaf or not.
			if( p_tree->m_left == 0 && p_tree->m_right == 0 )
			{
				// if this node is higher than the max entry, set
				// a new max entry.
				if( p_index > m_maxEntry )
					m_maxEntry = p_index;

				// set up the entry
				m_huffmanTree[p_index-1].m_data = p_tree->m_data.m_data;
				m_huffmanTree[p_index-1].m_code = p_code;
				m_huffmanTree[p_index-1].m_codeLength = p_length;
			}
			else
			{
				// else, recurse down the left and right children until
				// a leaf is found.
				if( p_tree->m_left != 0 )
				{
					Convert( p_tree->m_left, 
						p_index * 2, 
						p_length + 1, 
						p_code );
				}
				if( p_tree->m_right != 0 )
				{
					Convert( p_tree->m_right, 
						p_index * 2 + 1, 
						p_length + 1, 
						p_code | (1 << p_length) );
				}
			}
		}


		// ----------------------------------------------------------------
		//  Name:           CreateLookupTable
		//  Description:    This creates the lookup table of nodes.
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void CreateLookupTable()
		{
			int index;

			// loop through each tree entry
			for( index = 0; index < m_maxEntry; index++ )
			{
				// if the tree entry is valid, 
				if( m_huffmanTree[index].m_codeLength != 0 )
				{
					// store the entry into the lookuptable
					m_lookupTable[ m_huffmanTree[index].m_data ] = 
						m_huffmanTree[index];
				}
			}
		}
	};

};
#endif