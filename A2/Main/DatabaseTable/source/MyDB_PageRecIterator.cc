//
// Created by Hung-Chieh Wu on 2/3/23.
//
#ifndef PAGE_RI_C
#define PAGE_RI_C


#include "MyDB_PageRecIterator.h"

MyDB_PageRecIterator::MyDB_PageRecIterator(MyDB_RecordPtr rp, MyDB_PageHandle ph) {
    this->recordPtr = rp;
    this->pageHandle = ph;
    this->bytesUsed = sizeof (PageOverlay);
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
    auto* pageOverlay = (PageOverlay*) this->pageHandle->getBytes();
    if (pageOverlay->offsetToNextUnwritten > this->bytesUsed) {
        return true;
    }
    return false;
}


#endif