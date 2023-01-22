
#ifndef LRU_C
#define LRU_C

#include "LRU.h"

Node :: Node(Page* pagePtr) {
    this->pagePtr = pagePtr;
    this->prev = nullptr;
    this->next = nullptr;
}

Node :: Node(Page* pagePtr, Node* prev, Node* next) {
    this->pagePtr = pagePtr;
    this->prev = prev;
    this->next = next;
}

Page* Node :: getPage() {
    return this->pagePtr;
}

Node* Node :: getPrev() {
    return this->prev;
}

Node* Node :: getNext() {
    return this->next;
}

void Node :: setPrev(Node* prev) {
    this->prev = prev;
}

void Node :: setNext(Node* next) {
    this->next = next;
}

LRU :: LRU(size_t numPages) {
    this->head = new Node(nullptr);
    this->tail = new Node(nullptr);
    this->head->setNext(tail); // head->next = tail;
    this->tail->setPrev(head); // tail->prev = head;
    this->size = 0;
    this->capacity = numPages;
}

Node* LRU :: popLRU(Node* node) {
    if (this->size == 0) {
        return nullptr;
    }
    node->getPrev()->setNext(this->tail); // node->prev->next = tail;
    this->tail->setPrev(node->getPrev()); // tail->prev = node->prev;
    node->setPrev(nullptr); // node->prev = nullptr;
    node->setNext(nullptr); // node->next = nullptr;
    // TODO: do something with the deleted node
    size--;
    return node;
}

void LRU :: pushMRU(Node* node) {
    node->setNext(this->head->getNext()); // node->next = head->next;
    node->setPrev(this->head); // node->prev = head;
    this->head->getNext()->setPrev(node); // head->next->prev = node;
    this->head->setNext(node); // head->next = node;
    this->size++;
}

void LRU :: updateMRU(Node* node) { // Node is already in cache, just update it
    popLRU(node);
    pushMRU(node);
}

void LRU :: insert(Page* pagePtr) {
    Node *node = new Node(pagePtr);
    if (this->size == this->capacity) {
        popLRU(this->tail->getPrev()); // tail->prev is the LRU node
    }
    pushMRU(node);
}

#endif