//
// Created by Hung-Chieh Wu on 2/3/23.
//

#ifndef A1_MYDB_TABLERECITERATOR_H
#define A1_MYDB_TABLERECITERATOR_H


#include "MyDB_RecordIterator.h"
#include "MyDB_PageRecIterator.h"

class MyDB_TableRecIterator : public MyDB_RecordIterator {

public:
    MyDB_TableRecIterator();

    void getNext() override;

    bool hasNext() override;
private:
    MyDB_PageRecIterator pageRecIterator;
    MyDB_TableReaderWriter *tableRW;
    long pageId;

};



#endif
