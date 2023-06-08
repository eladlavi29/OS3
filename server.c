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
    schedalg = argv[4];

    if(argc > 5)
        *max_size = atoi(argv[5]);
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    //New parameters
    int threads, queue_size, max_size;
    char* schedalg = "";

    getargs(&port, &threads, &queue_size, &max_size, schedalg, argc, argv);

    // HW3: Create some threads...
    pthread_cond_t*  c = malloc(sizeof(pthread_cond_t));
    Pthread_cond_init(c, NULL);

    ThreadManager* tm = ThreadManagerCtor(threads, queue_size, schedalg, c);
    listenfd = Open_listenfd(port);

    pthread_mutex_t unnecessary_lock;
    pthread_mutex_init(&unnecessary_lock, NULL);
    while (1) {
        //Block overload protocol
        while(getSize(tm->waitingRequests) + getSize(tm->busyRequests) >= tm->queue_size && strcmp(schedalg, BLOCK_SCHEDALG)){
            pthread_cond_wait(c, &unnecessary_lock);
        }

        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        ThreadManagerHandleRequest(tm, connfd);
    }

    ThreadManagerDtor(tm);
    free(c);
}


    


 
