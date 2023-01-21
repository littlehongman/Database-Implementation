
#ifndef LRU_C
#define LRU_C

#include "LRU.h"

Node :: Node(size_t pageSize) {
    this->page = new Page(pageSize);
    prev = nullptr;
    next = nullptr;
}

Node :: Node(size_t pageSize, Node* prev, Node* next) {
    this->page = new Page(pageSize);
    prev = prev;
    next = next;
}

LRU :: LRU() {
    head = Node(1)
    tail = Node(1);
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