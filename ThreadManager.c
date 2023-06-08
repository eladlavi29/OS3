#include <stdbool.h>

#include "segel.h"
#include "request.h"
#include "Queue.h"
#include "ThreadManager.h"

ThreadManager* ThreadManagerCtor(int threads_amount, int queue_size){
    ThreadManager* tm = malloc(sizeof(ThreadManager));;
    tm->threads_amount = threads_amount;
    tm->queue_size = queue_size;
    tm->isThreadActivated = malloc(threads_amount * sizeof(bool));
    for(int i = 0; i < threads_amount; ++i){
        tm->isThreadActivated[i] = false;
    }

    tm->thread_pool = (pthread_t*)malloc(threads_amount * sizeof(pthread_t));
    tm->busyThreads = Queue_ctor();
    tm->waitingThreads = Queue_ctor();

    Pthread_cond_init(&tm->c, NULL);
    Pthread_mutex_init(&tm->m, NULL);

    return tm;
}

void ThreadManagerDtor(ThreadManager* tm){
    Queue_dtor(tm->busyThreads);
    Queue_dtor(tm->waitingThreads);
    free(tm->isThreadActivated);
    free(tm->thread_pool);
    free(tm);
}

void exeThread(ThreadManager*, int);
void removeThread(ThreadManager* tm, int fd){
    pthread_mutex_lock(&tm->m);

    dequeue_by_val(tm->busyThreads, fd);

    if(getSize(tm->waitingThreads) > 0){
        int new_fd = dequeue(tm->waitingThreads);
        enqueue(tm->busyThreads, new_fd);

        pthread_mutex_unlock(&tm->m);
        exeThread(tm, new_fd);
    }
    else{
        int currThreadInd = -1;
        for(int i = 0; currThreadInd == -1; ++i){
            if(Pthread_equal(Pthread_self(), tm->thread_pool[i]))
                currThreadInd = i;
        }
        tm->isThreadActivated[currThreadInd] = false;

        pthread_mutex_unlock(&tm->m);
        Pthread_exit(NULL);
    }
}

void exeThread(ThreadManager* tm, int fd){
    printf("Handling request %d \n", fd);

    requestHandle(fd);
    removeThread(tm, fd);
}

void* exeThreadWrapper(void* arg){
    exeThreadWrapperStruct temp = *(struct exeThreadWrapperStruct*) arg;

    exeThread(temp.tm, temp.fd);

    return NULL;
}

void ThreadManagerHandleRequest(ThreadManager* tm, int fd){
    if(getSize(tm->busyThreads) < tm->threads_amount) {

        printf("There is a thread available\n");

        int availableThread = -1;
        for(int i = 0; availableThread == -1; ++i){
            if(!tm->isThreadActivated[i]){
                availableThread = i;
            }
        }

        printf("Thread #%d available\n", availableThread);

        tm->isThreadActivated[availableThread] = true;
        enqueue(tm->busyThreads, fd);
        exeThreadWrapperStruct args;
        args.tm = tm;
        args.fd = fd;
        Pthread_create(&tm->thread_pool[availableThread], NULL, exeThreadWrapper, (void*)&args);

        printf("busyThreads Queue:\n");
        print_queue(tm->busyThreads);

    }
    else{
        //Check Overload

        printf("There is no thread available\n");

        enqueue(tm->waitingThreads, fd);

        printf("waitingThreads Queue:\n");
        print_queue(tm->waitingThreads);

    }
}