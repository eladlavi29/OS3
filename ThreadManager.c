#include <stdbool.h>

#include "segel.h"
#include "request.h"
#include "Queue.h"
#include "ThreadManager.h"

void* exeThread(void*);

ThreadManager* ThreadManagerCtor(int threads_amount, int queue_size, char* sched_alg){
    ThreadManager* tm = malloc(sizeof(ThreadManager));;
    tm->threads_amount = threads_amount;
    tm->queue_size = queue_size;
    tm->sched_alg = sched_alg;
    Pthread_cond_init(&tm->c, NULL);

    tm->busyRequests = Queue_ctor();
    tm->waitingRequests = Queue_ctor();

    tm->thread_pool = (pthread_t*)malloc(threads_amount * sizeof(pthread_t));
    for(int i = 0; i<threads_amount; i++){
        Pthread_create(&tm->thread_pool[i], NULL, exeThread, (void*)tm);
    }

    return tm;
}

void ThreadManagerDtor(ThreadManager* tm){
    Queue_dtor(tm->busyRequests);
    Queue_dtor(tm->waitingRequests);

    for(int i = 0; i<tm->threads_amount; i++){
        Pthread_cancel(tm->thread_pool[i]);
    }

    free(tm->thread_pool);

    free(tm);
}

void removeThread(ThreadManager* tm, int fd){
    dequeue_by_val(tm->busyRequests, fd);
    Close(fd);

    if(strcmp(tm->sched_alg, BLOCK_SCHEDALG))
        pthread_cond_signal(&tm->c);

    if(strcmp(tm->sched_alg, BLOCK_FLSUH_SCHEDALG) && getSize(tm->waitingRequests) == 0)
        pthread_cond_signal(&tm->c);

    print_queue(tm->waitingRequests);

    exeThread((void*)tm);
}

void* exeThread(void* temp){
    ThreadManager* tm = (ThreadManager*)temp;

    int new_fd = dequeue(tm->waitingRequests);

    enqueue(tm->busyRequests, new_fd);

    requestHandle(new_fd);

    removeThread(tm, new_fd);

    return NULL;
}

void ThreadManagerHandleRequest(ThreadManager* tm, int fd){
    enqueue(tm->waitingRequests, fd);

    //Block overload protocol
    pthread_mutex_t unnecessary_lock;
    pthread_mutex_init(&unnecessary_lock, NULL);
    while(getSize(tm->waitingRequests) + getSize(tm->busyRequests) >= tm->queue_size && strcmp(tm->sched_alg, BLOCK_SCHEDALG)){
        pthread_cond_wait(&tm->c, &unnecessary_lock);
    }
}
















