
#ifndef STACK_H
#define STACK_H

// this is the node class used to build up the LIFO stack
template <class Data>
class Node {

private:
    Data holdMe;
    Node *next;


public:
    ~Node () {}
    Node () {}

    Node(Data val) {
        this->holdMe = val;
        this->next = nullptr;
    }

    // Getters
    Data getHoldMe() {
        return holdMe;
    }

    Node* getNext() {
        return next;
    }

    // Setters
    void setHoldMe(Data val) {
        this->holdMe = val;
    }

    void setNext(Node* ptr) {
        this->next = ptr;
    }

};


// a simple LIFO stack
template <class Data>
class Stack {

    Node <Data> *head;

public:

    // destroys the stack
    ~Stack () {
        while (this->head != nullptr) {
            Node <Data> *temp = this->head;

            this->head = this->head->getNext();

            delete temp;
        }
    }

    // creates an empty stack
    Stack () {
        this->head = nullptr;
    }

    // adds pushMe to the top of the stack
    void push (Data val) {

        Node <Data> *temp = this->head;
        Node <Data> *newNode = new Node <Data>(val);
        newNode->setNext(temp);
        this->head = newNode;

    }

    // return true if there are not any items in the stack
    bool isEmpty () {
        return this->head == nullptr;
    }

    // pops the item on the top of the stack off, returning it...
    // if the stack is empty, the behavior is undefined
    Data pop () {
        if (this->head == nullptr) {
            return Data();
        }
        else {
            Node <Data> *temp = this->head;
            this->head = this->head->getNext();

            Data value = temp->getHoldMe();
            temp->setNext(nullptr);
            delete temp;

            return value;
        }
    }
};

#endif
