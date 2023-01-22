
#ifndef LRU_H
#define LRU_H


// Wrapper for a page
class Node {
public:

    Node(Page* pagePtr);
    Node(Page* pagePtr, Node* prev, Node* next);

    Page* getPage();

    Node* getPrev();

    Node* getNext();

    void setPrev(Node* prev);

    void setNext(Node* next);

private:
    Page* pagePtr;
    Node* prev;
    Node* next;
};

class LRU {
public:
    LRU(size_t numPages);

    void append(Node* node);
    void remove(Node* node);

private:
    Node* head;
    Node* tail;
    long size;
};

#endif
