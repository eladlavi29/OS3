//
// Created by amirb on 16/06/2023.
//

#ifndef OS3_STRUCTS_H
#define OS3_STRUCTS_H


struct Stats{
    struct timeval arrival_time;
    struct timeval dispatch_interval;
};
typedef struct Stats Stats;

typedef struct node {
    int fd;
    struct node * next;
    Stats* stats;
} node;

typedef struct Queue {
    pthread_cond_t  c; // should be initialized
    pthread_mutex_t m; // should be initialized
    int queue_size;
    node *first;
    node *last;
} Queue;


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

typedef struct exeThreadWrapperStruct{
    ThreadManager* tm;
    int fd;
} exeThreadWrapperStruct;


#endif //OS3_STRUCTS_H
