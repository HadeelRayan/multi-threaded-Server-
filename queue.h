//
// Created by student on 1/2/22.
//
#ifndef WEBSERVER_FILES_QUEUE_H
#define WEBSERVER_FILES_QUEUE_H
#include "stdio.h"
#include <pthread.h>
#include "segel.h"
extern int requests_in_progress;
typedef struct node_t{
    int connfd;
    struct timeval arrival_time;
    struct timeval dispatch_time;
    struct node_t* prev;
    struct node_t* next;
}*Node;
typedef struct workers_t{
    Node head;
    Node tail;
    int size;
    int max_size;
    pthread_mutex_t mutex;
    pthread_cond_t addingAllowed;
    pthread_cond_t removingAllowed;
}*Workers;
Workers createWorkersQueue(int max_size);
Node AddWorker(Workers queue,int connfd, struct timeval arrival_time);
int getWorker(Workers queue,struct timeval* dispatch_time,struct timeval* arrival_time);
void RemoveTailWorker(Workers queue);
void RemoveRandomWorker(Workers queue);
void queueDestroy(Workers queue);
#endif //WEBSERVER_FILES_QUEUE_H
