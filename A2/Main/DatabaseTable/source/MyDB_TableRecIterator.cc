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

    cout << " There is no next page" << endl;
    return;

}

bool MyDB_TableRecIterator::hasNext() {
    // Current pageRecIterator has next
    if (this->pageRecIterator->hasNext()) {
        return true;
    }

    else{
        // Current pageRecIterator does not have next
        // Check if there is next page
        if (this->pageId < this->tablePtr->lastPage()) {
            // There is next page
            // Get next page
            this->pageId++;
            this->pageRecIterator = tableRWPtr->operator[]( this->pageId).getIterator(recordPtr);

            return true;
        }
        else{
            // There is no next page
            cout << " There is no next page" << endl;
            return false;
        }
    }
}

MyDB_TableRecIterator::MyDB_TableRecIterator(MyDB_TablePtr tablePtr, MyDB_TableReaderWriter *tableRWPtr, MyDB_RecordPtr recordPtr) {
    this->tableRWPtr = tableRWPtr;
    this->pageRecIterator = tableRWPtr->operator[](0).getIterator(recordPtr);

    this->tablePtr = tablePtr;
    this->recordPtr = recordPtr;

    this->pageId = 0;
}

#endif

