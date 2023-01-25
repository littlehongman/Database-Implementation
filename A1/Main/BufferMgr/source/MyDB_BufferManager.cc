
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include <string>
#include <ctime>
#include <iostream>
#include <memory>
#include "MyDB_Page.h"
#include <unistd.h>
#include "LRU.h"
#include <fcntl.h>


using namespace std;

// Return a handle to the page with the given Table and pageId => Not anonymous
MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr tablePtr, long i) {
    auto key = tablePtr->getName() + "_" + to_string(i);

    // If the page is already in the buffer
    if (pageMap.find(key) != pageMap.end()){
        Page *pagePtr = pageMap[key];
        MyDB_PageHandle handle = make_shared<MyDB_PageHandleBase>(pagePtr, this);

        return handle;
    }
    //  If the page is not in the buffer
    else{
        Page *newPagePtr = new Page(tablePtr, i);

        MyDB_PageHandle handle = make_shared<MyDB_PageHandleBase>(newPagePtr, this);

        // Store the pagePtr in the unordered_map
        pageMap[key] = newPagePtr;

        return handle;

    }
}

// Return an anonymous page -> No need to track with pageMap
MyDB_PageHandle MyDB_BufferManager :: getPage () {
    Page *newPagePtr = new Page();
    MyDB_PageHandle handle = make_shared<MyDB_PageHandleBase>(newPagePtr, this);


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
}

void MyDB_BufferManager :: updateLRU (Page *pagePtr) {
    lru->update(pagePtr);
}

void MyDB_BufferManager :: removeFromLRU (Page *pagePtr) {
    lru->removeNode(pagePtr);
}

bool MyDB_BufferManager :: isFull(){
    return this->chunkPointers.size() == 0;
}

char* MyDB_BufferManager :: allocateChunk(){
    if (this->chunkPointers.size() != 0){
        char* bufferPtr = this->chunkPointers.back();
        this->chunkPointers.pop_back();

        return bufferPtr;
    }

    else{
        // Get a Node from the LRU
        Page* evictPage = this->lru->getEvictedPage();

        // If the page is dirty, then we write it to disk
        if (evictPage->getIsDirty()){
            this->writeDisk(evictPage);
        }

        char* bufferPtr = evictPage->getBufferPtr();
        evictPage->setBufferPtr(nullptr);

        // if the page is anonymous, then need to be deleted
        if (evictPage->getTablePtr() == nullptr){
            delete evictPage;
        }

        return bufferPtr;
    }
}

void MyDB_BufferManager :: reclaimChunk(char* chunkPtr){
    this->chunkPointers.push_back(chunkPtr);
}

int MyDB_BufferManager ::getTempSlot() {
    if (this->tempSlots.empty()){
        int offset = this->slot;
        this->slot += 1;

        return offset;
    }
    else{
        int offset = this->tempSlots.back();
        this->tempSlots.pop_back();

        return offset;
    }
}

void MyDB_BufferManager ::reclaimTempSlot(int offset) {
    this->tempSlots.push_back(offset);
}


void MyDB_BufferManager :: readDisk(Page *pagePtr) {
    MyDB_TablePtr whichTable = pagePtr->getTablePtr();
    long pageId = pagePtr->getPageId();
    char* bufferPtr = pagePtr->getBufferPtr();

    if (whichTable != nullptr){ // if not anonymous
        string key = whichTable->getStorageLoc() + "/" + whichTable->getName();
        int fd = -1;

        // if the file descriptor is exist, then read the page from disk
        // Maybe need "errno != EBADF";
        if (this->fileMap.find(key) != this->fileMap.end() && fcntl(this->fileMap[key], F_GETFL) != -1) {
            fd = this->fileMap[key];
        }
        else{
            //if the file descriptor is not exist, then open it
            fd = open(key.c_str(), O_RDONLY | O_CREAT | O_FSYNC); // (1) Read only (2) Create when not exist
        }

        // To set file pointer to the  pageSize * i byte in the file
        lseek(fd, this->pageSize * pageId, SEEK_SET);

        // Read the data from disk to buffer memory
        read(fd, bufferPtr, this->pageSize);
    }

    else{
        string key = this->tempFile;
        int slot = pagePtr->getSlot();

        int fd = open(key.c_str(), O_RDONLY | O_CREAT | O_FSYNC);

        // To set file pointer to the  pageSize * i byte in the file
        lseek(fd, this->pageSize * slot, SEEK_SET);

        // Write the data from disk to buffer memory
        // write(file to be written, buffer, size)
        read(fd, bufferPtr, this->pageSize);

        close(fd);
    }
}


void MyDB_BufferManager :: writeDisk(Page *pagePtr) {
    MyDB_TablePtr whichTable = pagePtr->getTablePtr();
    long pageId = pagePtr->getPageId();
    char* bufferPtr = pagePtr->getBufferPtr();

    if (whichTable != nullptr){ // if not anonymous
        string key = whichTable->getStorageLoc() + "/" + whichTable->getName();
        int fd = -1;

        // if the file descriptor is exist, then read the page from disk
        // Maybe need "errno != EBADF";
        if (this->fileMap.find(key) == this->fileMap.end() && fcntl(this->fileMap[key], F_GETFL) != -1) {
            fd = this->fileMap[key];
        }
        else{
            //if the file descriptor is not exist, then open it
            fd = open(key.c_str(), O_WRONLY | O_CREAT | O_FSYNC); // (1) Read only (2) Create when not exist
        }

        // To set file pointer to the  pageSize * i byte in the file
        lseek(fd, this->pageSize * pageId, SEEK_SET);

        // Write the data from disk to buffer memory
        // write(file to be written, buffer, size)
        write(fd, bufferPtr, this->pageSize);
    }

    else{
        //TODO: address anonymous case
        string key = this->tempFile;
        int slot = pagePtr->getSlot();

        int fd = open(key.c_str(), O_WRONLY | O_CREAT | O_FSYNC) ;

        // To set file pointer to the  pageSize * i byte in the file
        lseek(fd, this->pageSize * slot, SEEK_SET);

        // Write the data from disk to buffer memory
        // write(file to be written, buffer, size)
        write(fd, bufferPtr, this->pageSize);

        close(fd);
    }
}

MyDB_BufferManager :: MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile) {
    this->numPages = numPages;
    this->pageSize = pageSize;
    this->tempFile = tempFile;

    // Allocate memory for the whole buffer
    // TODO: WHY CAST THE MEMORY TO CHAR*?
    this->buffer = (char *)malloc(pageSize * numPages);

    this->lru = new LRU();

    // Initialize the vector of chunk starting pointers
    for (int i = 0; i < numPages; i++){
        chunkPointers.push_back(this->buffer + i * pageSize);
    }
}

MyDB_BufferManager :: ~MyDB_BufferManager () {

}

#endif


