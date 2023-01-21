
#ifndef LRU_H
#define LRU_H


// Wrapper for a page
class Node {
public:

    Node(size_t pageSize);
    Node(size_t pageSize, Node* prev, Node* next);

private:
    Page* page;

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

#endif
