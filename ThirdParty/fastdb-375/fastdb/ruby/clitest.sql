open 'clitest';
create table Person(name    string, 
	            salary  int8, 
	            address string, 
                    rating  real8, 
                    pets array of string,
	 	    subordinates array of reference to Person);
create index on Person.salary;
create hash on Person.name;
start server 'localhost:6100' 4
