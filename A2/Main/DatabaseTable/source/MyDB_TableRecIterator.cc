//
// Created by Hung-Chieh Wu on 2/3/23.
//

#ifndef TABLE_RI_C
#define TABLE_RI_C

#include "MyDB_TableRecIterator.h"

using namespace std;

class MyDB_TableReaderWriter;

void MyDB_TableRecIterator::getNext() {
    if (this->hasNext()){
        return this->pageRecIterator->getNext();
    }

    //cout << " There is no next page" << endl;
    return;

}

bool MyDB_TableRecIterator::hasNext() {
    // Current pageRecIterator has next
    while (this->pageId <= this->tablePtr->lastPage()) {
        if (this->pageRecIterator && this->pageRecIterator->hasNext()) {
            return true;
        }

        // There is next page
        // Get next page
        this->pageId++;

        if (this->pageId == 33){
            cout << "test";
        }

        //cout << pageId << endl;
        this->pageRecIterator = tableRWPtr->operator[]( this->pageId).getIterator(recordPtr);
    }
    // There is no next page
    //cout << " There is no next page" << endl;
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

    this->tablePtr = tablePtr;
    this->recordPtr = recordPtr;

    this->pageId = -1;
}

#endif

