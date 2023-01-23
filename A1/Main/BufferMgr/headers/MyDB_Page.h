
#ifndef PAGE_H
#define PAGE_H

using namespace std;
#include "MyDB_Table.h"

class Page {

public:
    Page();
    ~Page();

    // 1. Set the pageSize of the page 2. Set isDirty to false 3. Set isPinned to false 4. Set the pageData to NULL

    Page(MyDB_TablePtr tablePtr, long i);

    // Return the table pointer of the page
    MyDB_TablePtr getTablePtr();

    // Return the page Id of the page
    long getPageId();

    // Return if the page is dirty
    bool getDirty();

    // Mark the page as dirty if the page is modified in buffer, but not written to disk, then we
    // need to mark the page as dirty.
    void markDirty();
    void markClean();

    // Pin the page, if the page is pinned, then it cannot be evicted from buffer pool.
    void pin();

    // Unpin the page, if the page is unpinned, then it can be evicted from buffer pool.
    void unpin();

    // Increase the reference count of the page when a page handle base is create to reference the page.
    void increaseReferenceCount();

    // Decrease the reference count of the page when a page handle base is destroyed.
    void decreaseReferenceCount();

//    // Increase the pin count when the page is requested. (A query is executed on the page)
//    void incrementPinCount();
//
//    // Decrease the pin count when the page is released. (A query is finished on the page)
//    void decrementPinCount();


//    // Set the last access time to the current time.
//    void setLastAccessTime(std :: time_t lastAccessTime);
//
//    long getPinCount();
//    long getLastAccessTime();

private:
    // Absolute address of the page
    MyDB_TablePtr tablePtr;
    long pageId;

    // Other attributes of the page
    bool isDirty;
    bool isPinned;
    long referenceCount;

    // long pinCount;
//    std :: time_t lastAccessTime;
};

#endif