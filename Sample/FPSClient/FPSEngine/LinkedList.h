//-----------------------------------------------------------------------------
// Implementation of a linked list collection.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

//-----------------------------------------------------------------------------
// Linked List Class
//-----------------------------------------------------------------------------
template< class Type > class LinkedList
{
public:
	//-------------------------------------------------------------------------
	// Element Structure
	//-------------------------------------------------------------------------
	struct Element
	{
		Type *data; // Pointer to the data held in the element.
		Element *next; // Pointer to the next element in the list.
		Element *prev; // Pointer to the previous element in the list.

		//---------------------------------------------------------------------
		// The element structure constructor.
		//---------------------------------------------------------------------
		Element( Type *element )
		{
			data = element;
			next = prev = NULL;
		}

		//---------------------------------------------------------------------
		// The element structure destructor.
		//---------------------------------------------------------------------
		~Element()
		{
			SAFE_DELETE( data );

			if( next )
				next->prev = prev;
			if( prev )
				prev->next = next;
		}
	};

	//-------------------------------------------------------------------------
	// The linked list class constructor.
	//-------------------------------------------------------------------------
	LinkedList()
	{
		m_first = m_last = m_iterate = m_temp = NULL;
		m_totalElements = 0;
	}

	//-------------------------------------------------------------------------
	// The linked list class destructor.
	//-------------------------------------------------------------------------
	~LinkedList()
	{
		Empty();
	}

	//-------------------------------------------------------------------------
	// Adds the given element to the end of the linked list.
	//-------------------------------------------------------------------------
	Type *Add( Type *element )
	{
		if( element == NULL )
			return NULL;

		if( m_first == NULL )
		{
			m_first = new Element( element );
			m_last = m_first;
		}
		else
		{
			m_last->next = new Element( element );
			m_last->next->prev = m_last;
			m_last = m_last->next;
		}

		m_totalElements++;

		return m_last->data;
	}

	//-------------------------------------------------------------------------
	// Inserts the given element into the linked list just before nextElement.
	//-------------------------------------------------------------------------
	Type *InsertBefore( Type *element, Element *nextElement )
	{
		m_temp = nextElement->prev;

		m_totalElements++;

		if( m_temp == NULL )
		{
			m_first = new Element( element );
			m_first->next = nextElement;
			nextElement->prev = m_first;

			return m_first->data;
		}
		else
		{
			m_temp->next = new Element( element );
			m_temp->next->prev = m_temp;
			m_temp->next->next = nextElement;
			nextElement->prev = m_temp->next;

			return m_temp->next->data;
		}
	}

	//-------------------------------------------------------------------------
	// Removes the given element from the linked list and destroys its data.
	//-------------------------------------------------------------------------
	void Remove( Type **element )
	{
		m_temp = m_first;
		while( m_temp != NULL )
		{
			if( m_temp->data == *element )
			{
				if( m_temp == m_first )
				{
					m_first = m_first->next;
					if( m_first )
						m_first->prev = NULL;
				}
				if( m_temp == m_last )
				{
					m_last = m_last->prev;
					if( m_last )
						m_last->next = NULL;
				}

				SAFE_DELETE( m_temp );

				*element = NULL;

				m_totalElements--;

				return;
			}

			m_temp = m_temp->next;
		}
	}

	//-------------------------------------------------------------------------
	// Destroys all the elements in the linked list as well as their data.
	//-------------------------------------------------------------------------
	void Empty()
	{
		while( m_last != NULL )
		{
			m_temp = m_last;
			m_last = m_last->prev;
			SAFE_DELETE( m_temp );
		}
		m_first = m_last = m_iterate = m_temp = NULL;
		m_totalElements = 0;
	}

	//-------------------------------------------------------------------------
	// Removes all the elements and clears their data pointers.
	// Warning: This function does not destroy the data held be each element.
	//-------------------------------------------------------------------------
	void ClearPointers()
	{
		while( m_last != NULL )
		{
			m_temp = m_last;
			m_temp->data = NULL;
			m_last = m_last->prev;
			SAFE_DELETE( m_temp );
		}
		m_first = m_last = m_iterate = m_temp = NULL;
		m_totalElements = 0;
	}

	//-------------------------------------------------------------------------
	// Removes the given element and clears its data pointer.
	// Warning: This function does not destroy the data held by the element.
	//-------------------------------------------------------------------------
	void ClearPointer( Type **element )
	{
		m_temp = m_first;
		while( m_temp != NULL )
		{
			if( m_temp->data == *element )
			{
				if( m_temp == m_first )
				{
					m_first = m_first->next;
					if( m_first )
						m_first->prev = NULL;
				}
				if( m_temp == m_last )
				{
					m_last = m_last->prev;
					if( m_last )
						m_last->next = NULL;
				}

				m_temp->data = NULL;

				SAFE_DELETE( m_temp );

				*element = NULL;

				m_totalElements--;

				return;
			}

			m_temp = m_temp->next;
		}
	}

	//-------------------------------------------------------------------------
	// Iterates through the elements in the linked list.
	//-------------------------------------------------------------------------
	Type *Iterate( bool restart = false )
	{
		if( restart )
			m_iterate = NULL;
		else
		{
			if( m_iterate == NULL )
				m_iterate = m_first;
			else
				m_iterate = m_iterate->next;
		}

		if( m_iterate == NULL )
			return NULL;
		else
			return m_iterate->data;
	}

	//-------------------------------------------------------------------------
	// Returns the currently iterated element in the linked list.
	//-------------------------------------------------------------------------
	Type *GetCurrent()
	{
		if( m_iterate )
			return m_iterate->data;
		else
			return NULL;
	}

	//-------------------------------------------------------------------------
	// Returns the first element in the linked list.
	//-------------------------------------------------------------------------
	Type *GetFirst()
	{
		if( m_first )
			return m_first->data;
		else
			return NULL;
	}

	//-------------------------------------------------------------------------
	// Returns the last element in the linked list.
	//-------------------------------------------------------------------------
	Type *GetLast()
	{
		if( m_last )
			return m_last->data;
		else
			return NULL;
	}

	//-------------------------------------------------------------------------
	// Returns the next element in the linked list from the given element.
	//-------------------------------------------------------------------------
	Type *GetNext( Type *element )
	{
		m_temp = m_first;
		while( m_temp != NULL )
		{
			if( m_temp->data == element )
			{
				if( m_temp->next == NULL )
					return NULL;
				else
					return m_temp->next->data;
			}

			m_temp = m_temp->next;
		}

		return NULL;
	}

	//-------------------------------------------------------------------------
	// Returns a random element from the linked list.
	//-------------------------------------------------------------------------
	Type *GetRandom()
	{
		if( m_totalElements == 0 )
			return NULL;
		else if( m_totalElements == 1 )
			return m_first->data;

		unsigned long element = rand() * m_totalElements / RAND_MAX;

		m_temp = m_first;
		for( unsigned long e = 0; e < element; e++ )
			m_temp = m_temp->next;

		return m_temp->data;
	}

	//-------------------------------------------------------------------------
	// Returns the complete element (including its next and previous pointers).
	//-------------------------------------------------------------------------
	Element *GetCompleteElement( Type *element )
	{
		m_temp = m_first;
		while( m_temp != NULL )
		{
			if( m_temp->data == element )
					return m_temp;

			m_temp = m_temp->next;
		}

		return NULL;
	}

	//-------------------------------------------------------------------------
	// Returns the total number of elements in the linked list.
	//-------------------------------------------------------------------------
	unsigned long GetTotalElements()
	{
		return m_totalElements;
	}

private:
	Element *m_first; // First element in the linked list.
	Element *m_last; // Last element in the linked list.
	Element *m_iterate; // Used for iterating the linked list.
	Element *m_temp; // Used for temporary storage in various operations.

	unsigned long m_totalElements; // Total number of elements in the linked list.
};

#endif