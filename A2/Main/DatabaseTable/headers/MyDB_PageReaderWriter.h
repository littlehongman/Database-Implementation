
#ifndef PAGE_RW_H
#define PAGE_RW_H

#include "MyDB_PageType.h"
#include "MyDB_BufferManager.h"
#include "MyDB_Record.h"
#include "MyDB_RecordIterator.h"

struct PageOverlay{
    unsigned offsetToNextUnwritten;
    MyDB_PageType pageType;

    char bytes[0]; // this is where to store the actual data
};

class MyDB_PageReaderWriter;
typedef shared_ptr <MyDB_PageReaderWriter> MyDB_PageReaderWriterPtr;

class MyDB_PageReaderWriter {

public:

	// ANY OTHER METHODS YOU WANT HERE
//    MyDB_PageReaderWriter(MyDB_BufferManagerPtr bufferManager, MyDB_TablePtr table, long i);

    MyDB_PageReaderWriter(MyDB_PageHandle pageHandle, size_t pageSize);

    // TODO: temp empty constructor
    MyDB_PageReaderWriter();

	// empties out the contents of this page, so that it has no records in it
	// the type of the page is set to MyDB_PageType :: RegularPage
	void clear ();	

	// return an iterator over this page... each time returnVal->next () is
	// called, the resulting record will be placed into the record pointed to
	// by iterateIntoMe
	MyDB_RecordIteratorPtr getIterator (MyDB_RecordPtr iterateIntoMe);

	// appends a record to this page... return false is the append fails because
	// there is not enough space on the page; otherwise, return true
	bool append (MyDB_RecordPtr appendMe);

	// gets the type of this page... this is just a value from an enumeration
	// that is stored within the page
	MyDB_PageType getType ();

	// sets the type of the page
	void setType (MyDB_PageType toMe);

    size_t getCurrentPageSize();
	
private:
    // Use this to obtain correct memory to read record, or write record
    PageOverlay* page;

    MyDB_PageHandle pageHandle;

    // Check if there is enough space to write record
    // Often used with recordPtr -> getBinarySize ()
    size_t pageSize;
};

#endif
