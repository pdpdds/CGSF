/*
** Sorting stuff by Dann Corbit and Pete Filandr.
** (dcorbit@connx.com and pfilandr@mindspring.com)
** Use it however you like.
*/

//
//  The insertion sort template is used for small partitions.
//  Insertion sort is stable.
//

template < class e_type >
void insertion_sort(e_type * array, size_t nmemb)
{
    e_type temp,
          *last,
          *first,
          *middle;
    if (nmemb > 1) {
        first = middle = 1 + array;
        last = nmemb - 1 + array;
        while (first != last) {
            ++first;
            if ((*(middle) > *(first))) {
                middle = first;
            }
        }
        if ((*(array) > *(middle))) {
            ((void) ((temp) = *(array), *(array) = *(middle), *(middle) = (temp)));
        }
        ++array;
        while (array != last) {
            first = array++;
            if ((*(first) > *(array))) {
                middle = array;
                temp = *middle;
                do {
                    *middle-- = *first--;
                } while ((*(first) > *(&temp)));
                *middle = temp;
            }
        }
    }
}

//
// The median estimate is used to choose pivots for the quicksort algorithm
//

template < class e_type >
void median_estimate(e_type * array, size_t n)
{
    e_type          temp;
    long unsigned   lu_seed = 123456789LU;
    const size_t    k = ((lu_seed) = 69069 * (lu_seed) + 362437) % --n;
    ((void) ((temp) = *(array), *(array) = *(array + k), *(array + k) = (temp)));
    if ((*((array + 1)) > *((array)))) {
        (temp) = *(array + 1);
        if ((*((array + n)) > *((array)))) {
            *(array + 1) = *(array);
            if ((*(&(temp)) > *((array + n)))) {
                *(array) = *(array + n);
                *(array + n) = (temp);
            } else {
                *(array) = (temp);
            }
        } else {
            *(array + 1) = *(array + n);
            *(array + n) = (temp);
        }
    } else {
        if ((*((array)) > *((array + n)))) {
            if ((*((array + 1)) > *((array + n)))) {
                (temp) = *(array + 1);
                *(array + 1) = *(array + n);
                *(array + n) = *(array);
                *(array) = (temp);
            } else {
                ((void) (((temp)) = *((array)), *((array)) = *((array + n)), *((array + n)) = ((temp))));
            }
        }
    }
}


//
// This is the heart of the quick sort algorithm used here.
// If the sort is going quadratic, we switch to heap sort.
// If the partition is small, we switch to insertion sort.
//

template < class e_type >
void qloop(e_type * array, size_t nmemb, size_t d)
{
    e_type temp,
          *first,
          *last;
    while (nmemb > 50) {
        if (sorted(array, nmemb)) {
            return;
        }
        if (!d--) {
            heapsort(array, nmemb);
            return;
        }
        median_estimate(array, nmemb);
        first = 1 + array;
        last = nmemb - 1 + array;
        do {
            ++first;
        } while ((*(array) > *(first)));
        do {
            --last;
        } while ((*(last) > *(array)));
        while (last > first) {
            ((void) ((temp) = *(last), *(last) = *(first), *(first) = (temp)));
            do {
                ++first;
            } while ((*(array) > *(first)));
            do {
                --last;
            } while ((*(last) > *(array)));
        }
        ((void) ((temp) = *(array), *(array) = *(last), *(last) = (temp)));
        qloop(last + 1, nmemb - 1 + array - last, d);
        nmemb = last - array;
    }
    insertion_sort(array, nmemb);
}

//
// This heap sort is better than average because it uses Lamont's heap.
//

template < class e_type >
void heapsort(e_type * array, size_t nmemb)
{
    size_t i,
           child,
           parent;
    e_type temp;
    if (nmemb > 1) {
        i = --nmemb / 2;
        do {
            {
                (parent) = (i);
                (temp) = (array)[(parent)];
                (child) = (parent) * 2;
                while ((nmemb) > (child)) {
                    if ((*((array) + (child) + 1) > *((array) + (child)))) {
                        ++(child);
                    }
                    if ((*((array) + (child)) > *(&(temp)))) {
                        (array)[(parent)] = (array)[(child)];
                        (parent) = (child);
                        (child) *= 2;
                    } else {
                        --(child);
                        break;
                    }
                }
                if ((nmemb) == (child) && (*((array) + (child)) > *(&(temp)))) {
                    (array)[(parent)] = (array)[(child)];
                    (parent) = (child);
                }
                (array)[(parent)] = (temp);
            }
        } while (i--);
        ((void) ((temp) = *(array), *(array) = *(array + nmemb), *(array + nmemb) = (temp)));
        for (--nmemb; nmemb; --nmemb) {
            {
                (parent) = (0);
                (temp) = (array)[(parent)];
                (child) = (parent) * 2;
                while ((nmemb) > (child)) {
                    if ((*((array) + (child) + 1) > *((array) + (child)))) {
                        ++(child);
                    }
                    if ((*((array) + (child)) > *(&(temp)))) {
                        (array)[(parent)] = (array)[(child)];
                        (parent) = (child);
                        (child) *= 2;
                    } else {
                        --(child);
                        break;
                    }
                }
                if ((nmemb) == (child) && (*((array) + (child)) > *(&(temp)))) {
                    (array)[(parent)] = (array)[(child)];
                    (parent) = (child);
                }
                (array)[(parent)] = (temp);
            }
            ((void) ((temp) = *(array), *(array) = *(array + nmemb), *(array + nmemb) = (temp)));
        }
    }
}

// 
// We use this to check to see if a partition is already sorted.
// 

template < class e_type >
int sorted(e_type * array, size_t nmemb)
{
    for (--nmemb; nmemb; --nmemb) {
        if ((*(array) > *(array + 1))) {
            return 0;
        }
        ++array;
    }
    return 1;
}

// 
// We use this to check to see if a partition is already reverse-sorted.
// 

template < class e_type >
int             rev_sorted(e_type * array, size_t nmemb)
{
    for (--nmemb; nmemb; --nmemb) {
        if ((*(array + 1) > *(array))) {
            return 0;
        }
        ++array;
    }
    return 1;
}

// 
// We use this to reverse a reverse-sorted partition.
// 

template < class e_type >
void rev_array(e_type * array, size_t nmemb)
{
    e_type          temp,
        *end;
    for (end = array + nmemb - 1; end > array; ++array) {
        ((void) ((temp) = *(array), *(array) = *(end), *(end) = (temp)));
        --end;
    }
}

// 
// Introspective quick sort algorithm user entry point.
// You do not need to directly call any other sorting template.
// This sort will perform very well under all circumstances.
// 

template < class e_type >
void iqsort(e_type * array, size_t nmemb)
{
    size_t d,
           n;
    if (nmemb > 1 && !sorted(array, nmemb)) {
        if (!rev_sorted(array, nmemb)) {
            n = nmemb / 4;
            d = 2;
            while (n) {
                ++d;
                n /= 2;
            }
            qloop(array, nmemb, 2 * d);
        } else {
            rev_array(array, nmemb);
        }
    }
}

