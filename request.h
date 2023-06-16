#ifndef __REQUEST_H__

#include "segel.h"
#include "ThreadManager.h"


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

void requestHandle(int fd, Stats* stats,struct ThreadManager* tm);

#endif
