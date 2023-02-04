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
    MyDB_PageRecIterator(MyDB_RecordPtr rp, MyDB_PageHandle ph);

    void getNext() override;

    bool hasNext() override;
private:

    MyDB_RecordPtr recordPtr;
    MyDB_PageHandle pageHandle;
    unsigned bytesUsed;

};


#endif
