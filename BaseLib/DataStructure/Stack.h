// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// Stack.h
// This file holds the two stack implementations.
// ============================================================================
#ifndef STACK_H
#define STACK_H

#include "Array.h"
#include "DLinkedList.h"


namespace CGBase
{

	// -------------------------------------------------------
	// Name:        LStack
	// Description: This is the Linked stack implementation
	// -------------------------------------------------------
	template<class Datatype>
	class LStack : public DLinkedList<Datatype>
	{
	public:

		// ----------------------------------------------------------------
		//  Name:           Push
		//  Description:    pushes data onto the stack
		//  Arguments:      p_data: the data to push
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Push( Datatype p_data )
		{
			Append( p_data );
		}


		// ----------------------------------------------------------------
		//  Name:           Pop
		//  Description:    pops data from the stack
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Pop()
		{
			RemoveTail();
		}


		// ----------------------------------------------------------------
		//  Name:           Top
		//  Description:    gets the top of the stack
		//  Arguments:      None
		//  Return Value:   reference to the top of the stack
		// ----------------------------------------------------------------
		Datatype& Top()
		{
			return m_tail->m_data;
		}


		// ----------------------------------------------------------------
		//  Name:           Count
		//  Description:    gets the number of items in the stack
		//  Arguments:      None
		//  Return Value:   number of items in stack
		// ----------------------------------------------------------------
		int Count()
		{
			return m_count;
		}

	};


	// -------------------------------------------------------
	// Name:        AStack
	// Description: This is the Arrayed stack implementation
	// -------------------------------------------------------
	template<class Datatype>
	class AStack : public Array<Datatype>
	{
	public:

		// ----------------------------------------------------------------
		//  Name:           AStack
		//  Description:    Creates a stack with a given size
		//  Arguments:      p_size: the size of the stack
		//  Return Value:   None
		// ----------------------------------------------------------------
		AStack( int p_size ) : Array<Datatype>( p_size )
		{
			m_top = 0;
		}

		// ----------------------------------------------------------------
		//  Name:           Push
		//  Description:    pushes data onto the stack
		//  Arguments:      p_data: the data to push
		//  Return Value:   true if successful
		// ----------------------------------------------------------------
		bool Push( Datatype p_data )
		{
			if( m_size != m_top )
			{
				m_array[m_top] = p_data;
				m_top++;
				return true;
			}

			return false;
		}

		// ----------------------------------------------------------------
		//  Name:           Pop
		//  Description:    pops data from the stack
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Pop()
		{
			if( m_top > 0 )
				m_top--;
		}

		// ----------------------------------------------------------------
		//  Name:           Top
		//  Description:    gets the top of the stack
		//  Arguments:      None
		//  Return Value:   reference to the top of the stack
		// ----------------------------------------------------------------
		Datatype Top()
		{
			return m_array[m_top - 1];
		}

		// ----------------------------------------------------------------
		//  Name:           Count
		//  Description:    gets the number of items in the stack
		//  Arguments:      None
		//  Return Value:   number of items in stack
		// ----------------------------------------------------------------
		int Count()
		{
			return m_top;
		}


		// ----------------------------------------------------------------
		//  Name:           Resize
		//  Description:    redefinition of the array's Resize function, 
		//                  so that the m_top variable stays valid
		//  Arguments:      p_size: size of the stack
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Resize( int p_size )
		{
			if( p_size < m_top )
			{
				m_top = p_size;
			}
			Array<Datatype>::Resize( p_size );
		}



		int m_top;
	};

};


#endif
