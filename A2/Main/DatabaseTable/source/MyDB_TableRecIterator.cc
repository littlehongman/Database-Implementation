//
// Created by Hung-Chieh Wu on 2/3/23.
//

#ifndef TABLE_RI_C
#define TABLE_RI_C

#include "MyDB_TableRecIterator.h"

#include <utility>

using namespace std;

class MyDB_TableReaderWriter;

void MyDB_TableRecIterator::getNext() {
    if (this->hasNext()){
        return this->pageRecIterator->getNext();
    }
}

bool MyDB_TableRecIterator::hasNext() {
    // Current pageRecIterator has next
    while (this->pageId <= this->tablePtr->lastPage()) {

        if (this->pageRecIterator && this->pageRecIterator->hasNext()) {
            return true;
        }

        // Increment pageId
        this->pageId++;
        // IMPORTANT: We need to check if pageId is bigger than table numPage
        // because in tableRWPtr->operator[], it is allow to get idx > table numPage
        // If we don't check here, then we will end up it infinite loop
        if (this->pageId > this->tablePtr->lastPage())
            break;


        this->pageRecIterator = tableRWPtr->operator[]( this->pageId).getIterator(recordPtr);
    }

    // There is no next page
    return false;

//    else{
//        // Current pageRecIterator does not have next
//        // Check if there is next page
//        while (this->pageId < this->tablePtr->lastPage()) {
//            // There is next page
//            // Get next page
//            this->pageId++;
//            //cout << pageId << endl;
//            this->pageRecIterator = tableRWPtr->operator[]( this->pageId).getIterator(recordPtr);
//
//            if(this->pageRecIterator->hasNext()){
//                return true;
//            }
//        }
//    }

}

MyDB_TableRecIterator::MyDB_TableRecIterator(MyDB_TablePtr tablePtr, MyDB_TableReaderWriter *tableRWPtr, MyDB_RecordPtr recordPtr) {
    this->tableRWPtr = tableRWPtr;
    this->pageRecIterator = nullptr;

    this->tablePtr = std::move(tablePtr);
    this->recordPtr = std::move(recordPtr);

    this->pageId = -1;
}

#endif

