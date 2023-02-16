//
// Created by Hung-Chieh Wu on 2/3/23.
//
#ifndef PAGE_RI_C
#define PAGE_RI_C


#include "MyDB_PageRecIterator.h"
#include <utility>

MyDB_PageRecIterator::MyDB_PageRecIterator(MyDB_RecordPtr rp, MyDB_PageHandle ph, PageOverlay* page) {
    this->recordPtr = std::move(rp);
    this->pageHandle = std::move(ph);
    this->bytesUsed = sizeof (PageOverlay);
    
    this->page = page;
}

void MyDB_PageRecIterator::getNext() {
    if (this->hasNext()) {
        /*
         * In fresh start, aka (this->bytesUsed == sizeof (PageOverlay)),
         * pos is actually: (PageOverlay*) this->pageHandle->getBytes()->bytes
         * */
        void* pos = this->bytesUsed + (char*) this->pageHandle->getBytes();
        void* newPos = this->recordPtr->fromBinary(pos);
        this->bytesUsed += (char*) newPos - (char*) pos;
    }
}

bool MyDB_PageRecIterator::hasNext() {
//    auto* pageOverlay = (PageOverlay*) this->pageHandle->getBytes();
    size_t pageCurrSize = sizeof(PageOverlay) + sizeof(char) * this->page->offsetToNextUnwritten;

    //cout << pageCurrSize << endl;
    if (pageCurrSize > this->bytesUsed) {
        return true;
    }
    return false;
}


#endif