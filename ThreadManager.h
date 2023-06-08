//
// Created by Elad on 07/06/2023.
//

#ifndef OS3_THREADMANAGER_H
#define OS3_THREADMANAGER_H

#include <stdbool.h>
#include "segel.h"
#include "request.h"
#include "Queue.h"

#define BLOCK_SCHEDALG "block"
#define BLOCK_FLUSH_SCHEDALG "bf"

struct ThreadManager{
    int threads_amount;
    int queue_size;
    char* sched_alg;
    pthread_t* thread_pool;
    Queue* busyRequests;
    Queue* waitingRequests;

    pthread_cond_t  c;
};

typedef struct ThreadManager ThreadManager;

typedef struct exeThreadWrapperStruct{
    ThreadManager* tm;
    int fd;
} exeThreadWrapperStruct;


ThreadManager* ThreadManagerCtor(int threads_amount, int queue_size, char* sched_alg);

void ThreadManagerDtor(ThreadManager* tm);

void ThreadManagerHandleRequest(ThreadManager* tm, int fd);

#endif //OS3_THREADMANAGER_H
