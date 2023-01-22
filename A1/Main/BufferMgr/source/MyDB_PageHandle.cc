
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include "MyDB_PageHandle.h"

void *MyDB_PageHandleBase :: getBytes () {
	return nullptr;
}

void MyDB_PageHandleBase :: wroteBytes () {
}

Page* getPagePtr(){
    return this->pagePtr;
}

MyDB_PageHandleBase :: ~MyDB_PageHandleBase () {
}

MyDB_PageHandle :: MyDB_PageHandle (Page* pagePtr) {
    pagePtr = pagePtr;
}
}

#endif

