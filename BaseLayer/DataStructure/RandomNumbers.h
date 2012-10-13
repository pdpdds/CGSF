// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// RandomNumbers.h
// This file holds the random number generators
// ============================================================================
#ifndef RANDOMNUMBERS_H
#define RANDOMNUMBERS_H

#include <stdlib.h>

namespace CGBase
{

	// ----------------------------------------------------------------
	//  Name:           RandomPercent
	//  Description:    returns a random floating number from 0.0 to 
	//                  1.0, inclusive.
	//  Arguments:      None
	//  Return Value:   a random float from 0.0 to 1.0.
	// ----------------------------------------------------------------
	float RandomPercent()
	{
		return (float)rand() / (float)RAND_MAX;
	}


	// ----------------------------------------------------------------
	//  Name:           RandomRangeModulo
	//  Description:    Returns a random number with the given range
	//                  using the inferior modulo generator.
	//  Arguments:      p_min: the bottom number
	//                  p_max: the top number
	//  Return Value:   a random number between p_min and p_max, 
	//                  inclusive
	// ----------------------------------------------------------------
	int RandomRangeModulo( int p_min, int p_max )
	{
		int difference = (p_max - p_min) + 1;
		return (rand() % difference) + p_min;
	}


	// ----------------------------------------------------------------
	//  Name:           RandomRange
	//  Description:    Returns a random number with the given range
	//                  using the division generator.
	//  Arguments:      p_min: the bottom number
	//                  p_max: the top number
	//  Return Value:   a random number between p_min and p_max, 
	//                  inclusive
	// ----------------------------------------------------------------
	int RandomRange( int p_min, int p_max )
	{
		int difference = (p_max - p_min) + 1;
		return ( (difference * rand()) / (RAND_MAX + 1) )+ p_min;
	}


	// ----------------------------------------------------------------
	//  Name:           RandomRangeF
	//  Description:    Returns a random floating number with the given 
	//                  range
	//  Arguments:      p_min: the bottom number
	//                  p_max: the top number
	//  Return Value:   a random number between p_min and p_max, 
	//                  inclusive
	// ----------------------------------------------------------------
	float RandomRangeF( float p_min, float p_max )
	{
		float difference = (p_max - p_min);
		return (RandomPercent() * difference) + p_min;
	}

};

#endif