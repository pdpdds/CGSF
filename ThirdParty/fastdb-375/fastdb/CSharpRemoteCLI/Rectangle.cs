namespace CSharpCLI { 

using System;



/// <summary>
/// Class representing R2-rectangle
/// </summary>
public class Rectangle { 
    /// <summary>X coordinate of top left corner</summary>
    public int x0;
    /// <summary>Y coordinate of top left corner</summary>
    public int y0;
    /// <summary>X coordinate of bottom right corner</summary>
    public int x1;
    /// <summary>Y coordinate of bottom right corner</summary>
    public int y1;

    /// <summary>
    /// Hash code method for references
    /// </summary>
    ///
    public override int GetHashCode() { 
        return x0 ^ x1 ^ y0 ^ y1;
    }
  
    /// <summary>
    /// Print rectangle coordinates
    /// </summary>
    ///
    public override string ToString() { 
        return "(" + x0 + ", " +  y0 + ")-(" + x1 + ", " + y1 + ")";
    }

    /// <summary>
    /// Distance between two rectangles
    /// </summary>
    /// <param name="r">first rectangle</param>
    /// <param name="q">second rectangle</param>
    /// <returns>distance between two rectangles</returns>
    ///
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
        return (int)Math.Sqrt((double)d);
    }

    /// <summary>
    /// Rectangle area
    /// </summary>
    ///
    public int area() {
        return (y1-y0)*(x1-x0);
    }

    /// <summary>
    /// Calculate cover of two rectangles
    /// </summary>
    /// <param name="p">first rectangle</param>
    /// <param name="q">second rectangle</param>
    /// <returns>minimal rectangle containing both specified rectangles</returns>
    ///
    public static Rectangle join(Rectangle p, Rectangle q) {
        Rectangle res = new Rectangle(p);
        res.join(q);
        return res;
    }
    
    /// <summary>
    /// Calculate cover of two rectangles
    /// This rectangle is changesd to be the minimal rectangle containing 
    /// original rectangle and specified rectangles
    /// </summary>
    /// <param name="r">another rectangle</param>
    ///    
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

    /// <summary>
    /// Check whether two rectangles are the same
    /// </summary>
    ///
    public bool equals(Object obj) {
        if ((obj is Rectangle)) { 
            return false;
        }
        Rectangle r = (Rectangle)obj;
        return x0 == r.x0 && x1 == r.x1 && y0 == r.y0 && y1 == r.y1;
    }

    /// <summary>
    /// Check whether two rectngles overlap
    /// </summary>
    ///
    public bool overlaps(Rectangle r) { 
        return x0 <= r.x1 && y0 <= r.y1 && r.x0 <= x1 && r.y0 <= y1;
    }

    /// <summary>
    /// Checks whether this rectangle contains specified <code>r</code> rectangle
    /// </summary>
    ///
    public bool contains(Rectangle r) { 
        return x0 <= r.x0 && y0 <= r.y0 && x1 >= r.x1 && y1 >= r.y1;
    }

    /// <summary>
    /// Contruct rectangle with specified coordinates
    /// </summary>
    ///
    public Rectangle(int x0, int y0, int x1, int y1) { 
        this.x0 = x0;
        this.x1 = x1;
        this.y0 = y0;
        this.y1 = y1;
    }
    
    /// <summary>
    /// Create copy of the rectangle
    /// </summary>
    ///
    public Rectangle(Rectangle r) { 
        this.x0 = r.x0;
        this.x1 = r.x1;
        this.y0 = r.y0;
        this.y1 = r.y1;
    }

    /// <summary>
    /// Create rectangle with all zero coordinates
    /// </summary>
    ///
    public Rectangle() {}
}


}







