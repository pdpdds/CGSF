package jnicli;
import java.util.*;

class IncrementalCursor implements Cursor { 
    IncrementalCursor(DatabaseJNI db, long cursor, ClassDescriptor desc) { 
        this.db = db;
        this.desc = desc;
        this.cursor = cursor;
        nextOid = db.jniNext(cursor);
    }

    public int size() { 
        return db.jniGetNumberOfSelectedRecords(cursor);
    }
        
    public boolean hasMoreElements() {
        return nextOid != 0;
    }

    public Object nextElement() {
        if  (nextOid == 0) { 
            throw new NoSuchElementException();
        }
        Object obj = db.unswizzleObject(cursor, desc);
        currOid = nextOid;
        nextOid = db.jniNext(cursor);
        return obj;
    }

    public long getOid() { 
        return currOid;
    }

    public void update() { 
        throw new CliException("Cursor not in update mode");
    }

    protected void finalize() { 
        if (nextOid != 0) { 
            db.jniCloseCursor(cursor);
        }
    }

    DatabaseJNI     db;
    long            nextOid;
    long            currOid;
    long            cursor;
    ClassDescriptor desc;
}
    