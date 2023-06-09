#include <stdbool.h>

#include "segel.h"
#include "request.h"
#include "Queue.h"
#include "ThreadManager.h"

void* exeThread(void*);

ThreadManager* ThreadManagerCtor(int threads_amount, int queue_size, int max_size, char* sched_alg){
    ThreadManager* tm = malloc(sizeof(ThreadManager));;
    tm->threads_amount = threads_amount;

    //Dynamic protocol
    if(strcmp(sched_alg, DYNAMIC_SCHEDALG) == 0){
        printf("wait what\n");
        tm->queue_size = max_size;
        tm->sched_alg = DROP_TAIL_SCHEDALG;
    }
    else{
        tm->queue_size = queue_size;

        if(strcmp(sched_alg, BLOCK_SCHEDALG) == 0 ||
            strcmp(sched_alg, BLOCK_FLUSH_SCHEDALG) == 0 ||
            strcmp(sched_alg, DROP_TAIL_SCHEDALG) == 0 ||
            strcmp(sched_alg, DROP_HEAD_SCHEDALG) == 0 ||
            strcmp(sched_alg, DROP_RANDOM_SCHEDALG) == 0)
            tm->sched_alg = sched_alg;
        else
            //Default
            fprintf(stderr, "Invalid overload protocol\n");
    }

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

    //Block protocol
    if(strcmp(tm->sched_alg, BLOCK_SCHEDALG) == 0)
        pthread_cond_signal(&tm->c);

    //Block flush protocol
    if(strcmp(tm->sched_alg, BLOCK_FLUSH_SCHEDALG) == 0 && getSize(tm->waitingRequests) == 0){
        pthread_cond_signal(&tm->c);
    }

    printf("waiting queue:\n");
    print_queue(tm->waitingRequests);
    printf("busy queue:\n");
    print_queue(tm->busyRequests);

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
    //Drop tail protocol
    printf("handling %d\n", fd);

    if(getSize(tm->waitingRequests) + getSize(tm->busyRequests) >= tm->queue_size
          && strcmp(tm->sched_alg, DROP_TAIL_SCHEDALG) == 0){
        printf("Dropped %d\n", fd);
        Close(fd);
        return;
    }

    enqueue(tm->waitingRequests, fd);

    //Block and Block flush overload protocol
    pthread_mutex_t unnecessary_lock;
    pthread_mutex_init(&unnecessary_lock, NULL);
    printf("sum size: %d, queue size: %d \n", getSize(tm->waitingRequests) + getSize(tm->busyRequests), tm->queue_size);
    while(getSize(tm->waitingRequests) + getSize(tm->busyRequests) >= tm->queue_size
        && (strcmp(tm->sched_alg, BLOCK_SCHEDALG) == 0 || strcmp(tm->sched_alg, BLOCK_FLUSH_SCHEDALG) == 0)){
        printf("Block started by %d\n", fd);
        pthread_cond_wait(&tm->c, &unnecessary_lock);
    }
}
















