//
// Created by amirb on 07/06/2023.
//

#include "Queue.h"
#include "segel.h"

Queue* Queue_ctor(){
    Queue* q = (Queue * ) malloc(sizeof(Queue));
    q->queue_size = 0;
    Pthread_cond_init(&q->c, NULL);
    Pthread_mutex_init(&q->m, NULL);
    q->first = NULL;
    q->last = NULL;
    return q;
}

void enqueue(struct Queue* q, int fd) {
    printf("1\n");
    pthread_mutex_lock(&q->m);
    printf("2\n");

    /* insert from last */
    if(q->last==NULL){
        q->first = (node * ) malloc(sizeof(node));
        q->first->fd = fd;
        q->first->next =NULL;
        q->last = q->first;
    }
    else{
        node * temp = (node * ) malloc(sizeof(node));
        temp->next = NULL;
        temp->fd = fd;
        q->last->next = temp;
        q->last = temp;
    }
    q->queue_size++;
    pthread_cond_signal(&q->c);
    pthread_mutex_unlock(&q->m);
}

int dequeue(struct Queue* q) {
    pthread_mutex_lock(&q->m);
    int result;
    while (q->queue_size == 0) {
        pthread_cond_wait(&q->c, &q->m);
    }
    /* remove from first */
    if(q->first==NULL){
        unix_error("dequeue error");
        pthread_mutex_unlock(&q->m);
        return -1;
    }
    else{
        result = q->first->fd;
        node* to_free = q->first;
        if(q->first==q->last){
            q->last = NULL;
        }
        q->first = q->first->next;
        free(to_free);
    }
    q->queue_size--;
    pthread_mutex_unlock(&q->m);
    return result;
}

node* findBefore(node* first, int fd){
    node* temp = first;
    while(temp->next!=NULL && temp->next->fd!=fd){
        temp = temp->next;
    }
    if(temp->next==NULL){
        return NULL;
    }
    return temp;
}

void dequeue_by_val(struct Queue* q, int fd) {
    pthread_mutex_lock(&q->m);
    if(q->first==NULL){
        unix_error("dequeue error");
        pthread_mutex_unlock(&q->m);
        return;
    }
    if(q->first->fd==fd){
        node* to_free = q->first;
        if(q->first==q->last){
            q->last = NULL;
        }
        q->first = q->first->next;
        free(to_free);
        q->queue_size--;
        pthread_mutex_unlock(&q->m);
        return;
    }
    node* before = findBefore(q->first, fd);
    if(before==NULL){
        unix_error("dequeue error");
        pthread_mutex_unlock(&q->m);
        return;
    }
    else{
        node* to_free = before->next;
        before->next = to_free->next;
        free(to_free);
    }
    q->queue_size--;
    pthread_mutex_unlock(&q->m);
    return;
}

void unlocked_dequeue_by_val(struct Queue* q, int fd) {
    if(q->first==NULL){
        unix_error("dequeue error");
        pthread_mutex_unlock(&q->m);
        return;
    }
    if(q->first->fd==fd){
        node* to_free = q->first;
        if(q->first==q->last){
            q->last = NULL;
        }
        q->first = q->first->next;
        free(to_free);
        q->queue_size--;
        pthread_mutex_unlock(&q->m);
        return;
    }
    node* before = findBefore(q->first, fd);
    if(before==NULL){
        unix_error("dequeue error");
        pthread_mutex_unlock(&q->m);
        return;
    }
    else{
        node* to_free = before->next;
        before->next = to_free->next;
        free(to_free);
    }
    q->queue_size--;
    return;
}

void Queue_dtor(struct Queue* q){
    node* to_free;
    node* curr = q->first;
    while(curr!=NULL){
        to_free = curr;
        curr = curr->next;
        free(to_free);
    }

    Pthread_cond_destroy(&q->c);
    Pthread_mutex_destroy(&q->m);

    free(q);
}

void print_queue(struct Queue* q){
    node* curr = q->first;
    while(curr!=NULL){
        printf("pthread_fd=%d\n", (int)curr->fd);
        curr= curr->next;
    }
    printf("******************\n");
}

int getSize(Queue* q){
    return q->queue_size;
}

void getValues(Queue* q, int* dest){
    //dest must be the size of the queue
    node* curr = q->first;
    int i = 0;
    while(curr!=NULL){
        dest[i] = (int)curr->fd;
        ++i;
        curr= curr->next;
    }
}

