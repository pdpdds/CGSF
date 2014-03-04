package javacli;

/**
 * Class representing R2-rectangle
 */
public class Rectangle { 
    public int x0;
    public int y0;
    public int x1;
    public int y1;

    /**
     * Hash code method for references
     */
    public int hashCode() { 
        return x0 ^ x1 ^ y0 ^ y1;
    }

    static Class TYPE;

    static { 
        try { 
            TYPE = Class.forName("javacli.Rectangle");
        } catch (ClassNotFoundException x) { 
        }
    }

    public String toString() { 
        return "(" + x0 + ", " +  y0 + ")-(" + x1 + ", " + y1 + ")";
    }

    /**
     * Distance between two rectangles
     * @param r first rectangle
     * @param q second rectangle
     * @return distance between two rectangles
     */
    public static int distance(Rectangle r, Rectangle q) { 
        if (r.overlaps(q)) { 
            return 0;
        }
        int d = 0;
        if (r.x0 > q.x1) { 
            d += (r.x0 - q.x1)*(r.x0 - q.x1);
        } else if (q.x0 > r.x1) { 
            d += (q.x0 - r.x1)*(q.x0 - r.x1);
        }
        if (r.y0 > q.y1) { 
            d += (r.y0 - q.y1)*(r.y0 - q.y1);
        } else if (q.y0 > r.y1) { 
            d += (q.y0 - r.y1)*(q.y0 - r.y1);
        }
        return (int)Math.sqrt((double)d);
    }

    /**
     * Rectangle area
     */
    public int area() {
        return (y1-y0)*(x1-x0);
    }

    /**
     * Calculate cover of two rectangles
     * @param p first rectangle
     * @param q second rectangle
     * @return minimal rectangle containing both specified rectangles
     */
    public static Rectangle join(Rectangle p, Rectangle q) {
        Rectangle res = new Rectangle(p);
        res.join(q);
        return res;
    }
    
    /**
     * Calculate cover of two rectangles
     * This rectangle is changesd to be the minimal rectangle containing 
     * original rectangle and specified rectangles
     * @param r another rectangle
     */    
    public void join(Rectangle r) { 
        if (x0 > r.x0) { 
            x0 = r.x0;
        }
        if (y0 > r.y0) { 
            y0 = r.y0;
        }
        if (x1 < r.x1) { 
            x1 = r.x1;
        }
        if (y1 < r.y1) { 
            y1 = r.y1;
        }
    }

    /**
     * Comparison method for rectangles
     */

    /**
     * Checks whether two rectangles are the same
     */
    public boolean equals(Object obj) {
        if ((obj instanceof Rectangle)) { 
            return false;
        }
        Rectangle r = (Rectangle)obj;
        return x0 == r.x0 && x1 == r.x1 && y0 == r.y0 && y1 == r.y1;
    }

    /**
     * Check whether two rectngles overlaps
     */
    public boolean overlaps(Rectangle r) { 
        return x0 <= r.x1 && y0 <= r.y1 && r.x0 <= x1 && r.y0 <= y1;
    }

    /**
     * Checks whether this rectangle contains specified <code>r</code> rectangle
     */
    public boolean contains(Rectangle r) { 
        return x0 <= r.x0 && y0 <= r.y0 && x1 >= r.x1 && y1 >= r.y1;
    }

    /**
     * Contruct rectangle with specified coordinates
     */
    public Rectangle(int x0, int y0, int x1, int y1) { 
        this.x0 = x0;
        this.x1 = x1;
        this.y0 = y0;
        this.y1 = y1;
    }
    
    /**
     * Create copy of the rectangle
     */
    public Rectangle(Rectangle r) { 
        this.x0 = r.x0;
        this.x1 = r.x1;
        this.y0 = r.y0;
        this.y1 = r.y1;
    }

    /**
     * Create rectangle with all zero coordinates
     */
    public Rectangle() {}
}

