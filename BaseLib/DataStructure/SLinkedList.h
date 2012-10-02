// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// SLinkedList.h
// This is The basic Singly-Linked List class
// ============================================================================
#ifndef SLINKEDLIST_H
#define SLINKEDLIST_H

#include <stdio.h>

namespace CGBase
{


	// forward declarations of all the classes in this file
	template<class Datatype> class SListNode;
	template<class Datatype> class SLinkedList;
	template<class Datatype> class SListIterator;



	// -------------------------------------------------------
	// Name:        SListNode
	// Description: This is the basic Singly-linked list node
	//              class.
	// -------------------------------------------------------
	template<class Datatype>
	class SListNode
	{
	public:


		// ----------------------------------------------------------------
		//  Name:           m_data
		//  Description:    This is the data stored in each node
		// ----------------------------------------------------------------
		Datatype m_data;


		// ----------------------------------------------------------------
		//  Name:           m_next
		//  Description:    a pointer to the next node in the list
		// ----------------------------------------------------------------
		SListNode<Datatype>* m_next;


		// ----------------------------------------------------------------
		//  Name:           SListNode
		//  Description:    Constructor, creates an empty node.
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		SListNode()
		{
			m_next = 0;
		}

		// ----------------------------------------------------------------
		//  Name:           InsertAfter
		//  Description:    this inserts a new node after the current node
		//  Arguments:      p_data: the data to insert
		//  Return Value:   None
		// ----------------------------------------------------------------
		void InsertAfter( Datatype p_data )
		{
			// create the new node.
			SListNode<Datatype>* newnode = new SListNode<Datatype>;
			newnode->m_data = p_data;

			// make the new node point to the next node.
			newnode->m_next = m_next;

			// make the previous node point to the new node
			m_next = newnode;
		}

	};



	// -------------------------------------------------------
	// Name:        SLinkedList
	// Description: This is the Singly-linked list container.
	// -------------------------------------------------------
	template<class Datatype>
	class SLinkedList
	{
	public:

		// ----------------------------------------------------------------
		//  Name:           SLinkedList
		//  Description:    Constructor, creates an empty list.
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		SLinkedList()
		{
			m_head = 0;
			m_tail = 0;
			m_count = 0;
		}


		// ----------------------------------------------------------------
		//  Name:           ~SLinkedList
		//  Description:    destructor, deletes every node.
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		~SLinkedList()
		{
			// temporary node pointers.
			SListNode<Datatype>* itr = m_head;
			SListNode<Datatype>* next;

			while( itr != 0 )
			{
				// save the pointer to the next node.
				next = itr->m_next;

				// delete the current node.
				delete itr;

				// make the next node the current node.
				itr = next;
			}
		}


		// ----------------------------------------------------------------
		//  Name:           Append
		//  Description:    adds data to the end of the list
		//  Arguments:      p_data: the data to insert
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Append( Datatype p_data )
		{
			// check to make sure the list isn't empty
			if( m_head == 0 )
			{
				// if the list is empty,
				// create a new head node.
				m_head = m_tail = new SListNode<Datatype>;
				m_head->m_data = p_data;
			}
			else
			{
				// insert a new node after the tail, and reset the tail.
				m_tail->InsertAfter( p_data );
				m_tail = m_tail->m_next;
			}
			m_count++;
		}


		// ----------------------------------------------------------------
		//  Name:           Prepend
		//  Description:    adds data to the beginning of the list
		//  Arguments:      p_data: data to insert
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Prepend( Datatype p_data )
		{
			// create the new node.
			SListNode<Datatype>* newnode = new SListNode<Datatype>;
			newnode->m_data = p_data;
			newnode->m_next = m_head;

			// set the head node, and the tail node if needed.
			m_head = newnode;
			if( m_tail == 0 )
				m_tail = m_head;

			m_count++;
		}




		// ----------------------------------------------------------------
		//  Name:           Insert
		//  Description:    inserts new data after the given iterator, or
		//                  appends it if iterator is invalid.
		//  Arguments:      p_iterator: iterator to insert after
		//                  p_data: data to insert
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Insert( SListIterator<Datatype>& p_iterator, Datatype p_data )
		{
			// if the iterator doesn't belong to this list, do nothing.
			if( p_iterator.m_list != this )
				return;

			if( p_iterator.m_node != 0 )
			{
				// if the iterator is valid, then insert the node
				p_iterator.m_node->InsertAfter( p_data );

				// if the iterator is the tail node, then
				// update the tail pointer to point to the
				// new node.
				if( p_iterator.m_node == m_tail )
				{
					m_tail = p_iterator.m_node->m_next;
				}
				m_count++;
			}
			else
			{
				// if the iterator is invalid, just append the data
				Append( p_data );
			}
		}


		// ----------------------------------------------------------------
		//  Name:           Remove
		//  Description:    removes the node that the iterator points to
		//  Arguments:      p_iterator: points to the node to remove
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Remove( SListIterator<Datatype>& p_iterator )
		{
			SListNode<Datatype>* node = m_head;

			// if the iterator doesn't belong to this list, do nothing.
			if( p_iterator.m_list != this )
				return;

			// if node is invalid, do nothing.
			if( p_iterator.m_node == 0 )
				return;

			if( p_iterator.m_node == m_head )
			{
				// move the iterator forward, and delete the head.
				p_iterator.Forth();
				RemoveHead();
			}
			else
			{
				// scan forward through the list until we find
				// the node prior to the node we want to remove
				while( node->m_next != p_iterator.m_node )
					node = node->m_next;

				// move the iterator forward.
				p_iterator.Forth();

				// if the node we are deleting is the tail, 
				// update the tail node.
				if( node->m_next == m_tail )
				{
					m_tail = node;
				}

				// delete the node.
				delete node->m_next;

				// re-link the list.
				node->m_next = p_iterator.m_node;
			}
			m_count--;
		}


		// ----------------------------------------------------------------
		//  Name:           RemoveHead
		//  Description:    removes the head of the list
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void RemoveHead()
		{
			SListNode<Datatype>* node = 0;

			if( m_head != 0 )
			{
				// make node point to the next node.
				node = m_head->m_next;

				// then delete the head, and make the pointer
				// point to node.
				delete m_head;
				m_head = node;

				// if the head is null, then we've just deleted the only node
				// in the list. set the tail to 0.
				if( m_head == 0 )
					m_tail = 0;

				m_count--;
			}
		}


		// ----------------------------------------------------------------
		//  Name:           RemoveTail
		//  Description:    Removes the tail of the list
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void RemoveTail()
		{
			SListNode<Datatype>* node = m_head;

			// if the list isn't empty, then remove a node.
			if( m_head != 0 )
			{
				// if the head is equal to the tail, then 
				// the list has 1 node, and we are removing it.
				if( m_head == m_tail )
				{
					// delete the node, and set both pointers
					// to 0.
					delete m_head;
					m_head = m_tail = 0;
				}
				else
				{
					// skip ahead until we find the node
					// right before the tail node
					while( node->m_next != m_tail )
						node = node->m_next;

					// make the tail point to the node before the
					// current tail, and delete the old tail.
					m_tail = node;
					delete node->m_next;
					node->m_next = 0;
				}
				m_count--;
			}
		}


		// ----------------------------------------------------------------
		//  Name:           GetIterator
		//  Description:    gets an iterator pointing to the beginning of
		//                  the list
		//  Arguments:      None
		//  Return Value:   an iterator pointing to the beginning of the
		//                  list.
		// ----------------------------------------------------------------
		SListIterator<Datatype> GetIterator()
		{
			return SListIterator<Datatype>( this, m_head );
		}


		// ----------------------------------------------------------------
		//  Name:           Size
		//  Description:    gets the size of the list
		//  Arguments:      None
		//  Return Value:   size of the list
		// ----------------------------------------------------------------
		int Size()
		{
			return m_count;
		}


		// ----------------------------------------------------------------
		//  Name:           SaveToDisk
		//  Description:    saves the list to disk
		//  Arguments:      p_filename: name of the file to save to
		//  Return Value:   true if successful
		// ----------------------------------------------------------------
		bool SaveToDisk( char* p_filename )
		{
			FILE* outfile = 0;
			SListNode<Datatype>* itr = m_head;

			// open the file
			outfile = fopen( p_filename, "wb" );

			// return if it couldn't be opened
			if( outfile == 0 )
				return false;

			// write the size of the list first
			fwrite( &m_count, sizeof( int ), 1, outfile );

			// now loop through and write the list.
			while( itr != 0 )
			{
				fwrite( &(itr->m_data), sizeof( Datatype ), 1, outfile );
				itr = itr->m_next;
			}

			fclose( outfile );

			// return success.
			return true;
		}


		// ----------------------------------------------------------------
		//  Name:           ReadFromDisk
		//  Description:    reads a list from disk
		//  Arguments:      p_filename: name of the file
		//  Return Value:   true if successful
		// ----------------------------------------------------------------
		bool ReadFromDisk( char* p_filename )
		{
			FILE* infile = 0;
			Datatype buffer;
			int count = 0;

			// open the file
			infile = fopen( p_filename, "rb" );

			// return if it couldn't be opened
			if( infile == 0 )
				return false;

			// read the size of the list first
			fread( &count, sizeof( int ), 1, infile );

			// now loop through and read the list.
			while( count != 0 )
			{
				fread( &buffer, sizeof( Datatype ), 1, infile );
				Append( buffer );
				count--;
			}

			fclose( infile );

			// return success.
			return true;
		}


		SListNode<Datatype>* m_head;
		SListNode<Datatype>* m_tail;
		int m_count;
	};




	// -------------------------------------------------------
	// Name:        SListIterator
	// Description: This is the basic Singly-linked list 
	//              iterator class.
	// -------------------------------------------------------
	template<class Datatype>
	class SListIterator
	{
	public:


		// ----------------------------------------------------------------
		//  Name:           SListIterator
		//  Description:    Constructor, creates an iterator with a given
		//                  list and node.
		//  Arguments:      p_list: the list the iterator points to
		//                  p_node: the node the iterator points to
		//  Return Value:   None
		// ----------------------------------------------------------------
		SListIterator( SLinkedList<Datatype>* p_list = 0,
			SListNode<Datatype>* p_node = 0 )
		{
			m_list = p_list;
			m_node = p_node;
		}


		// ----------------------------------------------------------------
		//  Name:           Start
		//  Description:    moves the iterator to the start of the list.
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Start()
		{
			if( m_list != 0 )
				m_node = m_list->m_head;
		}


		// ----------------------------------------------------------------
		//  Name:           Forth
		//  Description:    Moves the iterator forward
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Forth()
		{
			if( m_node != 0 )
				m_node = m_node->m_next;
		}


		// ----------------------------------------------------------------
		//  Name:           Item
		//  Description:    gets the item the iterator points to
		//  Arguments:      None
		//  Return Value:   a reference to the item.
		// ----------------------------------------------------------------
		Datatype& Item()
		{
			return m_node->m_data;
		}


		// ----------------------------------------------------------------
		//  Name:           Valid
		//  Description:    determines if the iterator is valid or not
		//  Arguments:      None
		//  Return Value:   true if valid
		// ----------------------------------------------------------------
		bool Valid()
		{
			return (m_node != 0);
		}


		// ----------------------------------------------------------------
		//  Name:           m_node
		//  Description:    the current node
		// ----------------------------------------------------------------
		SListNode<Datatype>* m_node;

		// ----------------------------------------------------------------
		//  Name:           m_list
		//  Description:    the current list
		// ----------------------------------------------------------------
		SLinkedList<Datatype>* m_list;
	};

};
#endif
