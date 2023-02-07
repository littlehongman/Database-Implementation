
#ifndef TABLE_RW_H
#define TABLE_RW_H

#include <memory>
#include "MyDB_Record.h"
#include "MyDB_Table.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"

// create a smart pointer for the catalog
using namespace std;
class MyDB_PageReaderWriter;
class MyDB_TableReaderWriter;
typedef shared_ptr <MyDB_TableReaderWriter> MyDB_TableReaderWriterPtr;

class MyDB_TableReaderWriter {

public:

	// ANYTHING ELSE YOU NEED HERE

	// create a table reader/writer for the specified table, using the specified
	// buffer manager
	MyDB_TableReaderWriter (MyDB_TablePtr forMe, MyDB_BufferManagerPtr myBuffer);

	// gets an empty record from this table
	MyDB_RecordPtr getEmptyRecord ();

	// append a record to the table
	void append (MyDB_RecordPtr appendMe);

	// return an iterator over this table... each time returnVal->next () is
	// called, the resulting record will be placed into the record pointed to
	// by iterateIntoMe
	MyDB_RecordIteratorPtr getIterator (MyDB_RecordPtr iterateIntoMe);

	// load a text file into this table... overwrites the current contents
	void loadFromTextFile (string fromMe);

	// dump the contents of this table into a text file
	void writeIntoTextFile (string toMe);

	// access the i^th page in this file
	MyDB_PageReaderWriter operator [] (size_t id);

    // access the last page in the file
    MyDB_PageReaderWriter last ();

private:
    MyDB_BufferManagerPtr bufferManagerPtr;
    MyDB_TablePtr tablePtr;

    // Use this when append records
    MyDB_PageReaderWriterPtr lastPageRW;

    // Use this when indexing (operator [])
    MyDB_PageReaderWriterPtr pageRW;

    size_t lastPageId;
};

#endif
