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
    Pthread_mutex_init(&tm->m, NULL);

    tm->busyRequests = Queue_ctor();
    tm->waitingRequests = Queue_ctor();

    tm->thread_pool = (pthread_t*)malloc(threads_amount * sizeof(pthread_t));
    for(int i = 0; i<threads_amount; i++){
        Pthread_create(&tm->thread_pool[i], NULL, exeThread, (void*)tm);
    }

    srand(time(NULL));   // Initialization, should only be called once.

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
    dequeue_by_val(tm->busyRequests, fd);
    Close(fd);

    //Block protocol
    if(strcmp(tm->sched_alg, BLOCK_SCHEDALG) == 0)
        pthread_cond_signal(&tm->c);

    //Block flush protocol
    if(strcmp(tm->sched_alg, BLOCK_FLUSH_SCHEDALG) == 0 && getSize(tm->waitingRequests) == 0 && getSize(tm->busyRequests) == 0){
        pthread_cond_signal(&tm->c);
    }

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

void dropRandomThread(ThreadManager* tm){
    int waiting_requests[getSize(tm->waitingRequests)];
    printf("\n\n1\n\n");

    getValues(tm->waitingRequests, waiting_requests);
    printf("\n\n2\n\n");

    int removed_request = rand() % getSize(tm->waitingRequests);
    dequeue_by_val(tm->waitingRequests, removed_request);
    Close(removed_request);

    printf("\n\nremoved %d\n\n", removed_request);
}

void ThreadManagerHandleRequest(ThreadManager* tm, int fd){
    //Drop tail protocol
    printf("handling %d\n", fd);

    if(getSize(tm->waitingRequests) + getSize(tm->busyRequests) >= tm->queue_size
        && strcmp(tm->sched_alg, DROP_TAIL_SCHEDALG) == 0){
        printf("Dropped tail %d\n", fd);
        Close(fd);
        return;
    }

    if(getSize(tm->waitingRequests) + getSize(tm->busyRequests) >= tm->queue_size
        && strcmp(tm->sched_alg, DROP_HEAD_SCHEDALG) == 0){
        printf("Dropped head%d\n", fd);
        Close(dequeue(tm->waitingRequests));
    }

    if(getSize(tm->waitingRequests) + getSize(tm->busyRequests) >= tm->queue_size
       && strcmp(tm->sched_alg, DROP_RANDOM_SCHEDALG) == 0){
        printf("waiting queue:\n");
        print_queue(tm->waitingRequests);
        printf("busy queue:\n");
        print_queue(tm->busyRequests);

        pthread_mutex_lock(&tm->waitingRequests->m);

        printf("Dropped random\n");
        int removed_requests_amount = (tm->queue_size - tm->threads_amount + 1) / 2;
        for(int i = 0; i < removed_requests_amount; ++i){
            dropRandomThread(tm);
        }

        pthread_mutex_unlock(&tm->waitingRequests->m);


        printf("waiting queue:\n");
        print_queue(tm->waitingRequests);
        printf("busy queue:\n");
        print_queue(tm->busyRequests);
    }

    //Block and Block flush protocol
    pthread_mutex_lock(&tm->m);
    while(getSize(tm->waitingRequests) + getSize(tm->busyRequests) >= tm->queue_size
          && (strcmp(tm->sched_alg, BLOCK_SCHEDALG) == 0 || strcmp(tm->sched_alg, BLOCK_FLUSH_SCHEDALG) == 0)){
        printf("Block started by %d\n", fd);
        pthread_cond_wait(&tm->c, &tm->m);
    }
    pthread_mutex_unlock(&tm->m);
    enqueue(tm->waitingRequests, fd);
}
















