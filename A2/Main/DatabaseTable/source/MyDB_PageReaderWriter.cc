
#ifndef PAGE_RW_C
#define PAGE_RW_C

#include "MyDB_PageReaderWriter.h"
#include "MyDB_PageRecIterator.h"

using namespace std;

void MyDB_PageReaderWriter :: clear () {
    // clear the content on the page, and reset the offset to the beginning of the page
    memset(this->pageHandle->getBytes(),0,this->page->offsetToNextUnwritten - 1); // Need to minus 1
    this->page->offsetToNextUnwritten = 0;

    this->setType(MyDB_PageType::RegularPage);

    return;
}


MyDB_RecordIteratorPtr MyDB_PageReaderWriter :: getIterator (MyDB_RecordPtr iterateIntoMe) {
    // create a new iterator, the data of record will be put into th location where the iterateIntoMe points to
    MyDB_RecordIteratorPtr recordIterator = make_shared <MyDB_PageRecIterator> (iterateIntoMe, this->pageHandle);

    return recordIterator;
}

void MyDB_PageReaderWriter :: setType (MyDB_PageType pageType) {
    this->page->pageType = pageType;
}

MyDB_PageType MyDB_PageReaderWriter :: getType () {
    return this->page->pageType;
}

bool MyDB_PageReaderWriter :: append (MyDB_RecordPtr appendMe) {
    size_t recordSize = appendMe->getBinarySize();
    if (this->page->offsetToNextUnwritten + recordSize > this->pageSize) {
        return false;
    }

    // Get the current position of the page bytes
    auto pos = this->page->bytes[this->page->offsetToNextUnwritten];

    // Write the record to the page
    appendMe->toBinary(&(this->page[pos]));

    // Update the offset
    this->page->offsetToNextUnwritten += recordSize;


    return true;
}

MyDB_PageReaderWriter::MyDB_PageReaderWriter(MyDB_BufferManagerPtr bufferManager, MyDB_TablePtr tablePtr, long i) {
    this->pageHandle = bufferManager->getPage(tablePtr, i);

    page = (PageOverlay*) this->pageHandle->getBytes();
    page->offsetToNextUnwritten = 0;

    this->pageSize = bufferManager->getPageSize();
}

#endif
