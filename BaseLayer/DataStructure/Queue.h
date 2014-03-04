// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// Queue.h
// This file holds the two queue implementations.
// ============================================================================
#ifndef QUEUE_H
#define QUEUE_H

#include "Array.h"
#include "DLinkedList.h"


namespace CGBase
{

	// -------------------------------------------------------
	// Name:        LQueue
	// Description: This is the Linked queue implementation
	// -------------------------------------------------------
	template<class Datatype>
	class LQueue : public DLinkedList<Datatype>
	{
	public:

		// ----------------------------------------------------------------
		//  Name:           Enqueue
		//  Description:    This enqueues data at the end of the queue.
		//  Arguments:      p_data: the data to add
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Enqueue( Datatype p_data )
		{
			Append( p_data );
		}


		// ----------------------------------------------------------------
		//  Name:           Dequeue
		//  Description:    Dequeues the front of the queue
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Dequeue()
		{
			RemoveHead();
		}


		// ----------------------------------------------------------------
		//  Name:           Front
		//  Description:    gets the front of the queue
		//  Arguments:      None
		//  Return Value:   a reference to the item at the front.
		// ----------------------------------------------------------------
		Datatype& Front()
		{
			return m_head->m_data;
		}


		// ----------------------------------------------------------------
		//  Name:           Count
		//  Description:    Gets the number of items in the queue
		//  Arguments:      None
		//  Return Value:   The number of items in the queue
		// ----------------------------------------------------------------
		int Count()
		{
			return m_count;
		}

	};


	// -------------------------------------------------------
	// Name:        AQueue
	// Description: This is the Arrayed queue implementation
	// -------------------------------------------------------
	template<class Datatype>
	class AQueue : public Array<Datatype>
	{
	public:

		// ----------------------------------------------------------------
		//  Name:           AQueue
		//  Description:    Constructor, creates an empty queue with a 
		//                  given size.
		//  Arguments:      p_size: the size of the queue
		//  Return Value:   None
		// ----------------------------------------------------------------
		AQueue( int p_size ) : Array<Datatype>( p_size )
		{
			m_front = 0;
			m_count = 0;
		}


		// ----------------------------------------------------------------
		//  Name:           Enqueue
		//  Description:    This enqueues data at the end of the queue.
		//  Arguments:      p_data: the data to add
		//  Return Value:   true if successful
		// ----------------------------------------------------------------
		bool Enqueue( Datatype p_data )
		{
			// if the queue is not full
			if( m_size != m_count )
			{
				// add the new item at the first open circular index
				m_array[(m_count + m_front) % m_size] = p_data;
				m_count++;
				return true;
			}

			return false;
		}


		// ----------------------------------------------------------------
		//  Name:           Dequeue
		//  Description:    Dequeues the front of the queue
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Dequeue()
		{
			if( m_count > 0 )
			{
				// decrease the count
				m_count--;

				// increment the front index
				m_front++;

				// if the front went past the end, put it back to 0.
				if( m_front == m_size )
					m_front = 0;
			}
		}

		// ----------------------------------------------------------------
		//  Name:           Front
		//  Description:    gets the front of the queue
		//  Arguments:      None
		//  Return Value:   a reference to the item at the front.
		// ----------------------------------------------------------------
		Datatype& Front()
		{
			return m_array[m_front];
		}


		// ----------------------------------------------------------------
		//  Name:           Count
		//  Description:    Gets the number of items in the queue
		//  Arguments:      None
		//  Return Value:   The number of items in the queue
		// ----------------------------------------------------------------
		int Count()
		{
			return m_count;
		}


		// ----------------------------------------------------------------
		//  Name:           operator[]
		//  Description:    overload of the index-access operator, so that
		//                  it accesses cells relative to the front index.
		//  Arguments:      p_index: the index to access
		//  Return Value:   the item at the given relative index.
		// ----------------------------------------------------------------
		Datatype& operator[] ( int p_index )
		{
			return m_array[(p_index + m_front) % m_size];
		}


		// ----------------------------------------------------------------
		//  Name:           m_front
		//  Description:    an index pointing to the front item in the queue
		// ----------------------------------------------------------------
		int m_front;

		// ----------------------------------------------------------------
		//  Name:           m_count
		//  Description:    the number of items in the queue
		// ----------------------------------------------------------------
		int m_count;
	};

}



#endif
