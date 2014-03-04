// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// sorts.h
// This file holds all of the sorting algorithms
// ============================================================================
#ifndef SORTS_H
#define SORTS_H


namespace CGBase
{

#include "Array.h"


	// ----------------------------------------------------------------
	//  Name:           RADIXBINSIZE
	//  Description:    The size of each bin in the radix sort. If this
	//                  is too small, you can make it larger.
	// ----------------------------------------------------------------
	const int RADIXBINSIZE = 1024;


	// ----------------------------------------------------------------
	//  Name:           Swap
	//  Description:    a simple swap function, swaps two pieces of data
	//  Arguments:      a, b: the data to swap
	//  Return Value:   None
	// ----------------------------------------------------------------
	template<class DataType>
	void Swap( DataType& a, DataType& b )
	{
		static DataType t;
		t = a;
		a = b;
		b = t;
	}



	// ----------------------------------------------------------------
	//  Name:           BubbleSort
	//  Description:    sorts an array using the bubble sort
	//  Arguments:      p_array: the array to sort
	//                  p_compare: the comparison function
	//  Return Value:   None
	// ----------------------------------------------------------------
	template<class DataType>
	void BubbleSort( Array<DataType>& p_array, int (*p_compare)(DataType, DataType) )
	{
		int top = p_array.Size() - 1;
		int index;
		int swaps = 1;

		// loop through while the top is not 0 and swaps were made during the 
		// last iteration
		while( top != 0 && swaps != 0 )
		{
			// set the swap variable to 0, because we hanvent made any swaps yet.
			swaps = 0;

			// perform one iteration
			for( index = 0; index < top; index++ )
			{
				// swap the indexes if neccessary
				if( p_compare( p_array[index], p_array[index + 1] ) > 0 )
				{
					Swap( p_array[index], p_array[index + 1] );
					swaps++;
				}
			}
			top--;
		}
	}


	// ----------------------------------------------------------------
	//  Name:           HeapWalkDown
	//  Description:    walks an item down an array, as if it were a heap
	//  Arguments:      p_array: the heap
	//                  p_index: index of the item to walk down
	//                  p_maxIndex: last valid index of the heap
	//                  p_compare: the comparison function
	//  Return Value:   None
	// ----------------------------------------------------------------
	template<class DataType>
	void HeapWalkDown( Array<DataType>& p_array, 
		int p_index,
		int p_maxIndex,
		int (*p_compare)(DataType, DataType) )
	{
		int parent = p_index;
		int child = p_index * 2;
		DataType temp = p_array[parent - 1];

		// loop through, walking node down heap until both children are
		// smaller than node.
		while( child <= p_maxIndex )
		{
			// if left child is not the last node in the tree, then
			// find out which of the current node's children is largest.
			if( child < p_maxIndex )
			{
				if( p_compare( p_array[child - 1], p_array[child] ) < 0 )
				{   // change the pointer to the right child, since it is larger.
					child++;
				}
			}
			// if the node to walk down is lower than the highest value child,
			// move the child up one level.
			if( p_compare( temp, p_array[child - 1] ) < 0 )
			{
				p_array[parent - 1] = p_array[child - 1];
				parent = child;
				child *= 2;
			}
			else
				break;
		}
		p_array[parent - 1] = temp;
	}


	// ----------------------------------------------------------------
	//  Name:           HeapSort
	//  Description:    performs the heapsort on an array.
	//  Arguments:      p_array: array to sort
	//                  p_compare: comparison function
	//  Return Value:   None
	// ----------------------------------------------------------------
	template<class DataType>
	void HeapSort( Array<DataType>& p_array, int (*p_compare)(DataType, DataType) )
	{
		int i;
		int maxIndex = p_array.Size();
		int rightindex = maxIndex / 2;

		// walk everything down to transform it into a heap
		for( i = rightindex; i > 0; i-- )
		{
			HeapWalkDown( p_array, i, maxIndex, p_compare );
		}

		// remove the top, walk everything down.
		while( maxIndex > 0 )
		{
			Swap( p_array[0], p_array[maxIndex - 1] );
			maxIndex--;
			HeapWalkDown( p_array, 1, maxIndex, p_compare );
		}
	}




	// ----------------------------------------------------------------
	//  Name:           FindMedianOfThree
	//  Description:    Finds the median of three values in a segment
	//  Arguments:      p_array: the array to find the median of
	//                  p_first: the first index in the segment
	//                  p_size: the size of the segment
	//                  p_compare: comparison function
	//  Return Value:   index of the median
	// ----------------------------------------------------------------
	template<class DataType>
	int FindMedianOfThree( Array<DataType>& p_array, 
		int p_first, 
		int p_size,
		int (*p_compare)(DataType, DataType) )
	{
		// calculate the last and middle indexes
		int last = p_first + p_size - 1;
		int mid = p_first + (p_size / 2);

		// if the first index is the lowest,
		if( p_compare( p_array[p_first], p_array[mid] ) < 0 && 
			p_compare( p_array[p_first], p_array[last] ) < 0 )
		{
			// then the smaller of the middle and the last is the median.
			if( p_compare( p_array[mid], p_array[last] ) < 0 )
				return mid;
			else
				return last;
		}

		// if the middle index is the lowest,
		if( p_compare( p_array[mid], p_array[p_first] ) < 0 && 
			p_compare( p_array[mid], p_array[last] ) < 0 )
		{
			// then the smaller of the first and last is the median
			if( p_compare( p_array[p_first], p_array[last] ) < 0 )
				return p_first;
			else
				return last;
		}

		// by this point, we know that the last index is the lowest,
		// so the smaller of the middle and the first is the median.
		if( p_compare( p_array[mid], p_array[p_first] ) < 0 )
			return mid;
		else
			return p_first;
	}


	// ----------------------------------------------------------------
	//  Name:           QuickSort
	//  Description:    quicksorts the array
	//  Arguments:      p_array: the array to sort
	//                  p_first: first index of the segment to sort
	//                  p_size: size of the segment
	//                  p_compare: comparison function
	//  Return Value:   None
	// ----------------------------------------------------------------
	template<class DataType>
	void QuickSort( Array<DataType>& p_array, 
		int p_first, 
		int p_size, 
		int (*p_compare)(DataType, DataType) )
	{
		DataType pivot;
		int last = p_first + p_size - 1;    // index of the last cell
		int lower = p_first;                // index of the lower cell
		int higher = last;                  // index of the upper cell
		int mid;                            // index of the median value

		// if the size of the array to sort is greater than 1, then sort it.
		if( p_size > 1 )
		{
			// find the index of the median value, and set that as the pivot.
			mid = FindMedianOfThree( p_array, p_first, p_size, p_compare );
			pivot = p_array[mid];

			// move the first value in the array into the place where the pivot was
			p_array[mid] = p_array[p_first];

			// while the lower index is lower than the higher index
			while( lower < higher )
			{
				// iterate downwards until a value lower than the pivot is found
				while( p_compare( pivot, p_array[higher] ) < 0 && lower < higher )
					higher--;

				// if the previous loop found a value lower than the pivot, 
				// higher will not equal lower.
				if( higher != lower ) 
				{
					// so move the value of the higher index into the lower index 
					// (which is empty), and move the lower index up.
					p_array[lower] = p_array[higher];
					lower++;
				}

				// now iterate upwards until a value greater than the pivot is found
				while( p_compare( pivot, p_array[lower] ) > 0 && lower < higher )
					lower++;

				// if the previous loop found a value greater than the pivot,
				// higher will not equal lower
				if( higher != lower )
				{
					// move the value at the lower index into the higher index,
					// (which is empty), and move the higher index down.
					p_array[higher] = p_array[lower];
					higher--;
				}
			}

			// at the end of the main loop, the lower index will be empty, so
			// put the pivot in there.
			p_array[lower] = pivot;

			// recursively quicksort the left half
			QuickSort( p_array, p_first, lower - p_first, p_compare );

			// recursively quicksort the right half.
			QuickSort( p_array, lower + 1, last - lower, p_compare );
		}
	}



	// ----------------------------------------------------------------
	//  Name:           RadixSort2
	//  Description:    The 2 bit radix sort
	//  Arguments:      p_array: array to sort
	//                  p_passes: number of passes to perform
	//  Return Value:   None
	// ----------------------------------------------------------------
	void RadixSort2( Array<int>& p_array, int p_passes )
	{
		// error, might not be able to sort this array
		if( p_array.Size() > RADIXBINSIZE )
			return;

		static int bins[2][RADIXBINSIZE];
		int bincount[2];

		int radix = 1;
		int shift = 0;
		int index;
		int binindex;
		int currentbin;

		while( p_passes != 0 )
		{
			// decrement the number of passes.
			p_passes--;

			// clear the bin counts
			bincount[0] = bincount[1] = 0;

			// place the items in the bins
			for( index = 0; index < p_array.Size(); index++ )
			{
				binindex = (p_array[index] & radix) >> shift;
				bins[binindex][bincount[binindex]] = p_array[index];
				bincount[binindex]++;
			}

			// put the items back in the array
			index = 0;
			for( currentbin = 0; currentbin < 2; currentbin++ )
			{
				binindex = 0;
				while( bincount[currentbin] > 0 )
				{
					p_array[index] = bins[currentbin][binindex];
					binindex++;
					bincount[currentbin]--;
					index++;
				}
			}

			radix <<= 1;
			shift += 1;
		}
	}


	// ----------------------------------------------------------------
	//  Name:           RadixSort4
	//  Description:    The 4 bit radix sort
	//  Arguments:      p_array: array to sort
	//                  p_passes: number of passes to perform
	//  Return Value:   None
	// ----------------------------------------------------------------
	void RadixSort4( Array<int>& p_array, int p_passes )
	{
		// error, might not be able to sort this array
		if( p_array.Size() > RADIXBINSIZE )
			return;

		static int bins[4][RADIXBINSIZE];
		int bincount[4];

		int radix = 3;
		int shift = 0;
		int index;
		int binindex;
		int currentbin;

		while( p_passes != 0 )
		{
			// decrement the number of passes.
			p_passes--;

			// clear the bin counts
			bincount[0] = bincount[1] = bincount[2] = bincount[3] = 0;

			// place the items in the bins
			for( index = 0; index < p_array.Size(); index++ )
			{
				binindex = (p_array[index] & radix) >> shift;
				bins[binindex][bincount[binindex]] = p_array[index];
				bincount[binindex]++;
			}

			// put the items back in the array
			index = 0;
			for( currentbin = 0; currentbin < 4; currentbin++ )
			{
				binindex = 0;
				while( bincount[currentbin] > 0 )
				{
					p_array[index] = bins[currentbin][binindex];
					binindex++;
					bincount[currentbin]--;
					index++;
				}
			}

			radix <<= 2;
			shift += 2;
		}
	}


	// ----------------------------------------------------------------
	//  Name:           RadixSort16
	//  Description:    The 16 bit radix sort
	//  Arguments:      p_array: array to sort
	//                  p_passes: number of passes to perform
	//  Return Value:   None
	// ----------------------------------------------------------------
	void RadixSort16( Array<int>& p_array, int p_passes )
	{
		// error, might not be able to sort this array
		if( p_array.Size() > RADIXBINSIZE )
			return;

		static int bins[16][RADIXBINSIZE];
		int bincount[16];

		int radix = 15;
		int shift = 0;
		int index;
		int binindex;
		int currentbin;

		while( p_passes != 0 )
		{
			// decrement the number of passes.
			p_passes--;

			// clear the bin counts
			for( index = 0; index < 16; index++ )
				bincount[index] = 0;

			// place the items in the bins
			for( index = 0; index < p_array.Size(); index++ )
			{
				binindex = (p_array[index] & radix) >> shift;
				bins[binindex][bincount[binindex]] = p_array[index];
				bincount[binindex]++;
			}

			// put the items back in the array
			index = 0;

			for( currentbin = 0; currentbin < 16; currentbin++ )
			{
				binindex = 0;
				while( bincount[currentbin] > 0 )
				{
					p_array[index] = bins[currentbin][binindex];
					binindex++;
					bincount[currentbin]--;
					index++;
				}
			}

			radix <<= 4;
			shift += 4;
		}
	}
};

#endif