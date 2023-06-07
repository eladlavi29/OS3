//
// Created by amirb on 07/06/2023.
//

#ifndef OS3_QUEUE_H
#define OS3_QUEUE_H

typedef struct node {
    pthread_t thread;
    struct node * next;
} node;

typedef struct Queue {
    pthread_cond_t  c; // should be initialized
    pthread_mutex_t m; // should be initialized
    int queue_size;
    node *first;
    node *last;
} Queue;

Queue* Queue_ctor();

void enqueue(struct Queue* q, pthread_t t);

pthread_t dequeue(struct Queue* q);

node* findBefore(struct Queue* q, node* first, pthread_t target);

void dequeue(struct Queue* q, pthread_t target);

void dtor(struct Queue* q);

void print_queue(struct Queue* q);

#endif //OS3_QUEUE_H
