// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// BinaryTree.h
// This is the Binary Tree class
// ============================================================================
#ifndef BINARYTREE_H
#define BINARYTREE_H

namespace CGBase
{

	// -------------------------------------------------------
	// Name:        BinaryTree
	// -------------------------------------------------------
	template<class DataType>
	class BinaryTree
	{
	public:
		// typedef the tree class into "Node" to make it 
		// easier to work with.
		typedef BinaryTree<DataType> Node;

		// ----------------------------------------------------------------
		//  Name:           m_data
		//  Description:    This is the data in the node
		// ----------------------------------------------------------------
		DataType m_data;

		// ----------------------------------------------------------------
		//  Name:           m_parent
		//  Description:    This is a pointer to the parent node.
		//                  If 0, then it is the parent node.
		// ----------------------------------------------------------------
		Node* m_parent;

		// ----------------------------------------------------------------
		//  Name:           m_left, m_right
		//  Description:    These are the children of the node.
		// ----------------------------------------------------------------
		Node* m_left;
		Node* m_right;


		// ----------------------------------------------------------------
		//  Name:           BinaryTree
		//  Description:    Constructor; creates an empty node
		//  Arguments:      None.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		BinaryTree()
		{
			m_parent = 0;
			m_left = 0;
			m_right = 0;
		}


		// ----------------------------------------------------------------
		//  Name:           BinaryTree
		//  Description:    Data Constructor; creates a node with data
		//  Arguments:      p_data - the data to store in the node
		//  Return Value:   None.
		// ----------------------------------------------------------------
		BinaryTree( DataType p_data )
		{
			m_parent = 0;
			m_left = 0;
			m_right = 0;
			m_data = p_data;
		}

		// ----------------------------------------------------------------
		//  Name:           ~BinaryTree
		//  Description:    Destructor; recursively destroys the tree.
		//  Arguments:      None.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		~BinaryTree()
		{
			Destroy();
		}

		// ----------------------------------------------------------------
		//  Name:           Destroy
		//  Description:    This deletes the left and right children.
		//  Arguments:      None.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		void Destroy()
		{
			// if the left child exists, delete it.
			if( m_left )
				delete m_left;
			m_left = 0;

			// if the right child exists, delete it.
			if( m_right )
				delete m_right;
			m_right = 0;
		}


		// ----------------------------------------------------------------
		//  Name:           Count
		//  Description:    Constructor; creates an empty node
		//  Arguments:      None.
		//  Return Value:   The number of nodes in the tree
		// ----------------------------------------------------------------
		int Count()
		{
			// the count is set to 1, counting this node.
			int c = 1;

			// recursively add the counts of each child.
			if( m_left )
				c += m_left->Count();
			if( m_right )
				c += m_right->Count();

			// return the count.
			return c;
		}

	};



	// ----------------------------------------------------------------
	//  Name:           Preorder
	//  Description:    performs a preorder traversal on a tree
	//  Arguments:      p_node - the node to start processing at.
	//                  p_process - a function pointer to the process
	//                              function
	//  Return Value:   None.
	// ----------------------------------------------------------------
	template <class DataType>
	void Preorder( BinaryTree<DataType>* p_node, 
		void (*p_process)(BinaryTree<DataType>*) )
	{
		// if the node exists
		if( p_node )
		{
			// process the current node
			p_process( p_node );

			// process the left child
			if( p_node->m_left )
				Preorder( p_node->m_left );

			// process the right node
			if( p_node->m_right )
				Preorder( p_node->m_right );
		}
	}


	// ----------------------------------------------------------------
	//  Name:           Inorder
	//  Description:    performs an inorder traversal on a tree
	//  Arguments:      p_node - the node to start processing at.
	//                  p_process - a function pointer to the process
	//                              function
	//  Return Value:   None.
	// ----------------------------------------------------------------
	template <class DataType>
	void Inorder( BinaryTree<DataType>* p_node, 
		void (*p_process)(BinaryTree<DataType>*) )
	{
		// if the node exists
		if( p_node )
		{
			// process the left node
			if( p_node->m_left )
				Preorder( p_node->m_left );

			// process the current node
			p_process( p_node );

			// process the right node
			if( p_node->m_right )
				Preorder( p_node->m_right );
		}
	}


	// ----------------------------------------------------------------
	//  Name:           Postorder
	//  Description:    performs a postorder traversal on a tree
	//  Arguments:      p_node - the node to start processing at.
	//                  p_process - a function pointer to the process
	//                              function
	//  Return Value:   None.
	// ----------------------------------------------------------------
	template <class DataType>
	void Postorder( BinaryTree<DataType>* p_node, 
		void (*p_process)(BinaryTree<DataType>*) )
	{
		// if the node exists
		if( p_node )
		{
			// process the left node
			if( p_node->m_left )
				Preorder( p_node->m_left );

			// process the right node
			if( p_node->m_right )
				Preorder( p_node->m_right );

			// process the current node
			p_process( p_node );
		}
	}



	// ----------------------------------------------------------------
	//  Name:           GetDepth
	//  Description:    Recursively calculates the depth of a tree.
	//  Arguments:      p_node - the root of the tree
	//  Return Value:   The depth; 0 is the lowest.
	// ----------------------------------------------------------------
	template<class DataType>
	int GetDepth( BinaryTree<DataType>* p_tree )
	{
		// start off with -1 for the depth of each child
		int left = -1;
		int right = -1;

		// calculate the depth of the left child
		if( p_tree->m_left != 0 )
			left = GetDepth( p_tree->m_left );

		// calculate the depth of the right child
		if( p_tree->m_right != 0 )
			right = GetDepth( p_tree->m_right );

		// take the larger of the two depths, add one, and return.
		return ( left > right ? left : right ) + 1;
	}
};

#endif
