
#ifndef SORT_C
#define SORT_C

#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableRecIterator.h"
#include "MyDB_TableRecIteratorAlt.h"
#include "MyDB_TableReaderWriter.h"
#include "Sorting.h"
#include "MyDB_PageListIteratorAlt.h"

using namespace std;


void mergeIntoFile(MyDB_TableReaderWriter &sortIntoMe, vector<MyDB_RecordIteratorAltPtr> &mergeUs, function<bool()> comparator, MyDB_RecordPtr lhs,
                   MyDB_RecordPtr rhs) {

    auto compare = [&lhs, &rhs, &comparator](const MyDB_RecordIteratorAltPtr& a, const MyDB_RecordIteratorAltPtr& b) {
        // Load record ptrs with iterators
        a->getCurrent(lhs);
        b->getCurrent(rhs);

        // If comparator() == true => lhs < rhs
        // Else lhs > rhs
        // To make the min heap, we will set lhs > rhs
        return !comparator();
    };

    // Define data structures
    priority_queue<MyDB_RecordIteratorAltPtr, vector<MyDB_RecordIteratorAltPtr>, decltype(compare)> pq(compare);

    // Push vector items into queue
    while (mergeUs.size() > 0){
        pq.push(mergeUs.back());
        mergeUs.pop_back();
    }

    // Merge K sorted Lists
    while(pq.size() > 0){
        MyDB_RecordIteratorAltPtr smallest = pq.top();
        pq.pop();

        smallest->getCurrent(lhs);
        sortIntoMe.append(lhs);

        if (smallest->advance()){
            pq.push(smallest);
        }
    }

    return;
}


// Literally "Merge Two Sorted Lists" from LeetCode, but merges records into a vector of pages
// Merge two sorted pages into list of pages
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

        // If comparator == True => lhs < rhs
        // Else => lhs > rhs
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


void sort(int runSize, MyDB_TableReaderWriter &sortMe, MyDB_TableReaderWriter &sortIntoMe, function<bool()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {
    // 1. The sort phase -> Get sorted runs

    // (0) Define some variables
    int page_num = sortMe.getTable()->lastPage();
    int curr_idx = 0;
    MyDB_BufferManagerPtr bufferMgrPtr = sortMe.getBufferMgr();

    vector<MyDB_RecordIteratorAltPtr> all_pages; // store
    queue<MyDB_RecordIteratorAltPtr> run_pages; // store pages of current run

    // (1)  [1] Load a run of pages into RAM
    //      [2] Sort each individual page
    // Note: Since all maximum page size is identical, no need to consider memory size
    while (curr_idx <= page_num){

        // 1. Get runSize of pages
        for (int i = 0; i < runSize && curr_idx <= page_num; i++){ // TWO conditions
            MyDB_PageReaderWriter curr_page = sortIntoMe[curr_idx];

            // TODO: sort in-place or not?
            curr_page.getBytes(); // Load into RAM
            curr_page.sortInPlace(comparator, lhs, rhs); // Sort individual page

            run_pages.push(curr_page.getIteratorAlt()); // Push iterator into queue

            curr_idx ++; // increment curr_idx
        }
        // TODO: what if the whole table only have one page => no need to deal;
        // 2. Repeating called mergeToList (merge 2 sorted list), until only one big list of pages
        while (run_pages.size() > 1){
            MyDB_RecordIteratorAltPtr left_iter = run_pages.front();
            run_pages.pop();

            MyDB_RecordIteratorAltPtr right_iter = run_pages.front();
            run_pages.pop();

            vector<MyDB_PageReaderWriter> result = mergeIntoList(bufferMgrPtr, left_iter, right_iter, comparator, lhs, rhs);

            run_pages.push(make_shared <MyDB_PageListIteratorAlt> (result));

        }

        all_pages.push_back(run_pages.back());
        run_pages.pop();
    }

    // 2. The merge phase -> Merge all sorted runs
    mergeIntoFile(sortIntoMe, all_pages, comparator, lhs, rhs);

}

#endif
