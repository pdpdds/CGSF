#!/usr/local/bin/ruby

require 'fastdb'
include FastDB

class Person 
    def initialize(name, salary, address, rating, pets, subordinates)
        @name = name
        @salary = salary
        @address = address
        @rating = rating
        @pets = pets
        @subordinates = subordinates
    end
    
    attr_reader :name, :salary, :address, :rating, :pets, :subordinates;
    attr_writer :salary, :address, :rating, :pets, :subordinates;
end

con = Connection.new
con.open("localhost", 6100)
john = con.insert(Person.new("John Smith", 80000, "1 Guildhall St., Cambridge CB2 3NH, UK", 3.84, ["dog", "cat"], []))
joe = con.insert(Person.new("Joe Cooker", 50000, "Outlook drive, 15/3", 1.76, ["snake"], [john]))
hugo = con.insert(Person.new("Hugo Grant", 65000, "London, Baker street,12", 2.14, ["canary", "goldfish"], []))
con.commit()

stmt = con.createStatement("select * from Person where salary > %salary order by salary")
stmt["salary"] = 65000
cursor = stmt.fetch()
if cursor.size != 1
    raise "stmt->fetch 1 returns " + cursor.size.to_s + " instead of 1"
end
puts("NAME\t\tSALARY\tRATING")
for p in cursor
    puts(p.name + "\t" + p.salary.to_s + "\t" + p.rating.to_s)
end

stmt["salary"] = 50000
cursor = stmt.fetch()
if cursor.size != 2
    raise "stmt->fetch 2 returns " + cursor.size.to_s + " instead of 2"
end
stmt.close()

stmt = con.createStatement("select * from Person where current = %ref")
stmt["ref"] = joe
cursor = stmt.fetch
if cursor.size != 1
    raise "stmt->fetch 3 returns " + cursor.size.to_s + " instead of 1"
end
p = cursor.first
str=""
str << "Object oid=" << cursor.ref.to_s << ", name=" << p.name << ", address=" << p.address 
str << ", rating=" << p.rating.to_s << ", salary=" << p.salary.to_s << ", pets=["
for pet in p.pets
     str << pet << " "
end
str << "], subordinates=["
stmt2 = con.createStatement("select * from Person where current = %ref")
for s in p.subordinates
    stmt2["ref"] = s
    str << stmt2.fetch.first.name << " " 
end
str << "]";
puts(str)
stmt.close()
stmt2.close()

stmt = con.createStatement("select * from Person where rating > %rating and salary between %min and %max");
stmt["rating"] = 2.0
stmt["min"] = 50000
stmt["max"] = 100000
cursor = stmt.fetch(true)
if cursor.size != 2
    raise "stmt->fetch 4 returns " + cursor.size.to_s + " instead of 2"
end
p = cursor.last
puts("NAME\t\tSALARY\tRATING\tADDRESS")
while p != nil 
    puts(p.name+"\t"+p.salary.to_s+"\t"+p.rating.to_s+"\t"+p.address)
    p.salary = (p.salary * 1.1).to_i
    cursor.update
    p = cursor.prev
end
stmt.close()

con.commit()

stmt = con.createStatement("select * from Person where address like %pattern")
stmt["pattern"] = "%"
cursor = stmt.fetch()
if cursor.size != 3
    raise "stmt->fetch 5 returns " + cursor.size.to_s + " instead of 3"
end
puts("NAME\t\tSALARY\tRATING\tADDRESS")
for p in cursor
    puts(p.name + "\t" + p.salary.to_s + "\t" + p.rating.to_s + "\t" + p.address);
end
stmt.close()

stmt = con.createStatement("select * from Person")
cursor = stmt.fetch(true)
if cursor.size != 3
    raise "stmt->fetch 6 returns " + cursor.size.to_s + " instead of 3"
end
cursor.removeAll()
stmt.close()

con.close()
puts("*** CLI test sucessfully passed!")


