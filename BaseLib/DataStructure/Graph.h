// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// Graph.h
// This is the file containing the Graph classes
// ============================================================================
#ifndef GRAPH_H
#define GRAPH_H

#include "DLinkedList.h"
#include "Array.h"

namespace CGBase
{

	// forward declare all of the graph classes
	template<class NodeType, class ArcType> class GraphArc;
	template<class NodeType, class ArcType> class GraphNode;
	template<class NodeType, class ArcType> class Graph;



	// -------------------------------------------------------
	// Name:        GraphArc
	// Description: this is the arc class. The arc class
	//              points to a graph node, and contains a 
	//              weight.
	// -------------------------------------------------------
	template<class NodeType, class ArcType>
	class GraphArc
	{
	public:

		// -------------------------------------------------------
		// Name:        m_node
		// Description: pointer to the node that the arc points to
		// -------------------------------------------------------
		GraphNode<NodeType, ArcType>* m_node;

		// -------------------------------------------------------
		// Name:        m_weight
		// Description: Weight of the arc
		// -------------------------------------------------------
		ArcType m_weight;
	};


	// -------------------------------------------------------
	// Name:        GraphNode
	// Description: this is the node class. The node class 
	//              contains data, and has a linked list of 
	//              arcs.
	// -------------------------------------------------------
	template<class NodeType, class ArcType>
	class GraphNode
	{
	public:

		// typedef the classes to make our lives easier.
		typedef GraphArc<NodeType, ArcType> Arc;
		typedef GraphNode<NodeType, ArcType> Node;

		// -------------------------------------------------------
		// Name:        m_data
		// Description: data inside the node
		// -------------------------------------------------------
		NodeType m_data;

		// -------------------------------------------------------
		// Name:        m_arcList
		// Description: list of arcs that the node has.
		// -------------------------------------------------------
		DLinkedList<Arc> m_arcList;

		// -------------------------------------------------------
		// Name:        m_marked
		// Description: This remembers if the node is marked.
		// -------------------------------------------------------
		bool m_marked;


		// ----------------------------------------------------------------
		//  Name:           GetArc
		//  Description:    This finds the arc in the current node that
		//                  points to the node in the parameter.
		//  Arguments:      p_node: the node that the arc connects to.
		//  Return Value:   a pointer to the arc, or 0 if an arc doesn't
		//                  exist from this to p_node.
		// ----------------------------------------------------------------
		Arc* GetArc( Node* p_node )
		{
			DListIterator<Arc> itr = m_arcList.GetIterator();

			// find the arc that matches the node
			for( itr.Start(); itr.Valid(); itr.Forth() )
			{
				if( itr.Item().m_node == p_node )
					return &(itr.Item());
			}

			// not found, return 0.
			return 0;
		}


		// ----------------------------------------------------------------
		//  Name:           AddArc
		//  Description:    This adds an arc from the current node pointing
		//                  to p_node, with p_weight as the weight.
		//  Arguments:      p_node: the node to connect the arc to.
		//                  p_weight: the weight of the arc.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		void AddArc( Node* p_node, ArcType p_weight )
		{
			// create a new arc
			Arc a;
			a.m_node = p_node;
			a.m_weight = p_weight;

			// add it to the arc list
			m_arcList.Append( a );
		}


		// ----------------------------------------------------------------
		//  Name:           RemoveArc
		//  Description:    This finds an arc from this node to p_node and
		//                  removes it.
		//  Arguments:      None.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		void RemoveArc( Node* p_node )
		{
			DListIterator<Arc> itr = m_arcList.GetIterator();

			// find the arc that matches the node
			for( itr.Start(); itr.Valid(); itr.Forth() )
			{
				// if the nodes match, remove the arc, and exit.
				if( itr.Item().m_node == p_node )
				{
					m_arcList.Remove( itr );
					return;
				}
			}
		}
	};


	// ----------------------------------------------------------------
	//  Name:           Graph
	//  Description:    This is the graph class, it contains all the
	//                  nodes.
	// ----------------------------------------------------------------
	template<class NodeType, class ArcType>
	class Graph
	{
	public:
		// typedef the classes to make our lives easier.
		typedef GraphArc<NodeType, ArcType> Arc;
		typedef GraphNode<NodeType, ArcType> Node;

		// ----------------------------------------------------------------
		//  Name:           m_nodes
		//  Description:    An array of all the nodes in the graph.
		// ----------------------------------------------------------------
		Array<Node*> m_nodes;

		// ----------------------------------------------------------------
		//  Name:           m_count
		//  Description:    The number of nodes in the graph.
		// ----------------------------------------------------------------
		int m_count;


		// ----------------------------------------------------------------
		//  Name:           Graph
		//  Description:    Constructor, this constructs an empty graph
		//  Arguments:      p_size: the maximum number of nodes.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		Graph( int p_size ) : m_nodes( p_size )
		{
			int i;
			// go through every index and clear it to 0.
			for( i = 0; i < p_size; i++ )
			{
				m_nodes[i] = 0;
			}

			// set the node count to 0.
			m_count = 0;
		}


		// ----------------------------------------------------------------
		//  Name:           ~Graph
		//  Description:    destructor, This deletes every node
		//  Arguments:      None.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		~Graph()
		{
			int index;
			for( index = 0; index < m_nodes.m_size; index++ )
			{
				if( m_nodes[index] != 0 )
					delete m_nodes[index];
			}
		}


		// ----------------------------------------------------------------
		//  Name:           AddNode
		//  Description:    This adds a node at a given index in the graph.
		//  Arguments:      p_data: the data to store in the node.
		//                  p_index: index to store the node.
		//  Return Value:   true if successful
		// ----------------------------------------------------------------
		bool AddNode( NodeType p_data, int p_index )
		{
			// find out if a node already exists at that index.
			if( m_nodes[p_index] != 0 )
				return false;

			// create a new node, put the data in it, and unmark it.
			m_nodes[p_index] = new Node;
			m_nodes[p_index]->m_data = p_data;
			m_nodes[p_index]->m_marked = false;

			// increase the count and return success.
			m_count++;
			return true;
		}


		// ----------------------------------------------------------------
		//  Name:           RemoveNode
		//  Description:    This removes a node from the graph
		//  Arguments:      p_index: index of the node to return.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		void RemoveNode( int p_index )
		{
			// exit if node doesn't exist.
			if( m_nodes[p_index] == 0 )
				return;

			// now find every arc that points to the node that
			// is being removed and remove it.
			int node;
			Arc* arc;

			// loop through every node
			for( node = 0; node < m_nodes.Size(); node++ )
			{
				// if the node is valid...
				if( m_nodes[node] != 0 )
				{
					// see if the node has an arc pointing to the current node.
					arc = m_nodes[node]->GetArc( m_nodes[p_index] );

					// if it has an arc pointing to the current node, then
					// remove the arc.
					if( arc != 0 )
						RemoveArc( node, p_index );
				}
			}

			// now that every arc pointing to the current node has been removed,
			// the node can be deleted.
			delete m_nodes[p_index];
			m_nodes[p_index] = 0;
			m_count--;
		}


		// ----------------------------------------------------------------
		//  Name:           AddArd
		//  Description:    Adds an arc from p_from to p_to
		//  Arguments:      p_from: originating node index
		//                  p_to: ending node index
		//                  p_wieght: weight of the arc
		//  Return Value:   true on success.
		// ----------------------------------------------------------------
		bool AddArc( int p_from, int p_to, ArcType p_weight )
		{
			// make sure both nodes exist.
			if( m_nodes[p_from] == 0 || m_nodes[p_to] == 0 )
				return false;

			// if an arc already exists, exit
			if( m_nodes[p_from]->GetArc( m_nodes[p_to] ) != 0 )
				return false;

			// add the arc to the "from" node.
			m_nodes[p_from]->AddArc( m_nodes[p_to], p_weight );
			return true;
		}


		// ----------------------------------------------------------------
		//  Name:           RemoveArc
		//  Description:    This removes the arc from p_from to p_to.
		//  Arguments:      p_from: originating node index.
		//                  p_to: ending node index.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		void RemoveArc( int p_from, int p_to )
		{
			// make sure that the node exists before trying to remove
			// an arc from it.
			if( m_nodes[p_from] == 0 || m_nodes[p_to] == 0 )
				return;

			// remove the arc.
			m_nodes[p_from]->RemoveArc( m_nodes[p_to] );
		}


		// ----------------------------------------------------------------
		//  Name:           GetArc
		//  Description:    Gets a pointer to an arc from p_from to p_to
		//  Arguments:      p_from: originating node index.
		//                  p_to: ending node index.
		//  Return Value:   pointer to the arc, or 0 if it doesn't exist.
		// ----------------------------------------------------------------
		Arc* GetArc( int p_from, int p_to )
		{
			// make sure the "from" node exists.
			if( m_nodes[p_from] == 0 || m_nodes[p_to] == 0 )
				return 0;

			return m_nodes[p_from]->GetArc( m_nodes[p_to] );
		}


		// ----------------------------------------------------------------
		//  Name:           ClearMarks
		//  Description:    This clears every mark on every node.
		//  Arguments:      None.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		void ClearMarks()
		{
			int index;
			for( index = 0; index < m_nodes.m_size; index++ )
			{
				if( m_nodes[index] != 0 )
					m_nodes[index]->m_marked = false;
			}
		}


		// ----------------------------------------------------------------
		//  Name:           DepthFirst
		//  Description:    Performs a depth-first traversal on p_node
		//  Arguments:      p_node: the starting node
		//                  p_process: the processing function.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		void DepthFirst( Node* p_node, void (*p_process)(Node*) )
		{
			if( p_node == 0 )
				return;

			// process the current node and mark it
			p_process( p_node );
			p_node->m_marked = true;

			// go through each connecting node
			DListIterator<GraphArc<Coordinates, int> > itr = p_node->m_arcList.GetIterator();
			for( itr.Start(); itr.Valid(); itr.Forth() )
			{
				// process the linked node if it isn't already marked.
				if( itr.Item().m_node->m_marked == false )
				{
					DepthFirst( itr.Item().m_node, p_process );
				}
			}
		}


		// ----------------------------------------------------------------
		//  Name:           BreadthFirst
		//  Description:    Performs a depth-first traversal on p_node
		//  Arguments:      p_node: the starting node
		//                  p_process: the processing function.
		//  Return Value:   None.
		// ----------------------------------------------------------------
		void BreadthFirst( Node* p_node, void (*p_process)(Node*) )
		{
			if( p_node == 0 )
				return;

			LQueue<Node*> queue;
			DListIterator<Arc> itr;

			// place the first node on the queue, and mark it.
			queue.Enqueue( p_node );
			p_node->m_marked = true;

			// loop through the queue while there are nodes in it.
			while( queue.Count() != 0 )
			{
				// process the node at the front of the queue.
				p_process( queue.Front() );

				// add all of the child nodes that have not been 
				// marked into the queue
				itr = queue.Front()->m_arcList.GetIterator();
				for( itr.Start(); itr.Valid(); itr.Forth() )
				{
					if( itr.Item().m_node->m_marked == false )
					{
						// mark the node and add it to the queue.
						itr.Item().m_node->m_marked = true;
						queue.Enqueue( itr.Item().m_node );
					}
				}

				// dequeue the current node.
				queue.Dequeue();
			}
		}

	};
};
#endif
