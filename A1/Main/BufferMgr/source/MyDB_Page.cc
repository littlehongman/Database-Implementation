
#ifndef PAGE_C
#define PAGE_C


#include "MyDB_Page.h"

using namespace std;

Page :: Page() {
    this->tablePtr = nullptr;
    this->pageId = -1;

    this->bufferPtr = nullptr;

    this->isDirty = false;
    this->isPinned = false;

    this->referenceCount = 0;

    this->slot = -1;
}

Page :: Page(MyDB_TablePtr tablePtr, long i) {
    this->tablePtr = tablePtr;
    this->pageId = i;

    this->bufferPtr = nullptr;

    this->isDirty = false;
    this->isPinned = false;

    this->referenceCount = 0;

    this->slot = -1;
}

MyDB_TablePtr Page :: getTablePtr() {
    return this->tablePtr;
}

long Page :: getPageId() {
    return this->pageId;
}

char* Page :: getBufferPtr() {
    return this->bufferPtr;
}

void Page :: setBufferPtr(char* bufferPtr) {
    this->bufferPtr = bufferPtr;
}

bool Page :: getDirty() {
    return this->isDirty;
}

bool Page :: getPinned() {
    return this->isPinned;
}

long Page :: getReferenceCount() {
    return this->referenceCount;
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

void Page :: increaseReferenceCount() {
    this->referenceCount++;
}

void Page :: decreaseReferenceCount() {
    this->referenceCount--;
}

bool Page::getIsDirty() {
    return this->isDirty;
}

void Page::setSlot(int slot) {
    this->slot = slot;
}

int Page::getSlot() {
    return this->slot;

}

Page::~Page() {

}

# endif