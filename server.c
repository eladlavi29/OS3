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

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    //New parameters
    int threads, queue_size, max_size;
    char schedalg[MAXLINE];

    getargs(&port, &threads, &queue_size, &max_size, schedalg, argc, argv);

    // HW3: Create some threads...
    pthread_cond_t*  c = malloc(sizeof(pthread_cond_t));
    Pthread_cond_init(c, NULL);

    printf("START1\n");
    ThreadManager* tm = ThreadManagerCtor(threads, queue_size, schedalg, c);
    printf("START2\n");
    listenfd = Open_listenfd(port);

    printf("START LISTENING\n");

    while (1) {
        //Block overload protocol
        printf("\n\n%d out of %d\n\n", getSize(tm->waitingRequests) + getSize(tm->busyRequests), tm->queue_size);
        if(getSize(tm->waitingRequests) + getSize(tm->busyRequests) >= tm->queue_size && strcmp(schedalg, "block")){
            printf("\n\nHi there man\n\n");
        }

        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        //
        // HW3: In general, don't handle the request in the main thread.
        // Save the relevant info in a buffer and have one of the worker threads
        // do the work.
        //
        ThreadManagerHandleRequest(tm, connfd);
    }

    ThreadManagerDtor(tm);
    free(c);
}


    


 
