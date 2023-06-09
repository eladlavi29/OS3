#include "segel.h"
#include "ThreadManager.h"

// HW3: Parse the new arguments too
void getargs(int *port, int *threads, int *queue_size, int *max_size, char* schedalg, int argc, char *argv[])
{
    if (argc < 5) {
	fprintf(stderr, "Usage: %s <port> <threads> <queue_size> <schedalg> <max_size (optional)>\n", argv[0]);
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
    char* schedalg = "";

    getargs(&port, &threads, &queue_size, &max_size, schedalg, argc, argv);

    printf("what's goin on %s\n", schedalg);
    ThreadManager* tm = ThreadManagerCtor(threads, queue_size, max_size, schedalg);
    listenfd = Open_listenfd(port);

    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        ThreadManagerHandleRequest(tm, connfd);
    }

    ThreadManagerDtor(tm);
}


    


 
