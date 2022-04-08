#include "server.h"
//
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too

void getargs(int *port, int *threads_num, int *queue_size, char **policy, int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *threads_num = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    strcpy(*policy, argv[4]);
}


int main(int argc, char *argv[]) {
    int listenfd, connfd, port, clientlen, threads_num, queue_size;
    char *policy = malloc(sizeof(char) * 12);
    struct sockaddr_in clientaddr;
    getargs(&port, &threads_num, &queue_size, &policy, argc, argv); // port
    int threads_size = atoi(argv[2]);
    pthread_t *threads = malloc(sizeof(pthread_t) * threads_size);
    //printf("before creating threads \n");
    for (unsigned int i = 0; i < threads_size; i++) {
        struct requestData *new_req = (struct requestData *) malloc(sizeof(struct requestData));
        new_req->thread_id = i;
        new_req->thread_count = 0;
        new_req->thread_dynamic = 0;
        new_req->thread_static = 0;
        new_req->policy = malloc(sizeof(char) * 12);
        strcpy(new_req->policy, policy);
        pthread_create(&threads[i], NULL, (void *) requestHandle, (void *) (new_req));
    }
    //printf("after creating threads \n");
    int workers_num = atoi(argv[3]);
    workers = createWorkersQueue(workers_num);
    if (workers == NULL) {
        return 0;
    }
    listenfd = Open_listenfd(port);
    //printf("before while loop \n");
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, (socklen_t * ) & clientlen);
        struct timeval arrival_time;
        //struct timeval dispatch_time;
        arrival_time.tv_sec = 0;
        arrival_time.tv_usec = 0;
        gettimeofday(&arrival_time, NULL);
        //sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, arrival_time->tv_sec, arrival_time->tv_usec);
        pthread_mutex_lock(&workers->mutex);
        if (workers->size + requests_in_progress >= queue_size) {
            if (strcmp(policy, "block") == 0) {
                AddWorker(workers, connfd, arrival_time);
            }
            else if (strcmp(policy, "dh") == 0) {
                RemoveTailWorker(workers);
                AddWorker(workers, connfd, arrival_time);
            }
            else if (strcmp(policy, "random") == 0) {
                RemoveRandomWorker(workers);
                AddWorker(workers, connfd, arrival_time);
            }
            else if (strcmp(policy, "dt") == 0) {
                close(connfd);
            }
        }
        else
            AddWorker(workers, connfd, arrival_time);
        pthread_mutex_unlock(&workers->mutex);
    }

}


    


 
