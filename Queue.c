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

void enqueue(struct Queue* q, pthread_t t) {
    pthread_mutex_lock(&q->m);
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
    pthread_cond_signal(&q->c);
    pthread_mutex_unlock(&q->m);
}

pthread_t dequeue(struct Queue* q) {
    pthread_mutex_lock(&q->m);
    pthread_t result;
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
        result = q->first->thread;
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

node* findBefore(node* first, pthread_t target){
    node* temp = first;
    while(temp->next!=NULL && temp->next->thread!=target){
        temp = temp->next;
    }
    if(temp->next==NULL){
        return NULL;
    }
    return temp;
}

void dequeue_by_val(struct Queue* q, pthread_t target) {
    pthread_mutex_lock(&q->m);
    if(q->first==NULL){
        unix_error("dequeue error");
        pthread_mutex_unlock(&q->m);
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
        pthread_mutex_unlock(&q->m);
        return;
    }
    node* before = findBefore(q->first, target);
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

void queue_dtor(struct Queue* q){
    node* to_free;
    node* curr = q->first;
    while(curr!=NULL){
        to_free = curr;
        curr = curr->next;
        free(to_free);
    }
    free(q);
}

void print_queue(struct Queue* q){
    node* curr = q->first;
    while(curr!=NULL){
        printf("pthread_num=%d\n", (int)curr->thread);
        curr= curr->next;
    }
}

int getSize(Queue* q){
    return q->queue_size;
}

int main(int argc, char *argv[]) {
    Queue* q = Queue_ctor();
    pthread_t t1 = 1;
    pthread_t t2 = 2;
    pthread_t t3 = 3;
    pthread_t t4 = 4;
    pthread_t t5 = 5;
    print_queue(q);
    enqueue(q,t1);
    enqueue(q,t2);
    enqueue(q,t3);
    enqueue(q,t4);
    enqueue(q,t5);
    print_queue(q);
    dequeue(q);
    dequeue(q);
    dequeue(q);
    print_queue(q);
    enqueue(q,t1);
    enqueue(q,t2);
    enqueue(q,t3);
    print_queue(q);
    dequeue_by_val(q, 4);
    print_queue(q);
    return 0;
}
