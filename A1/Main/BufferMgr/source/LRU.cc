
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

LRU :: LRU(size_t numPages) {
    head = new Node(nullptr);
    tail = new Node(nullptr);
    head.setNext(tail); // head->next = tail;
    tail.setPrev(head); // tail->prev = head;
    size = 0;
    capacity = numPages;
}

Node* LRU :: popLRU(Node* node) {
    if size == 0 {
        return nullptr;
    }
    node.getPrev().setNext(tail); // node->prev->next = tail;
    tail.setPrev(node.getPrev()); // tail->prev = node->prev;
    node.setPrev(nullptr); // node->prev = nullptr;
    node.setNext(nullptr); // node->next = nullptr;
    // TODO: do something with the deleted node
    size--;
    return node;
}

void LRU :: pushMRU(Node* node) {
    node.setNext(head.getNext()); // node->next = head->next;
    node.setPrev(head); // node->prev = head;
    head.getNext().setPrev(node); // head->next->prev = node;
    head.setNext(node); // head->next = node;
    size++;
}

void LRU :: updateMRU(Node* node) { // Node is already in cache, just update it
    popLRU(node);
    pushMRU(node);
}

void LRU :: insert(Page* pagePtr) {
    Node* node = new Node(pagePtr);
    if (size == capacity) {
        popLRU(tail.getPrev()); // tail->prev is the LRU node
    }
    pushMRU(node);
}

#endif