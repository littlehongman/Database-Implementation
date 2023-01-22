
#ifndef LRU_C
#define LRU_C

#include "LRU.h"

Node :: Node(Page* pagePtr) {
    pagePtr = pagePtr;
    prev = nullptr;
    next = nullptr;
}

Node :: Node(Page* pagePtr, Node* prev, Node* next) {
    pagePtr = pagePtr;
    prev = prev;
    next = next;
}

Page* Node :: getPage() {
    return pagePtr;
}

Node* Node :: getPrev() {
    return prev;
}

Node* Node :: getNext() {
    return next;
}

void Node :: setPrev(Node* prev) {
    prev = prev;
}

void Node :: setNext(Node* next) {
    next = next;
}

LRU :: LRU() {
    head = nullptr;
    tail = nullptr;
    size = 0;
}

LRU :: append(Node* node) {
    temp = head->next;
    head->next = node;
    node->prev = head;
    node->next = temp;
    temp->prev = node;
}

LRU :: remove(Node* node) {
    prev = node->prev;
    next = node->next;
    prev->next = next;
    next->prev = prev;
}

#endif