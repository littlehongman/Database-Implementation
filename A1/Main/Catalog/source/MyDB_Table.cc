
#ifndef TABLE_C
#define TABLE_C

#include "MyDB_Table.h"
#include <sys/stat.h>

MyDB_Table :: MyDB_Table (string name, string storageLocIn) {
	tableName = name;
	storageLoc = storageLocIn;

    string dirname = tableName;

    // Make directory to put file
    mkdir(dirname.c_str(),0777);

}

MyDB_Table :: ~MyDB_Table () {}

string &MyDB_Table :: getName () {
	return tableName;
}

string &MyDB_Table :: getStorageLoc () {
	return storageLoc;
}

#endif

