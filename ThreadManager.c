#include <stdbool.h>

#include "segel.h"
#include "request.h"
#include "Queue.h"
#include "ThreadManager.h"

void* exeThread(void*);

ThreadManager* ThreadManagerCtor(int threads_amount, int queue_size){
    ThreadManager* tm = malloc(sizeof(ThreadManager));;
    tm->threads_amount = threads_amount;
    tm->queue_size = queue_size;

    tm->busyRequests = Queue_ctor();
    tm->waitingRequests = Queue_ctor();

    Pthread_cond_init(&tm->c, NULL);
    Pthread_mutex_init(&tm->m, NULL);

    printf("HERE1\n");

    tm->thread_pool = (pthread_t*)malloc(threads_amount * sizeof(pthread_t));
    for(int i = 0; i<threads_amount; i++){
        Pthread_create(&tm->thread_pool[i], NULL, exeThread, (void*)tm);
    }

    printf("HERE2\n");

    return tm;
}

void ThreadManagerDtor(ThreadManager* tm){
    Queue_dtor(tm->busyRequests);
    Queue_dtor(tm->waitingRequests);

    for(int i = 0; i<tm->threads_amount; i++){
        Pthread_cancel(tm->thread_pool[i]);
    }

    free(tm->thread_pool);

    Pthread_cond_destroy(&tm->c);
    Pthread_mutex_destroy(&tm->m);

    free(tm);
}

void removeThread(ThreadManager* tm, int fd){
    pthread_mutex_lock(&tm->m);

    dequeue_by_val(tm->busyRequests, fd);
    Close(fd);

    pthread_mutex_unlock(&tm->m);
    exeThread((void*)tm);
}

void* exeThread(void* temp){
    ThreadManager* tm = (ThreadManager*)temp;

    pthread_mutex_lock(&tm->m);

    int new_fd = dequeue(tm->waitingRequests);

    printf("A THREAD WOKE UP\n");

    enqueue(tm->busyRequests, new_fd);

    printf("THE THREAD GOT THE ASSIGNMENT\n");

    pthread_mutex_unlock(&tm->m);

    printf("REQUEST WILL BE HANDLED RIGHT NOW...\n");

    requestHandle(new_fd);

    printf("REQUEST WAS HANDLED...\n");

    removeThread(tm, new_fd);

    return NULL;
}

void ThreadManagerHandleRequest(ThreadManager* tm, int fd){
    printf("ThreadManagerHandleRequest\n");
    enqueue(tm->waitingRequests, fd);
}