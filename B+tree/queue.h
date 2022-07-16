#ifndef Queue_H
#define Queue_H
#include "Node.h"

struct Caja {
    Node* Node;
    Caja* next;
};

class Queue {
    private:
        Caja *primero, *ultimo;
        int cuantos;

    public:
        Queue();
        ~Queue();
        void nuevo();
        void empty();
        void insert(Node*);
        Node* take();
        void draw();
        bool estaVacia(){ return cuantos == 0; }
};


Queue::Queue() {
    primero = NULL;
    ultimo = NULL;
    cuantos = 0;
}

Queue::~Queue() {
    Caja *p;

    while(primero) {
        p = primero;
        primero = p->next;
        delete(p);
    }
    ultimo = NULL;
    cuantos = 0;
}

void Queue::empty() {
    Caja *p;

    while(primero) {
        p = primero;
        primero = p->next;
        delete(p);
    }
    ultimo = NULL;
    cuantos = 0;
    delete(this);
}

void Queue::nuevo() {
    primero = NULL;
    ultimo = NULL;
    cuantos = 0;
}

void Queue::insert(Node* n) {
    Caja *p;
    p = new Caja;
    p->Node = n;

    if(primero == NULL) {
        p->next  =  NULL;
        primero = p;
        ultimo = p;
    }else {
        p->next = NULL;
        ultimo->next = p;
        ultimo = p;
    }
    cuantos++;
}

Node* Queue::take() {
    Caja *p;
    Node* Node;

    if(primero == NULL) {
      return NULL;
    }else {
        p = primero;
        primero = p->next;
        Node = p->Node;
        delete(p);
        cuantos--;

        if (cuantos == 0) {
            ultimo = NULL;
        }
    }
    return Node;
}


#endif
