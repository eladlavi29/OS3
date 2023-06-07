//
// Created by amirb on 07/06/2023.
//
#include <pthread.h>

#ifndef OS3_QUEUE_H
#define OS3_QUEUE_H

typedef struct node {
    pthread_t thread;
    struct node * next;
} node;

typedef struct Queue {
    cond_t c; // should be initialized
    mutex_t m; // should be initialized
    int queue_size;
    node *first;
    node *last;
} Queue;

Queue* Queue(){
    Queue* q = (Queue * ) malloc(sizeof(Queue));
    q->queue_size = 0;
    Pthread_cond_init(&q->c, NULL);
    Pthread_mutex_init(&q->m, NULL);
    q->first = NULL;
    q->last = NULL;
    return q;
}

void enqueue(Queue* q, pthread_t t) {
    mutex_lock(&q->m);
    /* insert from last */
    if(q->last==NULL){
        q->first = (node * ) malloc(sizeof(node));
        q->first->thread = t;
        q->first->next =NULL;
        q->last = q->first;
    }
    else{
        node * temp = (node * ) malloc(sizeof(node));
        temp->next = NULL;
        temp->thread = t;
        q->last->next = temp;
        q->last = temp;
    }
    q->queue_size++;
    cond_signal(&q->c);
    mutex_unlock(&q->m);
}

pthread_t dequeue(Queue* q) {
    mutex_lock(&q->m);
    pthread_t result;
    while (q->queue_size == 0) {
        cond_wait(&q->c, &q->m);
    }
    /* remove from first */
    if(q->first==NULL){
        unix_error("dequeue error");
        mutex_unlock(&q->m);
        return;
    }
    else{
        result = q->first->thread;
        node* to_free = q->first;
        if(q->first==q->last){
            q->last = NULL;
        }
        q->first = q->first->next;
    }
    free(to_free);
    q->queue_size--;
    mutex_unlock(&q->m);
    return result;
}

node* findBefore(Queue* q, node* first, pthread_t target){
    node* temp = q->first;
    while(temp->next!=NULL && temp->next->thread!=target){
        temp = temp->next;
    }
    if(temp->next==NULL){
        return NULL;
    }
    return temp;
}

void dequeue(Queue* q, pthread_t target) {
    mutex_lock(&q->m);
    if(q->first==NULL){
        unix_error("dequeue error");
        mutex_unlock(&q->m);
        return;
    }
    if(q->first->thread==target){
        node* to_free = q->first;
        if(q->first==q->last){
            q->last = NULL;
        }
        q->first = q->first->next;
        free(to_free);
        q->queue_size--;
        mutex_unlock(&q->m);
        return;
    }
    node* before = findBefore(q->first, target);
    if(before==NULL){
        unix_error("dequeue error");
        mutex_unlock(&q->m);
        return;
    }
    else{
        node* to_free = before->next;
        before->next = to_free->next;
        free(to_free);
    }
    q->queue_size--;
    mutex_unlock(&q->m);
    return;
}

void dtor(Queue* q){
    node* to_free;
    node* curr = q->first;
    while(curr!=NULL){
        to_free = curr;
        curr = curr->next;
        free(to_free);
    }
    free(q);
}

void print(Queue* q){
    node* curr = q->first;
    while(curr!=NULL){
        printf("pthread_num=%d\n", curr->thread);
    }
}

#endif //OS3_QUEUE_H
