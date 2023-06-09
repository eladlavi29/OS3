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
#define DYNAMIC_SCHEDALG "dynamic"
#define DROP_TAIL_SCHEDALG "dt"
#define DROP_HEAD_SCHEDALG "dh"
#define DROP_RANDOM_SCHEDALG "random"


struct Stats{
    struct timeval arrival_time;
    struct timeval dispatch_interval;
    struct thread_stats handler_thread_stats;
};
typedef struct Stats Stats;

struct thread_stats{
    int handler_thread_id;
    int handler_thread_req_count;
    int handler_thread_static_req_count;
    int handler_thread_dynamic_req_count;
};
typedef struct thread_stats thread_stats;

struct Request{
    int fd;
    Stats* stats;
};

typedef struct Request Request;

struct ThreadManager{
    int threads_amount;
    int queue_size;
    char* sched_alg;
    pthread_t* thread_pool;
    Queue* busyRequests;
    Queue* waitingRequests;

    pthread_cond_t  c; // should be initialized
    pthread_mutex_t m; // should be initialized

};

typedef struct ThreadManager ThreadManager;




typedef struct exeThreadWrapperStruct{
    ThreadManager* tm;
    int fd;
} exeThreadWrapperStruct;


ThreadManager* ThreadManagerCtor(int threads_amount, int queue_size, int max_size, char* sched_alg);

void ThreadManagerDtor(ThreadManager* tm);

void ThreadManagerHandleRequest(ThreadManager* tm, int fd);

#endif //OS3_THREADMANAGER_H
