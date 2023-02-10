
#ifndef SORT_C
#define SORT_C

#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableRecIterator.h"
#include "MyDB_TableRecIteratorAlt.h"
#include "MyDB_TableReaderWriter.h"
#include "Sorting.h"

using namespace std;

void mergeIntoFile(MyDB_TableReaderWriter &, vector<MyDB_RecordIteratorAltPtr> &, function<bool()>, MyDB_RecordPtr,
                   MyDB_RecordPtr) {
}


// Literally "Merge Two Sorted Lists" from LeetCode, but merges records into a vector of pages
vector<MyDB_PageReaderWriter>
mergeIntoList(MyDB_BufferManagerPtr parent, MyDB_RecordIteratorAltPtr leftIter,
              MyDB_RecordIteratorAltPtr rightIter, function<bool()> comparator, MyDB_RecordPtr lhs,
              MyDB_RecordPtr rhs) {
    vector<MyDB_PageReaderWriter> sorted_list;
    MyDB_PageReaderWriter sorted_page(*parent);
    bool has_record = false;

    bool left_has_next = leftIter->advance();
    bool right_has_next = rightIter->advance();

    while (left_has_next && right_has_next) {
        MyDB_RecordPtr next;
        leftIter->getCurrent(lhs);
        rightIter->getCurrent(rhs);
        if (comparator()) {
            next = lhs;
            left_has_next = leftIter->advance();
        } else {
            next = rhs;
            right_has_next = rightIter->advance();
        }
        appendToPage(parent, sorted_list, sorted_page, next);
    }

    while (left_has_next) {
        leftIter->getCurrent(lhs);
        appendToPage(parent, sorted_list, sorted_page, lhs);
        left_has_next = leftIter->advance();
    }

    while (right_has_next) {
        rightIter->getCurrent(rhs);
        appendToPage(parent, sorted_list, sorted_page, rhs);
        right_has_next = rightIter->advance();
    }

    sorted_list.push_back(sorted_page);
    return sorted_list;
}

void appendToPage(const MyDB_BufferManagerPtr& parent, vector<MyDB_PageReaderWriter> &sorted_list,
                  MyDB_PageReaderWriter& sorted_page, const MyDB_RecordPtr& next) {
    if(!sorted_page.append(next)){
        sorted_list.push_back(sorted_page);
        sorted_page = MyDB_PageReaderWriter(*parent);
        sorted_page.append(next);
    }
}


void sort(int, MyDB_TableReaderWriter &, MyDB_TableReaderWriter &, function<bool()>, MyDB_RecordPtr, MyDB_RecordPtr) {

}

#endif
