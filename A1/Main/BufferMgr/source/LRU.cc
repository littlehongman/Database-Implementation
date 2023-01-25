
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

Page* Node :: getPagePtr() {
    return this->pagePtr;
}

Node* Node :: getPrev() {
    return this->prev;
}

Node* Node :: getNext() {
    return this->next;
}

void Node :: setPrev(Node* prv) {
    this->prev = prv;
}

void Node :: setNext(Node* nxt) {
    this->next = nxt;
}

Node::~Node() {
    this->pagePtr = nullptr;
    this->prev = nullptr;
    this->next = nullptr;
}

LRU :: LRU() {
    this->head = new Node(nullptr);
    this->tail = new Node(nullptr);
    this->head->setNext(tail); // head->next = tail;
    this->tail->setPrev(head); // tail->prev = head;
}

LRU::~LRU() {
    Node* curr = this->head;
    while (curr != nullptr) {
        Node* temp = curr;
        curr = curr->getNext();
        delete temp;
    }
    this->head = nullptr;
    this->tail = nullptr;
}

void LRU::update(Page* pagePtr) {
    if (this->NodeMap.find(pagePtr) == this->NodeMap.end()) { // not in the map
        Node* node = new Node(pagePtr);
        // insert to the head
        Node* next = this->head->getNext(); // next = head->next;
        this->head->setNext(node); // head->next = node;
        node->setPrev(this->head); // node->prev = head;
        node->setNext(next); // node->next = next;
        next->setPrev(node); // next->prev = node;
        this->NodeMap[pagePtr] = node;
    } else { // in the map
        Node* node = this->NodeMap[pagePtr]; // get the node
        // remove the node from the list
        Node* prev = node->getPrev(); // prev = node->prev;
        Node* next = node->getNext(); // next = node->next;
        prev->setNext(next); // prev->next = next;
        next->setPrev(prev); // next->prev = prev;
        // add the node to the head
        Node* nextHead = this->head->getNext();
        this->head->setNext(node);
        node->setPrev(this->head);
        node->setNext(nextHead);
        nextHead->setPrev(node);
    }
}

Page* LRU::getEvictedPage() {
    Node* last = this->tail->getPrev(); // last = tail->prev;
    if (last == this->head) { // empty list
        return nullptr;
    } else {
        // remove the last node
        Node* prev = last->getPrev(); // prev = last->prev;
        prev->setNext(this->tail); // prev->next = tail;
        this->tail->setPrev(prev); // tail->prev = prev;
        last->setNext(nullptr); // last->next = nullptr;
        last->setPrev(nullptr); // last->prev = nullptr;
        // remove the last node from the map
        this->NodeMap.erase(last->getPagePtr()); // remove the node from the map
        // return the page
        Page* pagePtr = last->getPagePtr();
        delete last;
        return pagePtr;
    }
}

bool LRU::removeNode(Page *pagePtr) {
    if (this->NodeMap.find(pagePtr) == this->NodeMap.end()) { // not in the map
        return false;
    } else { // in the map
        Node* node = this->NodeMap[pagePtr]; // get the node
        // remove the node from the list
        Node* prev = node->getPrev(); // prev = node->prev;
        Node* next = node->getNext(); // next = node->next;
        prev->setNext(next); // prev->next = next;
        next->setPrev(prev); // next->prev = prev;
        // remove the node from the map
        this->NodeMap.erase(pagePtr); // remove the node from the map
        // delete the node
        delete node;
        return true;
    }
}


#endif