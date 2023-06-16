#ifndef __REQUEST_H__

#include "segel.h"
#include "Queue.h"

struct ThreadManager{
    int threads_amount;
    int queue_size;
    int queue_size_dynamic;
    char* sched_alg;
    pthread_t* thread_pool;
    Queue* busyRequests;
    Queue* waitingRequests;
    Thread* thread_arr;

    pthread_cond_t  c; // should be initialized
    pthread_mutex_t m; // should be initialized

};

typedef struct ThreadManager ThreadManager;

struct Stats{
    struct timeval arrival_time;
    struct timeval dispatch_interval;
};
typedef struct Stats Stats;


struct Request{
    int fd;
    Stats* stats;
};
typedef struct Request Request;

struct Thread{
    pthread_t thread;
    int thread_id;
    int static_req_count;
    int dynamic_req_count;
    int req_count;
};
typedef struct Thread Thread;

void requestHandle(int fd, Stats* stats, ThreadManager* tm);

#endif
