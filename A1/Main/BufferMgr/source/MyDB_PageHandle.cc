
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include "MyDB_PageHandle.h"

class MyDB_BufferManager;

void *MyDB_PageHandleBase :: getBytes () {
    char* bufferPtr = this->pagePtr->getBufferPtr();

    if (bufferPtr != nullptr) {
        // TODO: update the LRU order

        // Return the page from buffer
        return bufferPtr;
    }
    else{ // Read the page from disk

        // allocate a new chunk of memory
        bufferPtr = this->bufferManagerPtr->allocateChunk();

        // set the buffer pointer to the page
        this->pagePtr->setBufferPtr(bufferPtr);

        // Read the page from disk
        this->bufferManagerPtr->readDisk(this->pagePtr);

        // TODO: update the LRU order

        return bufferPtr;
    }

}

void MyDB_PageHandleBase :: wroteBytes () {
    this->pagePtr->markDirty();
}

Page* MyDB_PageHandleBase :: getPagePtr() {
    return this->pagePtr;
}

MyDB_PageHandleBase :: ~MyDB_PageHandleBase () {
    this->pagePtr->decreaseReferenceCount();

    if (this->pagePtr->getReferenceCount() == 0) {
        // if the page is not anonymous, then we push it to the LRU
        if (this->pagePtr->getTablePtr() != nullptr) {
            this->bufferManagerPtr->insertLRU(this->pagePtr);
            this->pagePtr->unpin();
        }
        else{
            // if the page is anonymous, memory is automatically returned
            this->bufferManagerPtr->reclaimChunk(this->pagePtr->getBufferPtr());

            // we delete it
            delete this->pagePtr;
        }

        return;
    }

}

MyDB_PageHandleBase :: MyDB_PageHandleBase (Page* pagePtr, MyDB_BufferManager *bufferManagerPtr) {
    this->pagePtr = pagePtr;
    this->bufferManagerPtr = bufferManagerPtr;

    this->pagePtr->increaseReferenceCount();
}

#endif

