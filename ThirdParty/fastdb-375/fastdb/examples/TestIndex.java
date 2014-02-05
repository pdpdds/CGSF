import javacli.*;

class Record { 
    String strKey;
    long   intKey;
}

public class TestIndex { 
    final static int nRecords = 100000;

    static public void main(String[] args) {    
        Connection con = new Connection();
        con.open("localhost", 6100);

        if (con.createTable(Record.class, null)) { 
            con.addIndex("Record", "strKey");
            con.addIndex("Record", "intKey");
        }

        long start = System.currentTimeMillis();
        long key = 1999;
        int i;
        for (i = 0; i < nRecords; i++) { 
            Record rec = new Record();
            key = (3141592621L*key + 2718281829L) % 1000000007L;
            rec.intKey = key;
            rec.strKey = Long.toString(key);
            con.insert(rec);
        }
        con.commit();
        System.out.println("Elapsed time for inserting " + nRecords + " records: " 
                           + (System.currentTimeMillis() - start) + " milliseconds");
        
	Statement strQuery = con.createStatement("select * from Record where strKey=%strKey");
	Statement intQuery = con.createStatement("select * from Record where intKey=%intKey");
        ObjectSet cursor;
        start = System.currentTimeMillis();
        key = 1999;
        for (i = 0; i < nRecords; i++) { 
            key = (3141592621L*key + 2718281829L) % 1000000007L;
            strQuery.setString("%strKey", Long.toString(key));
            intQuery.setLong("%intKey", key);
            cursor = strQuery.fetch();
            if (cursor.size() != 1) {
                throw new Error("stmt->fetch 1 returns " + cursor.size() + " instead of 1");
            }
            Record rec1 = (Record)cursor.getNext();
            cursor.close();

            cursor = intQuery.fetch();
            if (cursor.size() != 1) {
                throw new Error("stmt->fetch 2 returns " + cursor.size() + " instead of 1");
            }
            Record rec2 = (Record)cursor.getNext();
            cursor.close();

            if (rec1.intKey != rec2.intKey || rec1.intKey != key || !rec1.strKey.equals(rec2.strKey)) {
                throw new Error("Inconsitent fetch");
            }
        }
        System.out.println("Elapsed time for performing " + nRecords*2 + " index searches: " 
                           + (System.currentTimeMillis() - start) + " milliseconds");
        
        start = System.currentTimeMillis();
        key = 1999;
        for (i = 0; i < nRecords; i++) {
            key = (3141592621L*key + 2718281829L) % 1000000007L;
            intQuery.setLong("%intKey", key);
            cursor = intQuery.fetch(true);
            if (cursor.size() != 1) {
                throw new Error("stmt->fetch 3 returns " + cursor.size() + " instead of 1");
            }
            cursor.removeAll();
        }
        System.out.println("Elapsed time for deleting " + nRecords + " records: " 
                           + (System.currentTimeMillis() - start) + " milliseconds");
        con.close();
    }
}
