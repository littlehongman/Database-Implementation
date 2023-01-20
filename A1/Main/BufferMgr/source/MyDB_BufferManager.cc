
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include <string>
#include <ctime>
#include "MyDB_Page.h"

using namespace std;

MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr, long i) {
    // If the page is already in the buffer pool
    if (pageMap.find(i) != pageMap.end()){
        Page* requestPage = pageMap[i];

        return nullptr;
    }
    // Else if the page is not in the buffer pool
    else{
        // If the buffer pool is full
        if (pageMap.size() == numPages){

            return nullptr;
        }
        // If the buffer pool is not full
        else{
            // Create a new page object and
            // Map the pageId to the pointer of the page object
            // Add the page object to the buffer pool
            Page *newPage = new Page(pageSize);
            newPage->setLastAccessTime(time(nullptr));


            pageMap[i] = newPage;
            lruBufferPool.push(*newPage);

            return nullptr;
        }
    }



	return nullptr;
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {
	return nullptr;		
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr, long) {
	return nullptr;		
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
	return nullptr;		
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
}

MyDB_BufferManager :: MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile) {
    numPages = numPages;
    pageSize = pageSize;
    tempFile = tempFile;

    // Allocate memory for the whole buffer pool
    lruBufferPool = (Page *)malloc(pageSize * numPages);
}

MyDB_BufferManager :: ~MyDB_BufferManager () {
    free(lruBufferPool);
}
	
#endif


