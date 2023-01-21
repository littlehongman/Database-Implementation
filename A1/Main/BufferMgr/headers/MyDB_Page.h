
#ifndef PAGE_H
#define PAGE_H

class Page {

public:
    Page();
    ~Page();

    // 1. Set the pageSize of the page 2. Set isDirty to false 3. Set isPinned to false 4. Set the pageData to NULL

    Page(size_t pageSize);

    long getPageId();
    bool getDirty();
    long getPinCount();
    long getLastAccessTime();


    // Mark the page as dirty if the page is modified in buffer, but not written to disk, then we
    // need to mark the page as dirty.
    void markDirty();
    void markClean();

    // Pin the page, if the page is pinned, then it cannot be evicted from buffer pool.
    void pin();

    // Increase the pin count when the page is requested. (A query is executed on the page)
    void incrementPinCount();

    // Decrease the pin count when the page is released. (A query is finished on the page)
    void decrementPinCount();

    // Set the last access time to the current time.
    void setLastAccessTime(std :: time_t lastAccessTime);

private:
    size_t pageSize;
    long pageId;
    bool isDirty;
    long pinCount;
    bool isPinned;
    std :: time_t lastAccessTime;
};

#endif