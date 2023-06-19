#include <stdbool.h>

#include "ThreadManager.h"

void* exeThread(void*);

ThreadManager* ThreadManagerCtor(int threads_amount, int queue_size, int max_size, char* sched_alg){
    ThreadManager* tm = malloc(sizeof(ThreadManager));
    tm->threads_amount = threads_amount;

    printf("sched_alg in Constructor: %s", sched_alg);

    //Dynamic protocol
    if(strcmp(sched_alg, DYNAMIC_SCHEDALG) == 0){
        tm->queue_size = max_size;
        tm->queue_size_dynamic = queue_size;
    }
    else{
        tm->queue_size = queue_size;
        tm->queue_size_dynamic = queue_size;

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

    pthread_cond_init(&tm->c, NULL);
    pthread_mutex_init(&tm->m, NULL);

    tm->busyRequests = Queue_ctor();
    tm->waitingRequests = Queue_ctor();

    tm->thread_pool = (pthread_t*)malloc(threads_amount * sizeof(pthread_t));
    tm->thread_arr = (Thread*)malloc(threads_amount * sizeof(Thread));
    for(int i = 0; i<threads_amount; i++){
        pthread_create(&tm->thread_pool[i], NULL, exeThread, (void*)tm);
        tm->thread_arr[i].thread = tm->thread_pool[i];
        tm->thread_arr[i].dynamic_req_count = 0;
        tm->thread_arr[i].static_req_count = 0;
        tm->thread_arr[i].req_count = 0;
        tm->thread_arr[i].thread_id = i;
    }

    srand(time(NULL));   // Initialization, should only be called once.

    return tm;
}

void ThreadManagerDtor(ThreadManager* tm){
    Queue_dtor(tm->busyRequests);
    Queue_dtor(tm->waitingRequests);

    for(int i = 0; i<tm->threads_amount; i++){
        pthread_cancel(tm->thread_pool[i]);
    }

    free(tm->thread_pool);

    pthread_cond_destroy(&tm->c);
    pthread_mutex_destroy(&tm->m);

    free(tm);
}

void removeThread(ThreadManager* tm, int fd){
    dequeue_by_val(tm->busyRequests, fd);
    Close(fd);

    //Block flush protocol
    pthread_mutex_lock(&tm->m);
    pthread_mutex_lock(&tm->busyRequests->m);
    pthread_mutex_lock(&tm->waitingRequests->m);
    if(strcmp(tm->sched_alg, BLOCK_FLUSH_SCHEDALG) == 0 && getSize(tm->waitingRequests) == 0 && getSize(tm->busyRequests) == 0){
        pthread_cond_signal(&tm->c);
    }
    pthread_mutex_unlock(&tm->busyRequests->m);
    pthread_mutex_unlock(&tm->waitingRequests->m);
    pthread_mutex_unlock(&tm->m);

    //Block protocol
    if(strcmp(tm->sched_alg, BLOCK_SCHEDALG) == 0)
        pthread_cond_signal(&tm->c);

    exeThread((void*)tm);
}

void timeval_subtract(struct timeval *elapsed, struct timeval *pickup, struct timeval *arrival)
{
    elapsed->tv_sec = pickup->tv_sec - arrival->tv_sec;

    if ((elapsed->tv_usec = pickup->tv_usec - arrival->tv_usec) < 0)
    {
        elapsed->tv_usec += 1000000;
        elapsed->tv_sec--;
    }

    return;
}

void* exeThread(void* temp){
    ThreadManager* tm = (ThreadManager*)temp;

    Request * r = dequeue(tm->waitingRequests);
    int new_fd = r->fd;
    Stats* stats = r->stats;
    free(r);

    struct timeval *pickup_time = malloc(sizeof(struct timeval));
    gettimeofday(pickup_time, NULL);

    timeval_subtract(&stats->dispatch_interval, pickup_time, &stats->arrival_time);

    free(pickup_time);

    enqueue(tm->busyRequests, new_fd, stats);

    requestHandle(new_fd, stats, tm);

    removeThread(tm, new_fd);

    return NULL;
}

void dropRandomThread(ThreadManager* tm){
    int waiting_requests[getSize(tm->waitingRequests)];
    getValues(tm->waitingRequests, waiting_requests);

    int removed_request = rand() % getSize(tm->waitingRequests);
    unlocked_dequeue_by_val(tm->waitingRequests, waiting_requests[removed_request]);
    Close(waiting_requests[removed_request]);
}

void ThreadManagerHandleRequest(ThreadManager* tm, int fd, Stats* stats){
    printf("START\n");
    printf("sched_alg: %s\n", tm->sched_alg);

    pthread_mutex_lock(&tm->m);
    pthread_mutex_lock(&tm->busyRequests->m);
    pthread_mutex_lock(&tm->waitingRequests->m);
    //Drop tail protocol
    if(getSize(tm->waitingRequests) + getSize(tm->busyRequests) >= tm->queue_size
        && strcmp(tm->sched_alg, DROP_TAIL_SCHEDALG) == 0){
        Close(fd);
        pthread_mutex_unlock(&tm->busyRequests->m);
        pthread_mutex_unlock(&tm->waitingRequests->m);
        pthread_mutex_unlock(&tm->m);
        return;
    }

    //Drop head protocol
    if(getSize(tm->waitingRequests) + getSize(tm->busyRequests) >= tm->queue_size
        && strcmp(tm->sched_alg, DROP_HEAD_SCHEDALG) == 0){
        pthread_mutex_unlock(&tm->waitingRequests->m);
        pthread_mutex_unlock(&tm->m);
        Request * r = dequeue(tm->waitingRequests);
        pthread_mutex_lock(&tm->m);
        pthread_mutex_lock(&tm->waitingRequests->m);
        Close(r->fd);
        free(r->stats);
        free(r);
    }

    //Dynamic protocol
    if(getSize(tm->waitingRequests) + getSize(tm->busyRequests) >= tm->queue_size_dynamic
       && strcmp(tm->sched_alg, DYNAMIC_SCHEDALG) == 0){
        printf("%s","HERE0\n");
        Close(fd);
        printf("%s","HERE1\n");
        if(tm->queue_size_dynamic < tm->queue_size){
            tm->queue_size_dynamic = tm->queue_size_dynamic + 1;
        }
        if(tm->queue_size_dynamic == tm->queue_size){
            tm->sched_alg = DROP_TAIL_SCHEDALG;
        }
        pthread_mutex_unlock(&tm->busyRequests->m);
        pthread_mutex_unlock(&tm->waitingRequests->m);
        pthread_mutex_unlock(&tm->m);

        printf("%s","HERE2\n");

        return;
    }

    //Drop random protocol
    if(getSize(tm->waitingRequests) + getSize(tm->busyRequests) >= tm->queue_size
       && strcmp(tm->sched_alg, DROP_RANDOM_SCHEDALG) == 0){

        int removed_requests_amount = (tm->queue_size - tm->threads_amount + 1) / 2;
        for(int i = 0; i < removed_requests_amount; ++i){
            dropRandomThread(tm);
        }
    }

    //Block and Block flush protocol
    while(getSize(tm->waitingRequests) + getSize(tm->busyRequests) >= tm->queue_size
          && (strcmp(tm->sched_alg, BLOCK_SCHEDALG) == 0)){
        pthread_mutex_unlock(&tm->busyRequests->m);
        pthread_mutex_unlock(&tm->waitingRequests->m);
        pthread_cond_wait(&tm->c, &tm->m);
        pthread_mutex_lock(&tm->busyRequests->m);
        pthread_mutex_lock(&tm->waitingRequests->m);
    }

    while(getSize(tm->waitingRequests) + getSize(tm->busyRequests) >= tm->queue_size
          && (strcmp(tm->sched_alg, BLOCK_FLUSH_SCHEDALG) == 0)){
        pthread_mutex_unlock(&tm->busyRequests->m);
        pthread_mutex_unlock(&tm->waitingRequests->m);
        pthread_cond_wait(&tm->c, &tm->m);
        pthread_mutex_unlock(&tm->m);
        Close(fd);
        return;
    }

    pthread_mutex_unlock(&tm->busyRequests->m);
    pthread_mutex_unlock(&tm->waitingRequests->m);
    pthread_mutex_unlock(&tm->m);

    printf("%s","END\n");

    enqueue(tm->waitingRequests, fd,stats);
}
















