
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include "MyDB_PageHandle.h"

class MyDB_BufferManager;

void *MyDB_PageHandleBase :: getBytes () {
    char* bufferPtr = this->pagePtr->getBufferPtr();

    if (bufferPtr == nullptr) { // Read the page from disk

        // allocate a new chunk of memory
        bufferPtr = this->bufferManagerPtr->allocateChunk();

        // set the buffer pointer to the page
        this->pagePtr->setBufferPtr(bufferPtr);

        // Read the page from disk
        this->bufferManagerPtr->readDisk(this->pagePtr);

    }

    // Use pin to update the LRU order
    if (!this->pagePtr->getPinned()){
        this->bufferManagerPtr->removeFromLRU(this->pagePtr);
    }
    else {
        this->bufferManagerPtr->updateLRU(this->pagePtr);
    }

    return bufferPtr;

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
            if (this->pagePtr->getPinned()){
                this->pagePtr->unpin();
                this->bufferManagerPtr->updateLRU(this->pagePtr);
            }

        }
        else{
            // if the page is anonymous, memory is automatically returned
            this->bufferManagerPtr->reclaimTempSlot(this->pagePtr->getSlot());

//            // we delete it
//            delete this->pagePtr;
            // Because LRU may still point to anonymous page, so we do not need to delete it here
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

