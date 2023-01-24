
#ifndef BUFFER_MGR_H
#define BUFFER_MGR_H

#include "MyDB_PageHandle.h"
#include "MyDB_Table.h"
#include <queue>
#include <unordered_map>
#include "MyDB_Page.h"
#include "LRU.h"
#include <set>

using namespace std;

class MyDB_BufferManager {

public:
	// THESE METHODS MUST APPEAR AND THE PROTOTYPES CANNOT CHANGE!

	// gets the i^th page in the table whichTable... note that if the page
	// is currently being used (that is, the page is current buffered) a handle 
	// to that already-buffered page should be returned

    // Johnson: return the pageId i from the table whichTable
	MyDB_PageHandle getPage (MyDB_TablePtr whichTable, long i);

	// gets a temporary page that will no longer exist (1) after the buffer manager
	// has been destroyed, or (2) there are no more references to it anywhere in the
	// program.  Typically such a temporary page will be used as buffer memory.
	// since it is just a temp page, it is not associated with any particular 
	// table
	MyDB_PageHandle getPage ();

	// gets the i^th page in the table whichTable... the only difference 
	// between this method and getPage (whicTable, i) is that the page will be 
	// pinned in RAM; it cannot be written out to the file
	MyDB_PageHandle getPinnedPage (MyDB_TablePtr whichTable, long i);

	// gets a temporary page, like getPage (), except that this one is pinned
	MyDB_PageHandle getPinnedPage ();

	// un-pins the specified page
	void unpin (MyDB_PageHandle unpinMe);

    // insert a unpinned page into the LRU
    void insertLRU(Page *pagePtr);

    // Read the requested data from the disk into a chunk of buffer memory
    void readDisk(Page *pagePtr);

    // Read the modified data on the buffer back to the disk
    void writeDisk(Page *pagePtr);

    // Check if the buffer pool is full
    bool isFull();

    // Allocate a chunk from the buffer memory
    char* allocateChunk();

    // Evict a chunk of buffer memory
    void reclaimChunk(char* chunkPtr);

	// creates an LRU buffer manager... params are as follows:
	// 1) the size of each page is pageSize 
	// 2) the number of pages managed by the buffer manager is numPages;
	// 3) temporary pages are written to the file tempFile
	MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile);
	
	// when the buffer manager is destroyed, all of the dirty pages need to be
	// written back to disk, any necessary data needs to be written to the catalog,
	// and any temporary files need to be deleted
	~MyDB_BufferManager ();

	// FEEL FREE TO ADD ADDITIONAL PUBLIC METHODS 

private:
    size_t numPages;
    size_t pageSize;
    string tempFile;

    // key: tableName_pageId
    // val: A pointer to the page object
    unordered_map<string, Page*> pageMap;

    // Apply LRU policy to maintain buffer pool
    LRU* lru;

    // The actual space for the buffer pool
    // buffer will point to the beginning of the buffer pool
    char *buffer;

    // Store the chunk pointers in which the chunk of the buffer pool is free
    vector<size_t> chunkPointers;

    // key: storageLoc/tableName
    // val: file descriptor
    unordered_map<string, int> fileMap;

};

#endif


