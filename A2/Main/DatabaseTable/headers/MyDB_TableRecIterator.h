//
// Created by Hung-Chieh Wu on 2/3/23.
//

#ifndef A1_MYDB_TABLERECITERATOR_H
#define A1_MYDB_TABLERECITERATOR_H


#include "MyDB_RecordIterator.h"
#include "MyDB_PageRecIterator.h"
#include "MyDB_TableReaderWriter.h"

class MyDB_TableReaderWriter;
class MyDB_TableRecIterator : public MyDB_RecordIterator {

public:
    MyDB_TableRecIterator(MyDB_TablePtr tablePtr, MyDB_TableReaderWriter *tableRWPtr, MyDB_RecordPtr recordPtr);

    void getNext() override;

    bool hasNext() override;

private:
    // Use to retrieve pageReadWriter and get pageRecIterator
    MyDB_TableReaderWriter *tableRWPtr;

    // Current pageRecIterator
    MyDB_RecordIteratorPtr pageRecIterator;

    // Use to check table size
    MyDB_TablePtr tablePtr;

    // The record ptr to read/write data
    MyDB_RecordPtr recordPtr;

    // Keep track of pageId
    long pageId;
};



#endif
