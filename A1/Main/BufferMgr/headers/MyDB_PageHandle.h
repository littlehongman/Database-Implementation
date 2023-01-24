
#ifndef PAGE_HANDLE_H
#define PAGE_HANDLE_H

#include <memory>
#include "MyDB_Page.h"
#include "MyDB_BufferManager.h"

// page handles are basically smart pointers
using namespace std;
class MyDB_PageHandleBase;
typedef shared_ptr <MyDB_PageHandleBase> MyDB_PageHandle;
// share_ptr reference: https://www.educative.io/answers/shared-pointers-in-cpp

class MyDB_PageHandleBase {

public:

	// THESE METHODS MUST BE IMPLEMENTED WITHOUT CHANGING THE DEFINITION

	// access the raw bytes in this page... if the page is not currently
	// in the buffer, then the contents of the page are loaded from 
	// secondary storage. 
	void *getBytes ();

	// let the page know that we have written to the bytes.  Must always
	// be called once the page's bytes have been written.  If this is not
	// called, then the page will never be marked as dirty, and the page
	// will never be written to disk. 
	void wroteBytes ();

    Page* getPagePtr();

	// There are no more references to the handle when this is called...
	// this should decrement a reference count to the number of handles
	// to the particular page that it references.  If the number of 
	// references to a pinned page goes down to zero, then the page should
	// become unpinned.  
	~MyDB_PageHandleBase ();

    // Initial a page handle base to store pointer to the actual page
    MyDB_PageHandleBase(Page *pagePtr);


	// FEEL FREE TO ADD ADDITIONAL PUBLIC METHODS

private:
    Page *pagePtr;

    // Reference to buffer manager
    // (1) Access the buffer memory
    // (2) Access the LRU list
    MyDB_BufferManager *bufferManagerPtr;

};

#endif

