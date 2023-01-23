
#ifndef PAGE_C
#define PAGE_C

using namespace std;

#include "MyDB_Table.h"

Page :: Page() {
    this->tablePtr = nullptr;
    this->pageId = -1;

    this->isDirty = false;
    this->isPinned = false;
}

Page :: Page(MyDB_TablePtr tablePtr, long i) {
    this->tablePtr = tablePtr;
    this->pageId = i;

    this->isDirty = false;
    this->isPinned = false;
}

MyDB_TablePtr Page :: getTablePtr() {
    return this->tablePtr;
}

long Page :: getPageId() {
    return this->pageId;
}

bool Page :: getDirty() {
    return this->isDirty;
}

void Page :: markDirty() {
    this->isDirty = true;
}

void Page :: markClean() {
    this->isDirty = false;
}

void Page :: pin() {
    this->isPinned = true;
}

void Page :: unpin() {
    this->isPinned = false;
}

# endif