This directory contains database pools for Oracle's occi, MySQL's connector C++ and SQL Server ODBC C++. A good place to start are the examples. Feel free to use and modify this code in anyway you see fit, without ever acknowledging me. I am not responsible for any natural or un-natural deaths resulting through the use or misuse of this code.

http://codediaries.blogspot.com.


======================================
A brief overview of the files
======================================

directory=. 
-----------
CommonDatabasePool.h, CommonDatabasePool.cpp - Implement the actual database pool. This contains the std::map that is used to store the connections and the logistics such as semaphores, 	mutexes, threads etc required to keep the pool. alive. It is basically a template that gets and returns any kind of 'connection' or class.

CrossHelper.h and CrossHelper.cpp - These files map a generic function name to the platform pecific function. For instance XSemTimedWait will map to WaitForSingleObject on 	Windows and sem_timedwait on Linux.


directory=./mysql 
-----------------
MySQLPool.h, MySQLPool.cpp -The MySQL specific implementation. It inherits from the CommonDatabasePool template. Functions in the CommonDatabasePool will call upon the implemented pure virtual functions in this class for database specific calls such as creating a connection and deleting one.


directory=./oracle
------------------
OraclePool.h, OraclePool.cpp - The oracle versions of the pool.


directory=./sqlserver
------------------
SQLSvrPool.h, SQLSvrPool.cpp - The SQL Server versions. These slightly differ from oracle and mysql because they use HANDLES and not pointers.



directory=./examples
--------------------
ExampleOracle.cpp, ExampleMySQL.cpp, ExampleSQLSvr.cpp - The examples, showing how the pools can be utilized.

makefile.oracle, makefile.mysql, makefile.sqlsvr - The makefiles that can be used to build the examples. These have to be modified to reflect your local installation directories.



======================================
Requirements
======================================
Oracle: The C/C++ header files, libs and dlls can be found under your oracle client install. Usually it's an "admin" install with the development environment.

MySQL: The MySQL Connector C++ header files and libs come with the MySQL Connector C++ package. However it neads some of the MySQL database runtime dlls to actually run the example.

SQL Server: Should not require anything, but you can install SQL Server Express edition if you find dll's etc missing.


======================================
Some other possibly useful information
======================================
The windows version uses WaitForSingleObjects to wait for semaphores and threads. This may be a problem if you have threads creating windows trhough DDE and CoInitialize. I suggest you replace WaitForSingleObject with MsgWaitForMultipleObjects and recompile to be safe.



======================================
Useful site information
======================================
http://codediaries.blogspot.com

Oracle article with more information
http://codediaries.blogspot.com/2009/07/oracle-c-oci-database-pool.html

MySQL article with more information
http://codediaries.blogspot.com/2009/07/mysql-connector-c-database-pool.html



