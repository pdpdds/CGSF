open 'clitest';
create table persons (name    string, 
	              salary  int8, 
	              address string, 
                      weight  real8, 
	 	      subordinates array of reference to persons,
                      blob array of int1);
create index on persons.salary;
create hash on persons.name;
start server 'localhost:6100' 8
