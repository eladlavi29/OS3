//
// Created by Elad on 07/06/2023.
//

#ifndef OS3_THREADMANAGER_H
#define OS3_THREADMANAGER_H

#include <stdbool.h>
#include "segel.h"
#include "request.h"
#include "Queue.h"

struct ThreadManager{
    int threads_amount;
    int queue_size;
    pthread_t* thread_pool;
    bool* isThreadActivated;
    Queue* busyThreads;
    Queue* waitingThreads;

    pthread_cond_t c; // should be initialized
    pthread_mutex_t m; // should be initialized
};

typedef struct ThreadManager ThreadManager;

ThreadManager* ThreadManagerCtor(int threads_amount, int queue_size);

void ThreadManagerDtor(ThreadManager* tm);

void ThreadManagerHandleRequest(ThreadManager* tm, int fd);

#endif //OS3_THREADMANAGER_H
