
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include "MyDB_PageHandle.h"

void *MyDB_PageHandleBase :: getBytes () {
    auto bufferPtr = this->pagePtr->getBufferPtr();

    if (bufferPtr != nullptr) {
        // TODO: update the LRU order

        // Return the page from buffer
        return bufferPtr;
    }
    else{ // Read the page from disk
        if (this->bufferManagerPtr->isFull()){
            // TODO: evict a page from the LRU
            // TODO: update the LRU order
        }
        else{
            // TODO: allocate a new chunk of memory
            // TODO: update the LRU order
        }
    }


	return nullptr;
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
            // if the page is anonymous, then we delete it
            delete this->pagePtr;
        }

        return;
    }

}

MyDB_PageHandleBase :: MyDB_PageHandleBase (Page* pagePtr) {
    pagePtr = pagePtr;
    pagePtr->increaseReferenceCount();
}

#endif

