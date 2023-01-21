
#ifndef LRU_H
#define LRU_H


// Wrapper for a page
class Node {
public:

    Node(Page* pagePtr);
    Node(Page* pagePtr, Node* prev, Node* next);

private:
    Page* pagePtr;

    Node* prev;
    Node* next;
};

class LRU {
public:
    LRU();

    void append(Node* node);
    void remove(Node* node);

private:
    Node* head;
    Node* tail;
    long size;
};

#endif LRU_H
