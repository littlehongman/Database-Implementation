
#ifndef LRU_H
#define LRU_H


// Wrapper for a page
class Node {
public:

    Node(Page *pagePtr);

    Node(Page *pagePtr, Node *prev, Node *next);

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
    LRU(size_t numPages);

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
