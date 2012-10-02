// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// HashTable.h
// This file holds the Linekd Hash Table implementation.
// ============================================================================


#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "DLinkedList.h"
#include "Array.h"

namespace CGBase
{


	// -------------------------------------------------------
	// Name:        HashEntry
	// Description: This is the hash table entry class. It
	//              stores a key and data pair.
	// -------------------------------------------------------
	template< class KeyType, class DataType >
	class HashEntry
	{
	public:
		KeyType m_key;
		DataType m_data;
	};



	// -------------------------------------------------------
	// Name:        HashTable
	// Description: This is the hashtable class.
	// -------------------------------------------------------
	template< class KeyType, class DataType >
	class HashTable
	{
	public:

		// typedef the entry class to make is easier to work with.
		typedef HashEntry<KeyType, DataType> Entry;


		// ----------------------------------------------------------------
		//  Name:           HashTable
		//  Description:    construct the table with a size, and a hash 
		//                  function. The constructor will construct the 
		//                  m_table array with the correct size.
		//  Arguments:      p_size: The size of the table
		//                  p_hash: the hashing function.
		//  Return Value:   None
		// ----------------------------------------------------------------
		HashTable( int p_size, unsigned long int (*p_hash)(KeyType) )
			: m_table( p_size )
		{
			// set the size, hash function, and count.
			m_size = p_size;
			m_hash = p_hash;
			m_count = 0;
		}


		// ----------------------------------------------------------------
		//  Name:           Insert
		//  Description:    Inserts a new key/data pair into the table. 
		//  Arguments:      p_key: the key
		//                  p_data: the data attached to the key.
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Insert( KeyType p_key, DataType p_data )
		{
			// create an entry structure.
			Entry entry;
			entry.m_data = p_data;
			entry.m_key = p_key;

			// get the hash value from the key, and modulo it
			// so that it fits within the table.
			int index = m_hash( p_key ) % m_size;

			// add the entry to the correct index, increment the count.
			m_table[index].Append( entry );
			m_count++;
		}


		// ----------------------------------------------------------------
		//  Name:           Find
		//  Description:    Finds a key in the table
		//  Arguments:      p_key: the key to search for
		//  Return Value:   a pointer to the entry that has the key/data,
		//                  or 0 if not found.
		// ----------------------------------------------------------------
		Entry* Find( KeyType p_key )
		{
			// find out which index the key should exist in
			int index = m_hash( p_key ) % m_size;

			// get an iterator for the list in that index.
			DListIterator<Entry> itr = m_table[index].GetIterator();

			// search each item
			while( itr.Valid() )
			{
				// if the keys match, then return a pointer to the entry
				if( itr.Item().m_key == p_key )
					return &(itr.Item());
				itr.Forth();
			}

			// no match was found, return 0.
			return 0;
		}


		// ----------------------------------------------------------------
		//  Name:           Remove
		//  Description:    Removes an entry based on key 
		//  Arguments:      p_key: the key
		//  Return Value:   true if removed, false if not found.
		// ----------------------------------------------------------------
		bool Remove( KeyType p_key )
		{
			// find the index that the key should be in.
			int index = m_hash( p_key ) % m_size;

			// get an iterator for the list in that index.
			DListIterator<Entry> itr = m_table[index].GetIterator();

			// search each item
			while( itr.Valid() )
			{
				// if the keys match, then remove the node, and return true.
				if( itr.Item().m_key == p_key )
				{
					m_table[index].Remove( itr );
					m_count--;
					return true;
				}
				itr.Forth();
			}

			// item wasn't found, return false.
			return false;
		}


		// ----------------------------------------------------------------
		//  Name:           Count
		//  Description:    Gets the number of entries in the table.
		//  Arguments:      None
		//  Return Value:   Number of entries in the table.
		// ----------------------------------------------------------------
		int Count()
		{
			return m_count;
		}


		// ----------------------------------------------------------------
		//  Name:           m_size
		//  Description:    This is the size of the table
		// ----------------------------------------------------------------
		int m_size;

		// ----------------------------------------------------------------
		//  Name:           m_count
		//  Description:    This is the number of entries in the table.
		// ----------------------------------------------------------------
		int m_count;

		// ----------------------------------------------------------------
		//  Name:           m_table
		//  Description:    This is the actual table, a list of linked 
		//                  lists of entries.
		// ----------------------------------------------------------------
		Array< DLinkedList< Entry > > m_table;

		// ----------------------------------------------------------------
		//  Name:           m_hash
		//  Description:    a pointer to the hash function.
		// ----------------------------------------------------------------
		unsigned long int (*m_hash)(KeyType);


	};
};

#endif

