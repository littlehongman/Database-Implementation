
#ifndef PAGE_RW_C
#define PAGE_RW_C

#include "MyDB_PageReaderWriter.h"
#include <utility>
#include "MyDB_PageRecIterator.h"

using namespace std;

void MyDB_PageReaderWriter :: clear () {
    // Clear the content on the page, and reset the offset to the beginning of the page

//    memset(this->pageHandle->getBytes(),0,this->pageSize); // Need to minus 1
    this->page->offsetToNextUnwritten = 0;

    // Tell me the buffer manager to write 0 back into the actual memory
    this->pageHandle->wroteBytes();

    this->setType(MyDB_PageType::RegularPage);

}


MyDB_RecordIteratorPtr MyDB_PageReaderWriter :: getIterator (const MyDB_RecordPtr& iterateIntoMe) {
    // create a new iterator, the data of record will be put into the location where the iterateIntoMe points to
    MyDB_RecordIteratorPtr pageIterator = make_shared <MyDB_PageRecIterator> (iterateIntoMe, this->pageHandle, this->page);

    return pageIterator;
}

void MyDB_PageReaderWriter :: setType (MyDB_PageType pageType) {
    this->page->pageType = pageType;
}

MyDB_PageType MyDB_PageReaderWriter :: getType () {
    return this->page->pageType;
}

bool MyDB_PageReaderWriter :: append (const MyDB_RecordPtr& appendMe) {
    size_t recordSize = appendMe->getBinarySize();

    if (this->getCurrentPageSize() + recordSize > this->pageSize) {
        return false;
    }

    // Get the current position of the page bytes
    unsigned pos = this->page->offsetToNextUnwritten ;

    // Write the record to the page
    void *next = appendMe->toBinary(&(this->page->bytes[pos]));
    this->pageHandle->wroteBytes(); // VERY IMPORTANT!!!

    // Update the offset
    this->page->offsetToNextUnwritten += (char*) next - &(this->page->bytes[pos]);


    return true;
}

size_t MyDB_PageReaderWriter ::getCurrentPageSize() {
    return sizeof(PageOverlay) + sizeof(char) * this->page->offsetToNextUnwritten;
}

//MyDB_PageReaderWriter::MyDB_PageReaderWriter(MyDB_BufferManagerPtr bufferManager, MyDB_TablePtr tablePtr, long i) {
//    // TODO: getPage or getPinnedPage?
//    this->pageHandle = bufferManager->getPage(tablePtr, i);
//
//    page = (PageOverlay*) this->pageHandle->getBytes();
//    page->offsetToNextUnwritten = 0;
//
//    this->pageSize = bufferManager->getPageSize();
//}

MyDB_PageReaderWriter::MyDB_PageReaderWriter(MyDB_PageHandle pageHandle, size_t pageSize) {
    this->pageHandle = std::move(pageHandle);

    // Cast the buffer into struct PageOverlay =>
    // first 4 bytes is unsigned
    // subsequent 4 bytes is PageType
    // rest of the memory will store records
    this->page = (PageOverlay*) this->pageHandle->getBytes();

    // Get the maximum pageSize, which is the buffer size
    this->pageSize = pageSize;
}

MyDB_PageReaderWriter::MyDB_PageReaderWriter() {
    this->page = nullptr;
    this->pageHandle = nullptr;

    this->pageSize = 0;
}

#endif
