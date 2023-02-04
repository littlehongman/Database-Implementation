//
// Created by Hung-Chieh Wu on 2/3/23.
//

#ifndef A1_MYDB_PAGERECITERATOR_H
#define A1_MYDB_PAGERECITERATOR_H

#include "MyDB_RecordIterator.h"
#include "MyDB_Record.h"
#include "MyDB_PageHandle.h"
#include "MyDB_PageReaderWriter.h"

class MyDB_PageRecIterator : public MyDB_RecordIterator {

public:
    MyDB_PageRecIterator(MyDB_RecordPtr myRecord, MyDB_PageReaderWriter* myPageReaderWriter);

    void getNext() override;

    bool hasNext() override;
private:


};


#endif
