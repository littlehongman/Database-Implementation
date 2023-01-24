
#ifndef LRU_H
#define LRU_H


#include "MyDB_Page.h"

// Wrapper for a page
class Node {
public:

    Node(Page *pagePtr);

    Node(Page *pagePtr, Node *prev, Node *next);

    ~Node(); // TODO: Node destructor

    Page *getPage();

    Node *getPrev();

    Node *getNext();

    void setPrev(Node *prev);

    void setNext(Node *next);

private:
    Page *pagePtr;
    Node *prev;
    Node *next;
};

class LRU {
public:
    explicit LRU(size_t numPages);

    ~LRU(); // TODO: LRU destructor

    Node* popLRU(Node *node); // helper function

    void pushMRU(Node *node); // helper function

    void updateMRU(Node *node);

    Node* insert(Page *pagePtr);

private:
    Node *head;
    Node *tail;
    long size;
    size_t capacity;

};

#endif
