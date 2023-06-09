//
// Created by amirb on 07/06/2023.
//

#ifndef OS3_QUEUE_H
#define OS3_QUEUE_H

#include <pthread.h>
#include <stdlib.h>
#include "segel.h"
#include "request.h"
#include "Structs.h"


Queue* Queue_ctor();

void enqueue(struct Queue* q, int fd, Stats* stats);

Request* dequeue(struct Queue* q);

node* findBefore(node* first, int fd);

void dequeue_by_val(struct Queue* q, int fd);
void unlocked_dequeue_by_val(struct Queue* q, int fd);

void Queue_dtor(struct Queue* q);

void print_queue(struct Queue* q, const char* msg);

int getSize(Queue* q);

void getValues(Queue* q, int* dest);

#endif //OS3_QUEUE_H
