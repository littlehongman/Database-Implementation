
#ifndef LRU_H
#define LRU_H


#include "MyDB_Page.h"
#include <unordered_map>

using namespace std;

// Wrapper for a page
class Node {
public:

    explicit Node(Page *pagePtr);

    Node(Page *pagePtr, Node *prev, Node *next);

    ~Node();

    Page* getPagePtr();

    Node* getPrev();

    Node* getNext();

    void setPrev(Node *prv);

    void setNext(Node *next);

private:
    Page *pagePtr;
    Node *prev;
    Node *next;
};

class LRU {
public:
    explicit LRU();

    ~LRU();

    void update(Page* pagePtr);

    Page* getEvictedPage();

    bool removeNode(Page* pagePtr);

    bool inLRU(Page *pagePtr);

    bool isEmpty();

private:
    Node *head;
    Node *tail;
    unordered_map<Page*, Node*> NodeMap;
};

#endif
