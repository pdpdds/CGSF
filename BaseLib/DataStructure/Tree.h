// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// Tree.h
// This is the Tree class
// ============================================================================
#ifndef TREE_H
#define TREE_H

#include "DLinkedList.h"

namespace CGBase
{

	// forward declarations of all the classes in this file
	template<class DataType> class Tree;
	template<class DataType> class TreeIterator;

	// -------------------------------------------------------
	// Name:        Tree
	// -------------------------------------------------------
	template<class DataType>
	class Tree
	{
	public:

		// typedef the node and iterators to make them easier to use.
		typedef Tree<DataType> Node;
		typedef TreeIterator<DataType> Iterator;

		// ----------------------------------------------------------------
		//  Name:           m_data
		//  Description:    The data in the node
		// ----------------------------------------------------------------
		DataType m_data;

		// ----------------------------------------------------------------
		//  Name:           m_parent
		//  Description:    a pointer to the parent of this node
		// ----------------------------------------------------------------
		Node* m_parent;

		// ----------------------------------------------------------------
		//  Name:           m_children
		//  Description:    a linked list of the child nodes
		// ----------------------------------------------------------------
		DLinkedList<Node*> m_children;


		// ----------------------------------------------------------------
		//  Name:           Tree
		//  Description:    Constructor, creates an empty tree node.
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		Tree()
		{
			m_parent = 0;
		}

		// ----------------------------------------------------------------
		//  Name:           Tree
		//  Description:    creates a tree node with some data
		//  Arguments:      p_data: the data to put in the node
		//  Return Value:   None
		// ----------------------------------------------------------------
		Tree( DataType p_data )
		{
			m_data = p_data;
			m_parent = 0;
		}


		// ----------------------------------------------------------------
		//  Name:           ~Tree
		//  Description:    destructor, recursively destroys the tree
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		~Tree()
		{
			Destroy();
		}


		// ----------------------------------------------------------------
		//  Name:           Destroy
		//  Description:    destroys every child node.
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Destroy()
		{
			DListIterator<Node*> itr = m_children.GetIterator();
			Node* node = 0;

			itr.Start();
			while( itr.Valid() )
			{
				// save the node value.
				node = itr.Item();

				// remove the node from the child list. This will move
				// the iterator forward by 1 node.
				m_children.Remove( itr );

				// delete the child node. Note that this will recursively
				// call the child's Destroy algorithm.
				delete node;
			}
		}


		// ----------------------------------------------------------------
		//  Name:           Count
		//  Description:    recursively counts the number of nodes in the
		//                  tree.
		//  Arguments:      None
		//  Return Value:   the count of the nodes in this tree.
		// ----------------------------------------------------------------
		int Count()
		{
			// set the count to 1, to count this node
			int c = 1;

			DListIterator<Node*> itr = m_children.GetIterator();

			// loop through each child and add that to the current count
			for( itr.Start(); itr.Valid(); itr.Forth() )
			{
				c += itr.Item()->Count();
			}

			return c;
		}

	};



	// ----------------------------------------------------------------
	//  Name:           TreeIterator
	//  Description:    This is an iterator used to iterate through
	//                  a tree
	// ----------------------------------------------------------------
	template<class DataType>
	class TreeIterator
	{
	public:

		// typedef the node to make it easier to work with
		typedef Tree<DataType> Node;

		// ----------------------------------------------------------------
		//  Name:           m_node
		//  Description:    this is the node the iterator points to
		// ----------------------------------------------------------------
		Node* m_node;

		// ----------------------------------------------------------------
		//  Name:           m_childitr
		//  Description:    this is a list iterator, pointing to the current
		//                  child node
		// ----------------------------------------------------------------
		DListIterator<Node*> m_childitr;


		// ----------------------------------------------------------------
		//  Name:           TreeIterator
		//  Description:    Constructor, creates a tree iterator pointing
		//                  to a tree node
		//  Arguments:      p_node: the node the iterator should point to
		//  Return Value:   None
		// ----------------------------------------------------------------
		TreeIterator( Node* p_node = 0 )
		{
			*this = p_node;
		}


		// ----------------------------------------------------------------
		//  Name:           operator=
		//  Description:    assignment operator, assigns a node to the
		//                  iterator.
		//  Arguments:      p_node: a pointer to the node that this iterator
		//                          should point to
		//  Return Value:   None
		// ----------------------------------------------------------------
		void operator= ( Node* p_node )
		{
			m_node = p_node;
			ResetIterator();
		}


		// ----------------------------------------------------------------
		//  Name:           operator==
		//  Description:    determines if two iterators point to the same 
		//                  node.
		//  Arguments:      p_itr: the iterator to compare
		//  Return Value:   true if equal
		// ----------------------------------------------------------------
		bool operator== ( TreeIterator<DataType>& p_itr )
		{
			if( m_node == p_itr.m_node &&
				m_childitr == p_itr.m_childitr )
			{
				return true;
			}
			return false;
		}


		// ----------------------------------------------------------------
		//  Name:           Valid
		//  Description:    determines if the iterator is valid
		//  Arguments:      None
		//  Return Value:   true if valid
		// ----------------------------------------------------------------
		bool Valid()
		{
			return (m_node != 0);
		}


		// ----------------------------------------------------------------
		//  Name:           Item
		//  Description:    gets the item the iterator points to
		//  Arguments:      None
		//  Return Value:   reference to the data
		// ----------------------------------------------------------------
		DataType& Item()
		{
			return m_node->m_data;
		}


		// ----------------------------------------------------------------
		//  Name:           Root
		//  Description:    moves the iterator to the root of the tree.
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Root()
		{
			if( m_node != 0 )
			{
				while( m_node->m_parent != 0 )
				{
					m_node = m_node->m_parent;
				}
			}
			ResetIterator();
		}


		// ----------------------------------------------------------------
		//  Name:           Up
		//  Description:    moves the iterator up by one level of the tree
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Up()
		{
			if( m_node != 0 )
			{
				m_node = m_node->m_parent;
			}
			ResetIterator();
		}


		// ----------------------------------------------------------------
		//  Name:           Down
		//  Description:    moves the iterator down one level, so that it
		//                  points to the current child
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void Down()
		{
			if( m_childitr.Valid() )
			{
				m_node = m_childitr.Item();
				ResetIterator();
			}
		}


		// ----------------------------------------------------------------
		//  Name:           ChildForth
		//  Description:    Moves the child iterator forward
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void ChildForth()
		{
			m_childitr.Forth();
		}

		// ----------------------------------------------------------------
		//  Name:           ChildBack
		//  Description:    Moves the child iterator back
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void ChildBack()
		{
			m_childitr.Back();
		}


		// ----------------------------------------------------------------
		//  Name:           ChildStart
		//  Description:    moves the child iterator to the first child
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void ChildStart()
		{
			m_childitr.Start();
		}


		// ----------------------------------------------------------------
		//  Name:           ChildEnd
		//  Description:    Moves the child iterator to the last child
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void ChildEnd()
		{
			m_childitr.End();
		}

		// ----------------------------------------------------------------
		//  Name:           ChildValid
		//  Description:    determines if the child iterator is valid
		//  Arguments:      None
		//  Return Value:   true if valid.
		// ----------------------------------------------------------------
		bool ChildValid()
		{
			return m_childitr.Valid();
		}


		// ----------------------------------------------------------------
		//  Name:           ChildItem
		//  Description:    returns the item the child pointer points to
		//  Arguments:      None
		//  Return Value:   reference to the item
		// ----------------------------------------------------------------
		DataType& ChildItem()
		{
			return m_childitr.Item()->m_data;
		}


		// ----------------------------------------------------------------
		//  Name:           AppendChild
		//  Description:    appends a child to the child list
		//  Arguments:      p_node: node to append
		//  Return Value:   None
		// ----------------------------------------------------------------
		void AppendChild( Node* p_node )
		{
			if( m_node != 0 )
			{
				m_node->m_children.Append( p_node );
				p_node->m_parent = m_node;
			}
		}

		// ----------------------------------------------------------------
		//  Name:           PrependChild
		//  Description:    prepends a child to the child list
		//  Arguments:      p_node node to prepend
		//  Return Value:   None
		// ----------------------------------------------------------------
		void PrependChild( Node* p_node )
		{
			if( m_node != 0 )
			{
				m_node->m_children.Prepend( p_node );
				p_node->m_parent = m_node;
			}
		}


		// ----------------------------------------------------------------
		//  Name:           InsertChildBefore
		//  Description:    inserts a new node before the current child
		//  Arguments:      p_node: node to insert
		//  Return Value:   None
		// ----------------------------------------------------------------
		void InsertChildBefore( Node* p_node )
		{
			if( m_node != 0 )
			{
				m_node->m_children.InsertBefore( m_childitr, p_node );
				p_node->m_parent = m_node;
			}
		}

		// ----------------------------------------------------------------
		//  Name:           InsertChildAfter
		//  Description:    inserts a new node after the current child
		//  Arguments:      p_node: node to insert
		//  Return Value:   None
		// ----------------------------------------------------------------
		void InsertChildAfter( Node* p_node )
		{
			if( m_node != 0 )
			{
				m_node->m_children.InsertAfter( m_childitr, p_node );
				p_node->m_parent = m_node;
			}
		}

		// ----------------------------------------------------------------
		//  Name:           RemoveChild
		//  Description:    removes the current child from the tree. Doesn't
		//                  delete the node.
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void RemoveChild()
		{
			if( m_node != 0 && m_childitr.Valid() )
			{
				m_childitr.Item()->m_parent = 0;
				m_node->m_children.Remove( m_childitr );
			}
		}


		// ----------------------------------------------------------------
		//  Name:           ResetIterator
		//  Description:    resets the child iterator, so that it points
		//                  to the right linked list
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		void ResetIterator()
		{
			if( m_node != 0 )
			{
				m_childitr = m_node->m_children.GetIterator();
			}
			else
			{
				m_childitr.m_list = 0;
				m_childitr.m_node = 0;
			}
		}
	};



	// ----------------------------------------------------------------
	//  Name:           Preorder
	//  Description:    performs a preorder traversal on a tree
	//  Arguments:      p_node: node to start traversing at
	//                  p_process: function that processes the node
	//  Return Value:   None
	// ----------------------------------------------------------------
	template <class DataType>
	void Preorder( Tree<DataType>* p_node, void (*p_process)(Tree<DataType>*) )
	{
		// process the current node
		p_process( p_node );

		// loop through each child and process it
		DListIterator<Tree<DataType>*> itr = p_node->m_children.GetIterator();
		for( itr.Start(); itr.Valid(); itr.Forth() )
			Preorder( itr.Item(), p_process );
	}


	// ----------------------------------------------------------------
	//  Name:           Postorder
	//  Description:    performs a postorder traversal on a tree
	//  Arguments:      p_node: node to start traversing at
	//                  p_process: function that processes the node
	//  Return Value:   None
	// ----------------------------------------------------------------
	template <class DataType>
	void Postorder( Tree<DataType>* p_node, void (*p_process)(Tree<DataType>*) )
	{
		// loop through each child and process it
		DListIterator<Tree<DataType>*> itr = p_node->m_children.GetIterator();
		for( itr.Start(); itr.Valid(); itr.Forth() )
			Postorder( itr.Item(), p_process );

		// process the current node
		p_process( p_node );
	}
}
#endif
