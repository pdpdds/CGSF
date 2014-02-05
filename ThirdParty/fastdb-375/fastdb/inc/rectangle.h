//-< RECTANGLE.H >---------------------------------------------------*--------*
// FastDB                   Version 1.0         (c) 1999  GARRET     *     ?  *
// (Post Relational Database Management System)                      *   /\|  *
//                                                                   *  /  \  *
//                          Created:     26-Nov-2001  K.A. Knizhnik  * / [] \ *
//                          Last update: 26-Nov-2001  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Rectangle class
//-------------------------------------------------------------------*--------*

#ifndef __RECTANGLE_H__
#define __RECTANGLE_H__

BEGIN_FASTDB_NAMESPACE

#ifndef RECTANGLE_DIMENSION
#define RECTANGLE_DIMENSION 2
#endif 

#ifndef RECTANGLE_COORDINATE_TYPE
#define RECTANGLE_COORDINATE_TYPE int4
#define RECTANGLE_AREA_TYPE int8
#endif


typedef RECTANGLE_COORDINATE_TYPE coord_t;
typedef RECTANGLE_AREA_TYPE       area_t;

/**
 * Multidimensional rectangle
 */
class FASTDB_DLL_ENTRY rectangle
{
  public:
    enum { dim = RECTANGLE_DIMENSION };
    /** 
     * Boundary array contains coordinates of two vertices specifying rectangle 
     * of "dim" dimmension. First vertex has coordinaes (boundary[0], ..., boundary[dim])
     * Second vertext has coordinates (boundary[dim], ..., boundary[dim*2-1])
     * Precondition: for i = 0, dim : boundary[i] <= boundary[dim+i]
     */
    coord_t boundary[dim*2];

    /**
     * Calculate distance between two rectangles
     */
    friend coord_t FASTDB_DLL_ENTRY distance(rectangle const& r, rectangle const& q);

    /**
     * Caclulate area of rectangle
     */
    friend area_t area(rectangle const& r) { 
        area_t area = 1;
        for (int i = dim; --i >= 0; area *= r.boundary[i+dim] - r.boundary[i]);
        return area;
    }

    /**
     * Join two rectangles: construct enveloping rectangle
     */
    void operator +=(rectangle const& r) { 
        int i = dim; 
        while (--i >= 0) { 
            boundary[i] = (boundary[i] <= r.boundary[i]) 
                ? boundary[i] : r.boundary[i];
            boundary[i+dim] = (boundary[i+dim] >= r.boundary[i+dim]) 
                ? boundary[i+dim] : r.boundary[i+dim];
        }
    }
    /**
     * Join two rectangles: construct enveloping rectangle
     */
    rectangle operator + (rectangle const& r) const { 
        rectangle res;
        int i = dim; 
        while (--i >= 0) { 
            res.boundary[i] = (boundary[i] <= r.boundary[i]) 
                ? boundary[i] : r.boundary[i];
            res.boundary[i+dim] = (boundary[i+dim] >= r.boundary[i+dim]) 
                ? boundary[i+dim] : r.boundary[i+dim];
        }
        return res;
    }
    /**
     * Intersect two rectangles
     */
    bool operator & (rectangle const& r) const {
        int i = dim; 
        while (--i >= 0) { 
            if (boundary[i] > r.boundary[i+dim] ||
                r.boundary[i] > boundary[i+dim])
            {
                return false;
            }
        }
        return true;
    }
    /**
     * Check if rectangle is part of other rectanle
     * @return <code>true</code> if this rectangle is part of rectangle <code>r</code>
     */
    bool operator <= (rectangle const& r) const { 
        int i = dim; 
        while (--i >= 0) { 
            if (boundary[i] < r.boundary[i] ||
                boundary[i+dim] > r.boundary[i+dim])
            {
                return false;
            }
        }
        return true;
    }
    /**
     * Check if rectangle is part of other rectanle
     * @return <code>true</code> if this rectangle contains rectangle <code>r</code>
     */
    bool operator >= (rectangle const& r) const { 
        int i = dim; 
        while (--i >= 0) { 
            if (r.boundary[i] < boundary[i] ||
                r.boundary[i+dim] > boundary[i+dim])
            {
                return false;
            }
        }
        return true;
    }

    /**
     * Check if rectangle is strict subpart of other rectanle
     * @return <code>true</code> if this rectangle is part of rectangle <code>r</code> and not the same
     */
    bool operator < (rectangle const& r) const { 
        return *this <= r && *this != r;
    }
    /**
     * Check if rectangle is strict subpart of other rectanle
     * @return <code>true</code> if this rectangle contains rectangle <code>r</code> and not the same
     */
    bool operator > (rectangle const& r) const { 
        return *this >= r && *this != r;
    }
    /**
     * Check if two rectangle are the same
     */
    bool operator == (rectangle const& r) const { 
        int i = dim*2; 
        while (--i >= 0) { 
            if (boundary[i] != r.boundary[i]) { 
                return false;
            }
        }
        return true;
    }
    /**
     * Check if two rectangle are not the same
     */    
    bool operator != (rectangle const& r) const { 
        int i = dim*2; 
        while (--i >= 0) { 
            if (boundary[i] != r.boundary[i]) { 
                return true;
            }
        }
        return false;
    }

    typedef bool (rectangle::*comparator)(rectangle const& r) const;
};

END_FASTDB_NAMESPACE

#endif


