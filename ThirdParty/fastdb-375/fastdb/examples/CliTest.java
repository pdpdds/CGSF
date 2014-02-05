import javacli.*;

import java.util.Date;
import java.util.HashMap;

class person {
    String    name;
    int       salary;
    String    address;
    double    rating;
    String    pets[];
    Reference subordinates[];
}

public class CliTest { 
    static public void main(String args[]) { 
	ConnectionPool pool = null;
	int nConnections = 1;
	if (args.length > 0) { 
	    try { 
		nConnections = Integer.parseInt(args[0]);
	    } catch (NumberFormatException x) {}
	}
	
	if (nConnections > 1) {
	    pool = new ConnectionPool();
	}
	for (int i = 0; i < nConnections; i++) { 
	    Connection con;
	    if (nConnections > 1) { 
		con = pool.newConnection("localhost", 6100);
	    } else { 
		con = new Connection();
		con.open("localhost", 6100);
	    }
	    doTest(con);
	}
	if (nConnections > 1) {
	    pool.close();
	}
    }
    
    static void doTest(Connection con) { 
	int i;
        HashMap refMap = new HashMap();
        refMap.put("subordinates", "person");
        if (con.createTable(person.class, refMap)) { 
            con.addIndex("person", "salary");
            con.addIndex("person", "name");
        }
	person p = new person();
	p.name = "John Smith";
	p.address = "1 Guildhall St., Cambridge CB2 3NH, UK";
	p.salary = 80000;
	p.rating = 3.84;
	p.pets = new String[2];
	p.pets[0] = "dog";
	p.pets[1] = "cat";
	p.subordinates = new Reference[0];
	Reference john = con.insert(p);
	p.name = "Joe Cooker";
	p.address = "Outlook drive, 15/3";
	p.salary = 50000;
	p.rating = 1.76;
	p.pets = new String[1];
	p.pets[0] = "snake";
	p.subordinates = new Reference[1];
	p.subordinates[0] = john;
	Reference joe = con.insert(p);
	p.name = "Hugo Grant";
	p.address = "London, Baker street,12";
	p.salary = 65000;
	p.rating = 2.14;	
	p.pets =  new String[2];
	p.pets[0] = "canary";
	p.pets[1] = "goldfish";
	p.subordinates = new Reference[0];
	con.insert(p);
	con.commit();

	Statement stmt = con.createStatement("select * from person where salary > %salary order by salary");
	stmt.setInt("%salary", 65000);
	ObjectSet cursor = stmt.fetch();
	if (cursor.size() != 1) {
	    System.err.println("stmt->fetch 1 returns " + cursor.size() + " instead of 1");
	}
	System.out.print("NAME\t\tSALARY\tRATING\n");
	while ((p = (person)cursor.getNext()) != null) { 
	    System.out.println(p.name + "\t" + p.salary + "\t" + p.rating);
	}
	stmt.setInt("%salary", 50000);
	cursor = stmt.fetch();
	if (cursor.size() != 2) {
	    System.err.println("stmt->fetch 2 returns " + cursor.size() + " instead of 2");
	}
	stmt.close();

	stmt = con.createStatement("select * from person where current = %ref");
	stmt.setRef("%ref", joe);
	cursor = stmt.fetch();
	if (cursor.size() != 1) {
	    System.err.println("stmt->fetch 3 returns " + cursor.size() + " instead of 1");
	}
	p = (person)cursor.getFirst();
	System.out.println("Object oid=" + cursor.getRef() + ", name=" + p.name + ", address=" 
			   + p.address + ", rating=" + p.rating + ", salary=" + p.salary);
	for (i = 0 ; i < p.pets.length; i++) { 
	    System.out.println("Pet: " + p.pets[i]);
	}
	for (i = 0 ; i < p.subordinates.length; i++) { 
	    Statement s =  con.createStatement("select * from person where current = %ref");
	    s.setRef("%ref", p.subordinates[i]);
	    ObjectSet  objs = s.fetch();
	    person q = (person)objs.getNext();
	    System.out.println("Subordinate: " + q.name);
	}	
	stmt.close();

	stmt = con.createStatement("select * from person where rating > %rating and salary between %min and %max");
	stmt.setDouble("%rating", 2.0);
	stmt.setInt("%min", 50000);
	stmt.setInt("%max", 100000);
	cursor = stmt.fetch(true);
	if (cursor.size() != 2) {
	    System.err.println("stmt->fetch 4 returns " + cursor.size() + " instead of 2");
	}
	p = (person)cursor.getLast();
	System.out.print("NAME\t\tSALARY\tRATING\tADDRESS\n");
	do { 
	    System.out.println(p.name+"\t"+p.salary+"\t"+p.rating+"\t"+p.address);
	    p.salary *= 1.1;
	    cursor.update();
	} while ((p = (person)cursor.getPrev()) != null);
	stmt.close();

	con.commit();

	stmt = con.createStatement("select * from person where address like %pattern");
	stmt.setString("%pattern", "%"); 
	cursor = stmt.fetch();
	if (cursor.size() != 3) {
	    System.err.println("stmt->fetch 5 returns " + cursor.size() + " instead of 3");
	}
	System.out.println("NAME\t\tSALARY\tRATING\tADDRESS");
	while ((p = (person)cursor.getNext()) != null) { 
	    System.out.println(p.name + "\t" + p.salary + "\t" + p.rating + "\t" + p.address);
	}
	stmt.close();

	stmt = con.createStatement("select * from person");
	cursor = stmt.fetch(true);
	if (cursor.size() != 3) {
	    System.err.println("stmt->fetch 6 returns " + cursor.size() + " instead of 3");
	}
	cursor.removeAll();
	stmt.close();

	con.close();
	System.out.println("*** CLI test sucessfully passed!");
    }
}


