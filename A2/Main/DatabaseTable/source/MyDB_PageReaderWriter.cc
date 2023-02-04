
#ifndef PAGE_RW_C
#define PAGE_RW_C

#include "MyDB_PageReaderWriter.h"

void MyDB_PageReaderWriter :: clear () {
}


MyDB_RecordIteratorPtr MyDB_PageReaderWriter :: getIterator (MyDB_RecordPtr) {
	return nullptr;
}

void MyDB_PageReaderWriter :: setType (MyDB_PageType pageType) {
    this->page->pageType = pageType;
}

MyDB_PageType MyDB_PageReaderWriter :: getType () {
    return this->page->pageType;
}

bool MyDB_PageReaderWriter :: append (MyDB_RecordPtr recordPtr) {
    size_t recordSize = recordPtr->getBinarySize();
    if (this->page->offsetToNextUnwritten + recordSize > this->pageSize) {
        return false;
    }

    // Get the current position of the page bytes
    auto pos = this->page->bytes[this->page->offsetToNextUnwritten];

    // Write the record to the page
    recordPtr->toBinary(&(this->page[pos]));

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
