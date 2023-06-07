//
// Created by amirb on 07/06/2023.
//

#include "Queue.h"

int main(){
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

}