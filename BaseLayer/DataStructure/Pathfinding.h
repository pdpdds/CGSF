// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// Pathfinding.h
// This is file that contains pathfinding algorithms
// ============================================================================
#ifndef PATHFINDING_H
#define PATHFINDING_H


#include "Array2D.h"
#include "Queue.h"
#include "Heap.h"

namespace CGBase
{


	// ----------------------------------------------------------------
	//  Name:           QUEUESIZE
	//  Description:    This is the size of the queues in the 
	//                  pathfinders. If you ever need more than this,
	//                  just increase the value.
	// ----------------------------------------------------------------
	const int QUEUESIZE = 1024;

	// ----------------------------------------------------------------
	//  Name:           DIRTABLE
	//  Description:    This is the direction table. It stores the
	//                  relative coordinates of each direction. There
	//                  are 8 directions and 2 coordinates per 
	//                  direction (x, y). They are stored in this 
	//                  order: N, E, S, W, NE, SE, SW, NW.
	//                  To move N, for example, move 0 places in the x
	//                  axis, and -1 places in the y axis.
	// ----------------------------------------------------------------
	const int DIRTABLE[8][2] = { {  0, -1 },    // N
	{  1,  0 },    // E
	{  0,  1 },    // S
	{ -1,  0 },    // W
	{  1, -1 },    // NE
	{  1,  1 },    // SE
	{ -1,  1 },    // SW
	{ -1, -1 } };  // NW

	// ----------------------------------------------------------------
	//  Name:           DISTTABLE
	//  Description:    A constant array full of the distances that
	//                  you travel when going in one of the 8 directions
	// ----------------------------------------------------------------
	const float DISTTABLE[8] = { 1.0f,      1.0f,      1.0f,      1.0f,
		1.414214f, 1.414214f, 1.414214f, 1.414214f };


	// ============================================================================
	//  Classes
	// ============================================================================



	// ----------------------------------------------------------------
	//  Name:           Cell
	//  Description:    This contains information about a single
	//                  cell in a map.
	// ----------------------------------------------------------------
	class Cell
	{
	public:

		// ----------------------------------------------------------------
		//  Name:           m_marked
		//  Description:    tells whether the cell is marked or not.
		// ----------------------------------------------------------------
		bool m_marked;

		// ----------------------------------------------------------------
		//  Name:           m_distance
		//  Description:    The actual distance from this cell to the
		//                  starting cell
		// ----------------------------------------------------------------
		float m_distance;

		// ----------------------------------------------------------------
		//  Name:           m_lastx, m_lasty
		//  Description:    the coordinates of the previous cell in the
		//                  path.
		// ----------------------------------------------------------------
		int m_lastx;
		int m_lasty;

		// ----------------------------------------------------------------
		//  Name:           m_passable
		//  Description:    tells if the pathfinder can go through this cell
		//  Arguments:      None
		//  Return Value:   None
		// ----------------------------------------------------------------
		bool m_passable;

		// ----------------------------------------------------------------
		//  Name:           m_weight
		//  Description:    describes how much work it takes to enter the
		//                  cell.
		// ----------------------------------------------------------------
		float m_weight;

	};



	// ----------------------------------------------------------------
	//  Name:           Coordinate
	//  Description:    a simple coordinate class, with a heuristic.
	// ----------------------------------------------------------------
	class Coordinate
	{
	public:
		int x;
		int y;
		float heuristic;
	};



	// ============================================================================
	//  Helper Functions
	// ============================================================================


	// ----------------------------------------------------------------
	//  Name:           CompareCoordinatesAscending
	//  Description:    Comparison function, higher heuristics are
	//                  "higher".
	// ----------------------------------------------------------------
	int CompareCoordinatesAscending( Coordinate left, Coordinate right )
	{
		if( left.heuristic > right.heuristic )
			return 1;
		if( left.heuristic < right.heuristic )
			return -1;
		return 0;
	}


	// ----------------------------------------------------------------
	//  Name:           CompareCoordinatesDescending
	//  Description:    Conparison function, higher heuristics are
	//                  "lower"
	// ----------------------------------------------------------------
	int CompareCoordinatesDescending( Coordinate left, Coordinate right )
	{
		if( left.heuristic < right.heuristic )
			return 1;
		if( left.heuristic > right.heuristic )
			return -1;
		return 0;
	}


	// ----------------------------------------------------------------
	//  Name:           ClearCells
	//  Description:    Clears every cell in a 2D map of Cell's
	//  Arguments:      p_map: the tilemap.
	//  Return Value:   None
	// ----------------------------------------------------------------
	void ClearCells( Array2D<Cell>& p_map )
	{
		int x;
		int y;

		// for each cell
		for( y = 0; y < p_map.Height(); y++ )
		{
			for( x = 0; x < p_map.Width(); x++ )
			{
				// unmark it, set distance to 0, 
				// and clear the last coordinates.
				p_map.Get( x, y ).m_marked = false;
				p_map.Get( x, y ).m_distance = 0.0f;
				p_map.Get( x, y ).m_lastx = -1;
				p_map.Get( x, y ).m_lasty = -1;
			}
		}
	}


	// ----------------------------------------------------------------
	//  Name:           CellDistance
	//  Description:    Finds the distance between two cells.
	//  Arguments:      x1, y1, x2, y2: coordinates
	//  Return Value:   The distance.
	// ----------------------------------------------------------------
	float CellDistance( int x1, int y1, int x2, int y2 )
	{
		// find the difference between the x and y coords.
		int dx = x1 - x2;
		int dy = y1 - y2;

		// use pythagorean's theorem: dist = sqrt( dx^2 + dy^2 )

		// square dx and dy
		dx = dx * dx;
		dy = dy * dy;

		// get the square root (sqrt works with doubles)
		return (float)sqrt( (double)dx + (double)dy );
	}


	// ----------------------------------------------------------------
	//  Name:           PathDistanceFirst
	//  Description:    Performs the distance-first pathfinding
	//                  algorithm on a tilemap.
	//  Arguments:      p_map: the map
	//                  p_x, p_y: starting coordinates
	//                  p_gx, p_gy: goal coordinates
	//  Return Value:   None
	// ----------------------------------------------------------------
	void PathDistanceFirst( Array2D<Cell>& p_map, 
		int p_x, int p_y, 
		int p_gx, int p_gy )
	{
		Coordinate c;
		int x, y;
		int ax, ay;
		int dir;
		float distance;

		static Heap<Coordinate> queue( QUEUESIZE, CompareCoordinatesDescending );
		queue.m_count = 0;

		// clear the cells first.
		ClearCells( p_map );

		// enqueue the starting cell in the queue
		c.x = p_x;
		c.y = p_y;
		c.heuristic = 0.0f;
		queue.Enqueue( c );


		// start the main loop
		while( queue.m_count != 0 )
		{
			// pull the first cell off the queue and process it.
			x = queue.Item().x;
			y = queue.Item().y;
			queue.Dequeue();

			// make sure the node isn't already marked. If it is, do
			// nothing.
			if( p_map.Get( x, y ).m_marked == false )
			{
				// mark the cell as it is pulled off the queue.
				p_map.Get( x, y ).m_marked = true;

				// quit out if the goal has been reached.
				if( x == p_gx && y == p_gy )
					break;

				// loop through each direction.
				for( dir = 0; dir < 8; dir++ )
				{
					// retrieve the coordinates of the current adjacent cell
					ax = x + DIRTABLE[dir][0];
					ay = y + DIRTABLE[dir][1];

					// check to see if the adjacent cell is a valid index, 
					// passable, and not marked.
					if( ax >= 0 && ax < p_map.Width() && 
						ay >= 0 && ay < p_map.Height() &&
						p_map.Get( ax, ay ).m_passable == true &&
						p_map.Get( ax, ay ).m_marked == false )
					{
						// calculate the distance to get into this cell.
						// this is calulated as:
						// distance of the current cell plus
						// the weight of the adjacent cell times the distance
						// of the cell.
						// diagonal cell's cost is around 1.4 times the cost of
						// a horizontal or vertical cell.
						distance = p_map.Get( x, y ).m_distance + 
							p_map.Get( ax, ay ).m_weight * DISTTABLE[dir]; 

						// check if the node has already been calculated before
						if( p_map.Get( ax, ay ).m_lastx != -1 )
						{
							// the node has already been calculated, see if the
							// new distance is shorter. If so, update the links.
							if( distance < p_map.Get( ax, ay ).m_distance )
							{
								// the new distance is shorter, update the links
								p_map.Get( ax, ay ).m_lastx = x;
								p_map.Get( ax, ay ).m_lasty = y;
								p_map.Get( ax, ay ).m_distance = distance;

								// add the cell to the queue.
								c.x = ax;
								c.y = ay;
								c.heuristic = distance;
								queue.Enqueue( c );
							}
						}
						else
						{
							// set the links and the distance
							p_map.Get( ax, ay ).m_lastx = x;
							p_map.Get( ax, ay ).m_lasty = y;
							p_map.Get( ax, ay ).m_distance = distance;

							// add the cell to the queue.
							c.x = ax;
							c.y = ay;
							c.heuristic = distance;
							queue.Enqueue( c );
						}
					}
				}
			}
		}
	}





	// ----------------------------------------------------------------
	//  Name:           SimpleHeuristic
	//  Description:    A simple heuristic function, used for the
	//                  PathSimpleHeuristic pathfinder.
	//  Arguments:      x, y: the current x and y coordinates
	//                  gx, gy: goal coordinates
	//                  dir: the direction the pathfinder is thinking
	//                       about moving in.
	//  Return Value:   the heuristic value
	// ----------------------------------------------------------------
	float SimpleHeuristic( int x, int y, int gx, int gy, int dir )
	{
		// start the heuristic at 0.
		float h = 0.0f;

		int diff1;
		int diff2;

		// first compare the horizontal

		// find the  difference from the goal to the current cell
		diff1 = gx - x;

		// find the difference from the goal to the cell the pathfinder
		// is analysing
		diff2 = gx - (x + DIRTABLE[dir][0]);

		// find the absolute value of the distances
		if( diff1 < 0 )
			diff1 = -diff1;
		if( diff2 < 0 )
			diff2 = -diff2;

		// if the current cell is further away, subtract 1 from the
		// heuristic; else if it is closer, add one.
		if( diff1 > diff2 )
			h -= 1.0f;
		else if( diff1 < diff2 )
			h += 1.0f;

		// now do the vertical.

		// find the  difference from the goal to the current cell
		diff1 = gy - y;

		// find the difference from the goal to the cell the pathfinder
		// is analysing
		diff2 = gy - (y + DIRTABLE[dir][1]);

		// find the absolute value of the distances
		if( diff1 < 0 )
			diff1 = -diff1;
		if( diff2 < 0 )
			diff2 = -diff2;

		// if the current cell is further away, subtract 1 from the
		// heuristic; else if it is closer, add one.
		if( diff1 > diff2 )
			h -= 1.0f;
		else if( diff1 < diff2 )
			h += 1.0f;

		// return the heuristic.
		return h;
	}



	// ----------------------------------------------------------------
	//  Name:           PathSimpleHeuristic
	//  Description:    pathfinder that uses a simple heuristic
	//  Arguments:      p_map: the map
	//                  p_x, p_y: starting coordinates
	//                  p_gx, p_gy: goal coordinates
	//  Return Value:   None
	// ----------------------------------------------------------------
	void PathSimpleHeuristic( Array2D<Cell>& p_map, 
		int p_x, int p_y, 
		int p_gx, int p_gy )
	{
		Coordinate c;
		int x, y;
		int ax, ay;
		int dir;
		float distance;

		static Heap<Coordinate> queue( QUEUESIZE, CompareCoordinatesDescending );
		queue.m_count = 0;

		// clear the cells first.
		ClearCells( p_map );

		// enqueue the starting cell in the queue
		c.x = p_x;
		c.y = p_y;
		queue.Enqueue( c );


		// start the main loop
		while( queue.m_count != 0 )
		{
			// pull the first cell off the queue and process it.
			x = queue.Item().x;
			y = queue.Item().y;
			queue.Dequeue();

			// make sure the node isn't already marked. If it is, do
			// nothing.
			if( p_map.Get( x, y ).m_marked == false )
			{
				// mark the cell as it is pulled off the queue.
				p_map.Get( x, y ).m_marked = true;

				// quit out if the goal has been reached.
				if( x == p_gx && y == p_gy )
					break;

				// loop through each direction.
				for( dir = 0; dir < 8; dir++ )
				{
					// retrieve the coordinates of the current adjacent cell
					ax = x + DIRTABLE[dir][0];
					ay = y + DIRTABLE[dir][1];

					// check to see if the adjacent cell is a valid index, 
					// passable, and not marked.
					if( ax >= 0 && ax < p_map.Width() && 
						ay >= 0 && ay < p_map.Height() &&
						p_map.Get( ax, ay ).m_passable == true &&
						p_map.Get( ax, ay ).m_marked == false )
					{
						// calculate the distance to get into this cell.
						// this is calulated as:
						// distance of the current cell plus
						// the weight of the adjacent cell times the distance
						// of the cell.
						// diagonal cell's cost is around 1.4 times the cost of
						// a horizontal or vertical cell.
						distance = p_map.Get( x, y ).m_distance + 
							p_map.Get( ax, ay ).m_weight * DISTTABLE[dir]; 

						// check if the node has already been calculated before
						if( p_map.Get( ax, ay ).m_lastx != -1 )
						{
							// the node has already been calculated, see if the
							// new distance is shorter. If so, update the links.
							if( distance < p_map.Get( ax, ay ).m_distance )
							{
								// the new distance is shorter, update the links
								p_map.Get( ax, ay ).m_lastx = x;
								p_map.Get( ax, ay ).m_lasty = y;
								p_map.Get( ax, ay ).m_distance = distance;

								// add the cell to the queue.
								c.x = ax;
								c.y = ay;
								c.heuristic = SimpleHeuristic( x, y, 
									p_gx, p_gy, 
									dir );
								queue.Enqueue( c );
							}
						}
						else
						{
							// set the links and the distance
							p_map.Get( ax, ay ).m_lastx = x;
							p_map.Get( ax, ay ).m_lasty = y;
							p_map.Get( ax, ay ).m_distance = distance;

							// add the cell to the queue.
							c.x = ax;
							c.y = ay;
							c.heuristic = SimpleHeuristic( x, y, 
								p_gx, p_gy,
								dir );
							queue.Enqueue( c );
						}
					}
				}
			}
		}
	}




	// ----------------------------------------------------------------
	//  Name:           ComplexHeuristic
	//  Description:    calculates a complex heuristic
	//  Arguments:      x, y: coordinates of the current cell
	//                  gx, gy: goal coordinates
	//                  dir: direction of the target cell
	//  Return Value:   the heuristic
	// ----------------------------------------------------------------
	float ComplexHeuristic( int x, int y, int gx, int gy, int dir )
	{
		// calculate the coordinates of the target cell
		x = x + DIRTABLE[dir][0];
		y = y + DIRTABLE[dir][1];

		// find the distance from the target to the goal
		return CellDistance( x, y, gx, gy );

	}


	// ----------------------------------------------------------------
	//  Name:           PathComplexHeuristic
	//  Description:    pathfinder that uses a complex heuristic
	//  Arguments:      p_map: the map
	//                  p_x, p_y: starting coordinates
	//                  p_gx, p_gy: goal coordinates
	//  Return Value:   None
	// ----------------------------------------------------------------
	void PathComplexHeuristic( Array2D<Cell>& p_map, 
		int p_x, int p_y, 
		int p_gx, int p_gy )
	{
		Coordinate c;
		int x, y;
		int ax, ay;
		int dir;
		float distance;

		static Heap<Coordinate> queue( QUEUESIZE, CompareCoordinatesDescending );
		queue.m_count = 0;

		// clear the cells first.
		ClearCells( p_map );

		// enqueue the starting cell in the queue
		c.x = p_x;
		c.y = p_y;
		queue.Enqueue( c );


		// start the main loop
		while( queue.m_count != 0 )
		{
			// pull the first cell off the queue and process it.
			x = queue.Item().x;
			y = queue.Item().y;
			queue.Dequeue();

			// make sure the node isn't already marked. If it is, do
			// nothing.
			if( p_map.Get( x, y ).m_marked == false )
			{
				// mark the cell as it is pulled off the queue.
				p_map.Get( x, y ).m_marked = true;

				// quit out if the goal has been reached.
				if( x == p_gx && y == p_gy )
					break;

				// loop through each direction.
				for( dir = 0; dir < 8; dir++ )
				{
					// retrieve the coordinates of the current adjacent cell
					ax = x + DIRTABLE[dir][0];
					ay = y + DIRTABLE[dir][1];

					// check to see if the adjacent cell is a valid index, 
					// passable, and not marked.
					if( ax >= 0 && ax < p_map.Width() && 
						ay >= 0 && ay < p_map.Height() &&
						p_map.Get( ax, ay ).m_passable == true &&
						p_map.Get( ax, ay ).m_marked == false )
					{
						// calculate the distance to get into this cell.
						// this is calulated as:
						// distance of the current cell plus
						// the weight of the adjacent cell times the distance
						// of the cell.
						// diagonal cell's cost is around 1.4 times the cost of
						// a horizontal or vertical cell.
						distance = p_map.Get( x, y ).m_distance + 
							p_map.Get( ax, ay ).m_weight * DISTTABLE[dir]; 

						// check if the node has already been calculated before
						if( p_map.Get( ax, ay ).m_lastx != -1 )
						{
							// the node has already been calculated, see if the
							// new distance is shorter. If so, update the links.
							if( distance < p_map.Get( ax, ay ).m_distance )
							{
								// the new distance is shorter, update the links
								p_map.Get( ax, ay ).m_lastx = x;
								p_map.Get( ax, ay ).m_lasty = y;
								p_map.Get( ax, ay ).m_distance = distance;

								// add the cell to the queue.
								c.x = ax;
								c.y = ay;
								c.heuristic = ComplexHeuristic( x, y, 
									p_gx, p_gy, 
									dir );
								queue.Enqueue( c );
							}
						}
						else
						{
							// set the links and the distance
							p_map.Get( ax, ay ).m_lastx = x;
							p_map.Get( ax, ay ).m_lasty = y;
							p_map.Get( ax, ay ).m_distance = distance;

							// add the cell to the queue.
							c.x = ax;
							c.y = ay;
							c.heuristic = ComplexHeuristic( x, y, 
								p_gx, p_gy,
								dir );
							queue.Enqueue( c );
						}
					}
				}
			}
		}
	}




	// ----------------------------------------------------------------
	//  Name:           AStarHeuristic
	//  Description:    calculates the A* heuristic
	//  Arguments:      x, y: coordinates of the current cell
	//                  gx, gy: goal coordinates
	//                  dir: direction of the target cell
	//  Return Value:   the heuristic
	// ----------------------------------------------------------------
	float AStarHeuristic( int x, int y, int gx, int gy, int dir )
	{
		x = x + DIRTABLE[dir][0];
		y = y + DIRTABLE[dir][1];

		return CellDistance( x, y, gx, gy );

	}

	// ----------------------------------------------------------------
	//  Name:           PathAStar
	//  Description:    pathfinder that uses the A* heuristic
	//  Arguments:      p_map: the map
	//                  p_x, p_y: starting coordinates
	//                  p_gx, p_gy: goal coordinates
	//  Return Value:   None
	// ----------------------------------------------------------------
	void PathAStar( Array2D<Cell>& p_map, 
		int p_x, int p_y, 
		int p_gx, int p_gy )
	{
		Coordinate c;
		int x, y;
		int ax, ay;
		int dir;
		float distance;

		static Heap<Coordinate> queue( QUEUESIZE, CompareCoordinatesDescending );
		queue.m_count = 0;

		// clear the cells first.
		ClearCells( p_map );

		// enqueue the starting cell in the queue
		c.x = p_x;
		c.y = p_y;
		queue.Enqueue( c );


		// start the main loop
		while( queue.m_count != 0 )
		{
			// pull the first cell off the queue and process it.
			x = queue.Item().x;
			y = queue.Item().y;
			queue.Dequeue();

			// make sure the node isn't already marked. If it is, do
			// nothing.
			if( p_map.Get( x, y ).m_marked == false )
			{
				// mark the cell as it is pulled off the queue.
				p_map.Get( x, y ).m_marked = true;

				// quit out if the goal has been reached.
				if( x == p_gx && y == p_gy )
					break;

				// loop through each direction.
				for( dir = 0; dir < 8; dir++ )
				{
					// retrieve the coordinates of the current adjacent cell
					ax = x + DIRTABLE[dir][0];
					ay = y + DIRTABLE[dir][1];

					// check to see if the adjacent cell is a valid index, 
					// passable, and not marked.
					if( ax >= 0 && ax < p_map.Width() && 
						ay >= 0 && ay < p_map.Height() &&
						p_map.Get( ax, ay ).m_passable == true &&
						p_map.Get( ax, ay ).m_marked == false )
					{
						// calculate the distance to get into this cell.
						// this is calulated as:
						// distance of the current cell plus
						// the weight of the adjacent cell times the distance
						// of the cell.
						// diagonal cell's cost is around 1.4 times the cost of
						// a horizontal or vertical cell.
						distance = p_map.Get( x, y ).m_distance + 
							p_map.Get( ax, ay ).m_weight * DISTTABLE[dir]; 

						// check if the node has already been calculated before
						if( p_map.Get( ax, ay ).m_lastx != -1 )
						{
							// the node has already been calculated, see if the
							// new distance is shorter. If so, update the links.
							if( distance < p_map.Get( ax, ay ).m_distance )
							{
								// the new distance is shorter, update the links
								p_map.Get( ax, ay ).m_lastx = x;
								p_map.Get( ax, ay ).m_lasty = y;
								p_map.Get( ax, ay ).m_distance = distance;

								// add the cell to the queue.
								c.x = ax;
								c.y = ay;
								c.heuristic = distance + 
									AStarHeuristic( x, y, 
									p_gx, p_gy, 
									dir );
								queue.Enqueue( c );
							}
						}
						else
						{
							// set the links and the distance
							p_map.Get( ax, ay ).m_lastx = x;
							p_map.Get( ax, ay ).m_lasty = y;
							p_map.Get( ax, ay ).m_distance = distance;

							// add the cell to the queue.
							c.x = ax;
							c.y = ay;
							c.heuristic = distance + 
								AStarHeuristic( x, y,
								p_gx, p_gy,
								dir );
							queue.Enqueue( c );
						}
					}
				}
			}
		}
	}
};


#endif
