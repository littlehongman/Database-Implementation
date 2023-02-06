//
// Created by Hung-Chieh Wu on 2/3/23.
//

#ifndef TABLE_RI_C
#define TABLE_RI_C

#include "MyDB_TableRecIterator.h"


#endif

void MyDB_TableRecIterator::getNext() {

}

bool MyDB_TableRecIterator::hasNext() {
    // Current pageRecIterator has next
    if (this->pageRecIterator.hasNext()) {
        return true;
    }

    else{
        // Current pageRecIterator does not have next
        // Check if there is next page
        if (this->pageId < this->tableRW->lastPage()) {
            // There is next page
            // Get next page
            this->pageId++;
            this->pageRecIterator = MyDB_PageRecIterator();

            return true;
        }
        else{
            // There is no next page
            return false;
        }
    }
}
