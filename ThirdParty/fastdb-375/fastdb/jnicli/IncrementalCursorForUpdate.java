package jnicli;
import java.util.*;

class IncrementalCursorForUpdate extends IncrementalCursor { 
    IncrementalCursorForUpdate(DatabaseJNI db, long cursor, ClassDescriptor desc) { 
        super(db, cursor, desc);
    }

    public Object nextElement() {
        return currObj = super.nextElement();
    }

    public void update() { 
        if  (currOid == 0) { 
            throw new NoSuchElementException();
        }
        db.update(currOid, currObj);
    }

    Object currObj;
}
    