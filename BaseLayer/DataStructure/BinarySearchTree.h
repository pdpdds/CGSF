// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// BinarySearchTree.h
// This is the Binary Search Tree class
// ============================================================================
#ifndef BINARYSEARCHTREE_H
#define BINARYSEARCHTREE_H

#include "BinaryTree.h"

namespace CGBase
{

	template <class DataType>
	class BinarySearchTree
	{
	public:
		typedef BinaryTree<DataType> Node;


		// ----------------------------------------------------------------
		//  Name:           BinarySearchTree::BinarySearchTree
		//  Description:    Default Constructor. it creates an empty BST
		//  Arguments:      - compare: A function which compares two
		//                             DataTypes and returns -1 if the left
		//                             is "less than" the right, 0 if they
		//                             are equal, and 1 if the left is 
		//                             "greater than" the right.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		BinarySearchTree( int (*p_compare)(DataType, DataType) )
		{
			m_root = 0;
			m_compare = p_compare;
		};


		// ----------------------------------------------------------------
		//  Name:           BinarySearchTree::~BinarySearchTree
		//  Description:    Destructor.
		//  Arguments:      None.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		~BinarySearchTree()
		{
			// if the root exists, recursively delete the tree.
			if( m_root != 0 )
				delete m_root;
		}


		// ----------------------------------------------------------------
		//  Name:           BinarySearchTree::Insert
		//  Description:    Inserts data into the BST
		//  Arguments:      - p_data: data to insert
		//  Return Value:   None.
		// ----------------------------------------------------------------
		void Insert( DataType p_data )
		{
			Node* current = m_root;

			// if there is no root, then just put the new data
			// at the root.
			if( m_root == 0 )
				m_root = new Node( p_data );
			else
			{
				// else, you need to find where to insert the node.
				while( current != 0 )
				{
					// compare the data to insert with the current node
					if( m_compare( p_data, current->m_data ) < 0 )
					{
						// test to see if the left child exists
						if( current->m_left == 0 )
						{
							// if it doesnt exist, then insert the node
							// as the left child.
							current->m_left = new Node( p_data );
							current->m_left->m_parent = current;
							current = 0;
						}
						else
						{
							// else, go down to the left.
							current = current->m_left;
						}
					}
					else
					{
						// test to see if the right child exists
						if( current->m_right == 0 )
						{
							// if it doesn't exist, then insert the node
							// as the right child.
							current->m_right = new Node( p_data );
							current->m_right->m_parent = current;
							current = 0;
						}
						else
						{
							// else, go down to the right
							current = current->m_right;
						}
					}
				}
			}
		}



		// ----------------------------------------------------------------
		//  Name:           BinarySearchTree::Find
		//  Description:    finds a piece of data in the tree and returns
		//                  a pointer to the node which contains a match,
		//                  or 0 if no match is found.
		//  Arguments:      - p_data: data to find
		//  Return Value:   pointer to node which contains data, or 0.
		// ----------------------------------------------------------------
		Node* Find( DataType p_data )
		{
			Node* current = m_root;
			int temp;

			while( current != 0 )
			{
				// compare the data with the current nodes data.
				temp = m_compare( p_data, current->m_data );

				// if the data is equal, the node was found,
				// therefore, return it.
				if( temp == 0 )
					return current;

				// if the node was less than the current node, then
				// go to the left, else go to the right.
				if( temp < 0 )
					current = current->m_left;
				else
					current = current->m_right;
			}
			return 0;
		}


		// ----------------------------------------------------------------
		//  Name:           BinarySearchTree::count
		//  Description:    returns the count of items within the tree
		//  Arguments:      None.
		//  Return Value:   the number of items.
		// ----------------------------------------------------------------
		inline int Count()
		{
			// recursively count the nodes starting at the root.
			if( m_root != 0 )
				return m_root->Count();
			return 0;
		}   


		// ----------------------------------------------------------------
		//  Name:           BinarySearchTree::m_root
		//  Description:    root node of the tree.
		// ----------------------------------------------------------------
		Node* m_root;

		// ----------------------------------------------------------------
		//  Name:           BinarySearchTree::m_compare
		//  Description:    comparison function
		// ----------------------------------------------------------------
		int (*m_compare)(DataType, DataType);
	}; 
};
#endif
