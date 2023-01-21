
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

#endif LRU_C