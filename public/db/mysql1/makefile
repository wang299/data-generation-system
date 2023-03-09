#########################################
MYSQLLIBS = -lmysqlclient

#CFLAGS = -O2

# mysql¿¿¿¿¿¿¿
MYSQLINCL = -I/usr/mysql/include  -I/project/public/db/mysql

# mysql¿¿¿¿¿¿¿
MYSQLLIB = -L/usr/mysql/lib

CFLAGS = -g -Wno-write-strings 

all:	createtable inserttable counttable updatetable deletetable selecttable filetoblob blobtofile selecttable1

createtable: createtable.cpp _mysql.h _mysql.cpp
	g++ $(CFLAGS) -o createtable createtable.cpp $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) _mysql.cpp -lm -lc

inserttable: inserttable.cpp _mysql.h _mysql.cpp
	g++ $(CFLAGS) -o inserttable inserttable.cpp $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) _mysql.cpp -lm -lc

selecttable: selecttable.cpp _mysql.h _mysql.cpp
	g++ $(CFLAGS) -o selecttable selecttable.cpp $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) _mysql.cpp -lm -lc

selecttable1: selecttable1.cpp _mysql.h _mysql.cpp
	g++ $(CFLAGS) -o selecttable1 selecttable1.cpp $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) _mysql.cpp -lm -lc

counttable: counttable.cpp _mysql.h _mysql.cpp
	g++ $(CFLAGS) -o counttable counttable.cpp $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) _mysql.cpp -lm -lc

updatetable: updatetable.cpp _mysql.h _mysql.cpp
	g++ $(CFLAGS) -o updatetable updatetable.cpp $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) _mysql.cpp -lm -lc

deletetable: deletetable.cpp _mysql.h _mysql.cpp
	g++ $(CFLAGS) -o deletetable deletetable.cpp $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) _mysql.cpp -lm -lc

filetoblob: filetoblob.cpp _mysql.h _mysql.cpp
	g++ $(CFLAGS) -o filetoblob filetoblob.cpp $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) _mysql.cpp -lm -lc

blobtofile: blobtofile.cpp _mysql.h _mysql.cpp
	g++ $(CFLAGS) -o blobtofile blobtofile.cpp $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) _mysql.cpp -lm -lc


clean:	
	rm -rf createtable inserttable selecttable counttable updatetable deletetable filetoblob blobtofile
