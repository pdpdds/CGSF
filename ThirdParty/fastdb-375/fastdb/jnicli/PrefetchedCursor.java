package jnicli;

import java.util.*;
import java.io.Serializable;

public class PrefetchedCursor implements Cursor, Serializable { 
    // Public constructor to allow instantiation of this class
    public PrefetchedCursor() {}

    PrefetchedCursor(DatabaseJNI db, long cursor, ClassDescriptor desc) { 
        long oid;
        int size =  db.jniGetNumberOfSelectedRecords(cursor);
        objects = new Object[size];
        oids = new long[size];
        for (int i = 0; i < size; i++) {
            oids[i] = db.jniNext(cursor);            
            objects[i] = db.unswizzleObject(cursor, desc);
        }
        db.jniCloseCursor(cursor);
    }
        
    public boolean hasMoreElements() {
        return i < objects.length;
    }

    public Object nextElement() {
        if  (i >= objects.length) { 
            throw new NoSuchElementException();
        }
        return objects[i++];
    }

    public long getOid() { 
        if  (i == 0) { 
            throw new NoSuchElementException();
        }
        return oids[i-1];
    }

    public void update() { 
        throw new CliException("Cursor not in update mode");
    }

    public int size() { 
        return objects.length;
    }

    // make this fields public in order to be accessible in JDK1.1 environment
    public Object[] objects;
    public long[]   oids;
    transient int i;
}
