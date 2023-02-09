
#ifndef TABLE_RW_C
#define TABLE_RW_C

#include <fstream>
#include <utility>
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_TableRecIterator.h"

using namespace std;


MyDB_PageReaderWriter MyDB_TableReaderWriter :: operator [] (size_t id) {
    MyDB_PageReaderWriterPtr temp = nullptr;

    // When we initialize the table, the table have the size of table
    if (id <= this->lastPageId){
        // make a pageHandle
        MyDB_PageHandle pageHandle = this->bufferManagerPtr->getPage(this->tablePtr, id);

        // make a pageReadWriter
        temp = make_shared<MyDB_PageReaderWriter>(pageHandle, this->bufferManagerPtr->getPageSize());

        return *(temp);
    }

    while (this->lastPageId < id){ // Although the id > lastPageId, the table should have the ability to expand without any limit.
        // TODO: Loop through all of the other pages in the file, up to and including i,
        // Make sure to initialize them properly (probably this involves setting the used bytes to zero, for example).
        // Make sure to update the table size appropiately. Then return the last one.

        // increment lastPageId
        this->lastPageId += 1;

        // Update table size
        this->tablePtr->setLastPage(this->lastPageId);

        MyDB_PageHandle pageHandle = this->bufferManagerPtr->getPage(this->tablePtr, this->lastPageId);
        temp = make_shared<MyDB_PageReaderWriter>(pageHandle, this->bufferManagerPtr->getPageSize());

        // Because this page don't actually exist in table, clear it for safety
        temp->clear();
    }

    return *(temp);
}

MyDB_RecordPtr MyDB_TableReaderWriter :: getEmptyRecord () {
    MyDB_RecordPtr emptyRecordPtr = make_shared<MyDB_Record>(this->tablePtr->getSchema());

    return emptyRecordPtr;
}

MyDB_PageReaderWriter MyDB_TableReaderWriter :: last () {
	return *(this->lastPageRW);
}


void MyDB_TableReaderWriter :: append (const MyDB_RecordPtr& appendMe) {
    if (this->lastPageRW && this->lastPageRW->append(appendMe)){
        return;
    }

    // Create a new page
    // increment pageId
    this->lastPageId++;

    // Set table size
    this->tablePtr->setLastPage (lastPageId);

    // make new pageHandle
    MyDB_PageHandle newPageHandle = this->bufferManagerPtr->getPinnedPage(this->tablePtr, this->lastPageId);


    // Create pageReadWriter
    this->lastPageRW = make_shared<MyDB_PageReaderWriter>(newPageHandle, this->bufferManagerPtr->getPageSize());

    // Clear page
    this->lastPageRW->clear(); // VERY IMPORTANT: set offset = 0

    this->lastPageRW->append(appendMe);

}

void MyDB_TableReaderWriter :: loadFromTextFile (const string& filename) {
    // Set the last page to 0
    // this->tablePtr->lastPage() = -1 if there has never been anything written to the table
    this->tablePtr->setLastPage (0);

    // Iteratively read data from the text file with a record ptr and append it to the table
    MyDB_RecordPtr recordPtr = this->getEmptyRecord();

    // Use to store each line of the text file
    string line;

    // Open the file
    ifstream infile (filename);

    if (infile.is_open()){
        // Get the line from the file
        while (getline(infile, line)){

            // parse the contents of this record from the given string
            recordPtr->fromString(line);

            // append the record to the table
            this->append(recordPtr);
        }

        infile.close();
    }

}

MyDB_RecordIteratorPtr MyDB_TableReaderWriter :: getIterator (const MyDB_RecordPtr& iterateIntoMe) {

	return make_shared <MyDB_TableRecIterator> (this->tablePtr, this, iterateIntoMe);
}

void MyDB_TableReaderWriter :: writeIntoTextFile (const string& filename) {

    // Iteratively read data from the database with a record ptr and append it to the text file
    MyDB_RecordPtr recordPtr = this->getEmptyRecord();

    // Initialize an iterator to iterate through whole table
    MyDB_RecordIteratorPtr tableRecIterator = this->getIterator(recordPtr);

    // Open the file
    ofstream outfile (filename);

    if (outfile.is_open()){
        while (tableRecIterator->hasNext()){
            // Write data from the page to record object
            tableRecIterator->getNext();

            outfile << tableRecIterator;
        }

        outfile.close();
    }

}

MyDB_TableReaderWriter :: MyDB_TableReaderWriter (const MyDB_TablePtr& forMe, MyDB_BufferManagerPtr myBuffer) {
    this->bufferManagerPtr = std::move(myBuffer);
    this->tablePtr = forMe;

    this->lastPageRW = nullptr;

    // Initially, tablePtr numPage is set to -1
    // However, after when we call initialization, the table numPage will change
    this->lastPageId = forMe->lastPage();
    if (this->lastPageId != -1){
        MyDB_PageHandle pageHandle = this->bufferManagerPtr->getPage(this->tablePtr, this->lastPageId);
        this->lastPageRW = make_shared<MyDB_PageReaderWriter>(pageHandle, this->bufferManagerPtr->getPageSize());
    }
}

#endif

