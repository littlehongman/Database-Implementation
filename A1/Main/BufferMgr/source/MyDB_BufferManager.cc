
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include "MyDB_PageHandle.h"
#include <string>
#include <ctime>
#include <memory>
#include "MyDB_Page.h"


using namespace std;

// Return a handle to the page with the given Table and pageId => Not anonymous
MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr tablePtr, long i) {
    // TODO: make_pair error
    auto key = make_pair(tablePtr->getName(), i);

    // If the page is already in the buffer
    if (pageMap.find(key) != pageMap.end()){
        Page *pagePtr = pageMap[key];
        MyDB_PageHandle handle = make_shared<MyDB_PageHandleBase>(pagePtr);

        // TODO: update the LRU order
        //TODO: think if accessTime is needed

        return handle;
    }
    //  If the page is not in the buffer
    else{
        // If the buffer pool is not full
        if (pageMap.size() != this->numPages){
            Page *newPagePtr = new Page(tablePtr, i);
            MyDB_PageHandle handle = make_shared<MyDB_PageHandleBase>(newPagePtr);

            // Apply LRU policy to the page
            lru.insert(newPagePtr);

            // Store the pagePtr in the unordered_map
            pageMap[key] = newPagePtr;

            // TODO: update the LRU order

            return nullptr;
        }
        // If the buffer pool is full
        else{
            Page *newPagePtr = new Page(tablePtr, i);
            MyDB_PageHandle handle = make_shared<MyDB_PageHandleBase>(newPagePtr);

            // Apply LRU policy to the page
            // TODO: get the pagePtr that is evicted
            lru.insert(newPagePtr);

            // Store the pagePtr in the unordered_map
            pageMap[key] = newPagePtr;

            // TODO: update the LRU order
            // TODO: Remove the evicted page from the unordered_map

            return handle;
        }
    }
}

// Return an anonymous page -> No need to track with pageMap
MyDB_PageHandle MyDB_BufferManager :: getPage () {
    Page *newPagePtr = new Page();
    MyDB_PageHandle handle = make_shared<MyDB_PageHandleBase>(newPagePtr);

    lru.insert(newPagePtr);


	return handle;
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr tablePtr, long i) {
    MyDB_PageHandle handle = getPage(tablePtr, i);
    handle->getPagePtr()->pin();

	return handle;
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
    MyDB_PageHandle handle = getPage();
    handle->getPagePtr()->pin();

	return handle;
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
    unpinMe->getPagePtr()->unpin();

    return;
}

MyDB_BufferManager :: MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile) {
    numPages = numPages;
    pageSize = pageSize;
    tempFile = tempFile;

    // Allocate memory for the whole buffer
    // TODO: WHY CAST THE MEMORY TO CHAR*?
    buffer = (char *)malloc(pageSize * numPages);

//    lru = LRU(numPages);

    // Initialize the set of chunkIds
    for (int i = 0; i < numPages; i++){
        chunkIds.insert(i);
    }
}

MyDB_BufferManager :: ~MyDB_BufferManager () {

}

#endif


