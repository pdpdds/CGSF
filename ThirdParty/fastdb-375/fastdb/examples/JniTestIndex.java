import jnicli.*;

import java.util.*;

public class JniTestIndex { 
    static class Record { 
        String strKey;
        long   intKey;

        static final String CONSTRAINTS = "strKey using index, intKey using index";
    }

    final static int nRecords = 100000;
    final static int initSize = 8*1024*1024; // 40Mb page pool

    static public void main(String[] args) {    
        Database db = new DatabaseJNI();
        db.open(Database.READ_WRITE, "testjni", "testjni.dbs", initSize, 0);
        long start = System.currentTimeMillis();
        long key = 1999;
        int i;
        for (i = 0; i < nRecords; i++) { 
            Record rec = new Record();
            key = (3141592621L*key + 2718281829L) % 1000000007L;
            rec.intKey = key;
            rec.strKey = Long.toString(key);
            db.insert(rec);
        }
        db.commit();
        System.out.println("Elapsed time for inserting " + nRecords + " records: " 
                           + (System.currentTimeMillis() - start) + " milliseconds");
        
        start = System.currentTimeMillis();
        key = 1999;
        for (i = 0; i < nRecords; i++) { 
            key = (3141592621L*key + 2718281829L) % 1000000007L;
            Cursor cursor1 = db.select(Record.class, "strKey='" + key + "'", 0);
            if (cursor1.size() != 1 ) {
                throw new Error("search by strKey returns " + cursor1.size() + " instead of 1");
            }
            Record rec1 = (Record)cursor1.nextElement();
            
            Cursor cursor2 = db.select(Record.class, "intKey=" + key, 0);
            if (cursor2.size() != 1 ) {
                throw new Error("search by intKey returns " + cursor2.size() + " instead of 1");
            }
            Record rec2 = (Record)cursor2.nextElement();
            if (rec1.intKey != rec2.intKey || rec1.intKey != key || !rec1.strKey.equals(rec2.strKey)) {
                throw new Error("Inconsitent fetch");
            }
        }
        System.out.println("Elapsed time for performing " + nRecords*2 + " index searches: " 
                           + (System.currentTimeMillis() - start) + " milliseconds");
        
        start = System.currentTimeMillis();
        db.delete(Record.class, "");
        System.out.println("Elapsed time for deleting " + nRecords + " records: " 
                           + (System.currentTimeMillis() - start) + " milliseconds");
        db.close();
    }
}
