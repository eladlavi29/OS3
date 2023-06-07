#include "segel.h"
#include "ThreadManager.h"

// HW3: Parse the new arguments too
void getargs(int *port, int *threads, int *queue_size, int *max_size, char* schedalg, int argc, char *argv[])
{
    if (argc < 5) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *threads = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    strcpy(schedalg, argv[4]);

    if(argc > 5)
        *max_size = atoi(argv[5]);
}

#include "Queue.h"

int main(int argc, char *argv[])
{
    Queue* q = Queue_ctor();
    pthread_t t1 = 1;
    pthread_t t2 = 2;
    pthread_t t3 = 3;
    pthread_t t4 = 4;
    pthread_t t5 = 5;
    print_queue(q);
    enqueue(q,t1);
    enqueue(q,t2);
    enqueue(q,t3);
    enqueue(q,t4);
    enqueue(q,t5);
    print_queue(q);
    /*
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    //New parameters
    int threads, queue_size, max_size;
    char schedalg[MAXLINE];

    getargs(&port, &threads, &queue_size, &max_size, schedalg, argc, argv);

    // HW3: Create some threads...
    ThreadManager* tm = ThreadManagerCtor(threads);

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        //
        // HW3: In general, don't handle the request in the main thread.
        // Save the relevant info in a buffer and have one of the worker threads
        // do the work.
        //
        ThreadManagerHandleRequest(tm, connfd);

        Close(connfd);
    }

    ThreadManagerDtor(tm);
     */
}


    


 
