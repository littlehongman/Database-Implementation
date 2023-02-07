
#ifndef TABLE_RW_C
#define TABLE_RW_C

#include <fstream>
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_TableRecIterator.h"

using namespace std;


MyDB_PageReaderWriter MyDB_TableReaderWriter :: operator [] (size_t id) {
    if (id <= this->lastPageId){
        // make a pageHandle
        MyDB_PageHandle pageHandle = this->bufferManagerPtr->getPinnedPage(this->tablePtr, id);

        // make a pageReadWriter
        this->pageRW = make_shared<MyDB_PageReaderWriter>(pageHandle, this->bufferManagerPtr->getPageSize());

        return *(this->pageRW);
    }

    else {
        // TODO: Loop through all of the other pages in the file, up to and including i, and make sure to initialize them properly (probably this involves setting the used bytes to zero, for example). Make sure to update the table size appropiately. Then return the last one.

        return MyDB_PageReaderWriter();
    }
}

MyDB_RecordPtr MyDB_TableReaderWriter :: getEmptyRecord () {
    MyDB_RecordPtr emptyRecordPtr = make_shared<MyDB_Record>(this->tablePtr->getSchema());

    return emptyRecordPtr;
}

MyDB_PageReaderWriter MyDB_TableReaderWriter :: last () {
	return *(this->lastPageRW);
}


void MyDB_TableReaderWriter :: append (MyDB_RecordPtr appendMe) {
    bool canAppend = this->lastPageRW->append(appendMe);

    if (!canAppend) {// Create a new page
        // increment pageId
        this->lastPageId++;

        // Set table size
        this->tablePtr->setLastPage (lastPageId);

        // make new pageHandle
        MyDB_PageHandle newPageHandle = this->bufferManagerPtr->getPinnedPage(this->tablePtr, this->lastPageId);

        // Create pageReadWriter
        this->lastPageRW = make_shared<MyDB_PageReaderWriter>(newPageHandle, this->bufferManagerPtr->getPageSize());
        this->lastPageRW->append(appendMe);
    }

    return;

}

void MyDB_TableReaderWriter :: loadFromTextFile (string filename) {
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
        while (infile.good()){
            // Get the line from the file
            getline(infile, line);

            // parse the contents of this record from the given string
            recordPtr->fromString(line);

            // append the record to the table
            this->append(recordPtr);
        }

        infile.close();
    }

    return;
}

MyDB_RecordIteratorPtr MyDB_TableReaderWriter :: getIterator (MyDB_RecordPtr iterateIntoMe) {

	return make_shared <MyDB_TableRecIterator> (this->tablePtr, this, iterateIntoMe);
}

void MyDB_TableReaderWriter :: writeIntoTextFile (string filename) {

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

    return;
}

MyDB_TableReaderWriter :: MyDB_TableReaderWriter (MyDB_TablePtr forMe, MyDB_BufferManagerPtr myBuffer) {
    this->bufferManagerPtr = myBuffer;
    this->tablePtr = forMe;

    this->lastPageRW = nullptr;
    this->pageRW = nullptr;

    this->lastPageId = forMe->lastPage(); // Initially should be -1
}

#endif

