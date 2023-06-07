#include "segel.h"
#include "request.h"
#include "ThreadManager.h"

ThreadManager* ThreadManagerCtor(int threads_amount){
    ThreadManager* tm = malloc(sizeof(ThreadManager));;
    tm->m_threads_amount = threads_amount;
    tm->thread_pool = (pthread_t*)malloc(threads_amount * sizeof(pthread_t));

    return tm;
}

void ThreadManagerDtor(ThreadManager* tm){
    free(tm->thread_pool);
}

void ThreadManagerHandleRequest(ThreadManager* tm, int fd){
    requestHandle(fd);
}