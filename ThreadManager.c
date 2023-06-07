#include "segel.h"
#include "request.h"
#include "Queue.h"
#include "ThreadManager.h"

ThreadManager* ThreadManagerCtor(int threads_amount){
    ThreadManager* tm = malloc(sizeof(ThreadManager));;
    tm->m_threads_amount = threads_amount;
    tm->thread_pool = (pthread_t*)malloc(threads_amount * sizeof(pthread_t));
    tm->busyThreads = Queue_ctor();
    tm->waitingThreads = Queue_ctor();

    return tm;
}

void ThreadManagerDtor(ThreadManager* tm){
    Queue_dtor(tm->busyThreads);
    Queue_dtor(tm->waitingThreads);
    free(tm->thread_pool);
}

void addThread(ThreadManager* tm){

}

void removeThread(ThreadManager* tm){

}

void ThreadManagerHandleRequest(ThreadManager* tm, int fd){
    addThread(tm);
    requestHandle(fd);
    removeThread(tm);
}