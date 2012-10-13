// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// Heap.h
// This is the heap class
// ============================================================================

#ifndef HEAP_H
#define HEAP_H

#include "Array.h"

namespace CGBase
{

	// ----------------------------------------------------------------
	//  Name:           Heap
	//  Description:    This is the heap class, an efficient priority
	//                  queue.
	// ----------------------------------------------------------------
	template <class DataType>
	class Heap : public Array<DataType>
	{
	public:


		// ----------------------------------------------------------------
		//  Name:           Heap
		//  Description:    constructor; creates a heap with a size
		//                  and a comparison function
		//  Arguments:      p_size: the size of the heap.
		//                  p_compare: a comparison function pointer.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		Heap( int p_size, int (*p_compare)(DataType, DataType) )
			: Array<DataType>( p_size + 1 )
		{
			m_count = 0;
			m_compare = p_compare;
		}


		// ----------------------------------------------------------------
		//  Name:           Enqueue
		//  Description:    Adds new data to the queue.
		//  Arguments:      p_data: the data to add
		//  Return Value:   None.
		// ----------------------------------------------------------------
		void Enqueue( DataType p_data )
		{
			m_count++;

			// if we ran out of room, double the size of the heap.
			if( m_count >= m_size )
				Resize( m_size * 2 );

			// insert the data at the bottom of the heap
			m_array[m_count] = p_data;

			// walk it up to its correct position.
			WalkUp( m_count );
		}


		// ----------------------------------------------------------------
		//  Name:           Dequeue
		//  Description:    removes the top of the heap
		//  Arguments:      None
		//  Return Value:   None.
		// ----------------------------------------------------------------
		void Dequeue()
		{
			// if the count is more than or equal to one, then we can remove
			// the item, else, there is nothing to remove.
			if( m_count >= 1 )
			{
				m_array[1] = m_array[m_count];
				WalkDown( 1 );
				m_count--;
			}
		}


		// ----------------------------------------------------------------
		//  Name:           Item
		//  Description:    gets the top of the heap
		//  Arguments:      None
		//  Return Value:   a reference to the top of the heap.
		// ----------------------------------------------------------------
		DataType& Item()
		{
			return m_array[1];
		}



		// ============================
		//        HEAP HELPERS
		// ============================


		// ----------------------------------------------------------------
		//  Name:           WalkUp
		//  Description:    Walks an item up the heap into the right
		//                  position.
		//  Arguments:      p_index: the index of the place to start
		//  Return Value:   None.
		// ----------------------------------------------------------------
		void WalkUp( int p_index )
		{
			// set up the parent and child indexes
			int parent = p_index / 2;
			int child = p_index;

			// store the item to walk up in a temporary buffer.
			DataType temp = m_array[child];

			while( parent > 0 )
			{   // if the node to walk up is more than the parent,
				// then swap nodes.
				if( m_compare( temp, m_array[parent] ) > 0 )
				{
					// swap the parent and child, and go up a level.
					m_array[child] = m_array[parent];
					child = parent;
					parent /= 2;
				}
				else
					break;
			}

			// put the temp variable (the one that was walked up)
			// into the child index.
			m_array[child] = temp;
		}


		// ----------------------------------------------------------------
		//  Name:           WalkDown
		//  Description:    Walks an item down the heap into the right
		//                  place.
		//  Arguments:      p_index: index of the item to start at.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		void WalkDown( int p_index )
		{
			// calculate the parent and child indexes.
			int parent = p_index;
			int child = p_index * 2;

			// store the data to walk down in a temporary buffer.
			DataType temp = m_array[parent];

			// loop through, walking node down heap until both children are
			// smaller than node.
			while( child < m_count )
			{
				// if left child is not the last node in the tree, then
				// find out which of the current node's children is largest.
				if( child < m_count - 1)
				{
					if( m_compare( m_array[child], m_array[child + 1] ) < 0 )
					{   // change the pointer to the right child, since it is larger.
						child++;
					}
				}
				// if the node to walk down is lower than the highest value child,
				// move the child up one level.
				if( m_compare( temp, m_array[child] ) < 0 )
				{
					m_array[parent] = m_array[child];
					parent = child;
					child *= 2;
				}
				else
					break;
			}
			m_array[parent] = temp;
		}


		// ----------------------------------------------------------------
		//  Name:           m_count
		//  Description:    The number of items in the heap.
		// ----------------------------------------------------------------
		int m_count;

		// ----------------------------------------------------------------
		//  Name:           m_compare
		//  Description:    The comparison function.
		// ----------------------------------------------------------------
		int (*m_compare)(DataType, DataType);
	};

};

#endif