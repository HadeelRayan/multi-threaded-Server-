//
// Created by student on 1/8/22.
//
#include "queue.h"

Workers createWorkersQueue(int max_size){
    if(max_size <= 0){
        return NULL;
    }
    Workers new_queue = malloc(sizeof(*new_queue));
    if(new_queue == NULL){
        return NULL;
    }
    pthread_mutex_init(&new_queue->mutex,NULL);
    pthread_cond_init(&new_queue->addingAllowed,NULL);
    pthread_cond_init(&new_queue->removingAllowed,NULL);
    new_queue->max_size = max_size;
    new_queue->size = 0;
    new_queue->head = NULL;
    new_queue->tail = NULL;
    return new_queue;
}

Node AddWorker(Workers queue,int connfd, struct timeval arrival_time){
    if(queue == NULL){
        return NULL;
    }
    //pthread_mutex_lock(&queue->mutex);
    Node new_node = malloc(sizeof(*new_node));
    if(new_node == NULL){
        return NULL;
    }
    new_node->connfd = connfd;
    new_node->arrival_time.tv_usec = arrival_time.tv_usec;
    new_node->arrival_time.tv_sec = arrival_time.tv_sec;
    new_node->dispatch_time.tv_sec = 0;
    new_node->dispatch_time.tv_usec = 0;
    while(queue->size + requests_in_progress >= queue->max_size){
        pthread_cond_wait(&queue->addingAllowed,&queue->mutex);
    }
    if(queue->head == NULL) {//first_node
        queue->head = new_node;
        new_node->next = NULL;
        new_node->prev = NULL;
        queue->tail = new_node;
    }
    else{
        new_node->next = queue->head;
        queue->head ->prev = new_node;
        queue->head = new_node;
    }
    queue->size++;
    pthread_cond_signal(&queue->removingAllowed);
    //pthread_mutex_unlock(&queue->mutex);
    return new_node;
}

int getWorker(Workers queue,struct timeval* dispatch_time,struct timeval* arrival_time){
    //printf("In get worker\n");
    if(queue == NULL){
        return -1;
    }
    //pthread_mutex_lock(&queue->mutex);
    int data;
    while(queue->size <= 0 ){ //check if queue empty
        pthread_cond_wait(&queue->removingAllowed,&queue->mutex);
    }
    //printf("0000\n");
    Node temp = queue->tail;
    data = temp -> connfd;
    if(queue->size != 1) {
        temp->prev->next = NULL;
        queue->tail = temp->prev;
    }
    else{
        queue->head = NULL;
        queue->tail = NULL;
    }
    gettimeofday(&(temp->dispatch_time),NULL);
    //struct timeval temp_tim;
    dispatch_time->tv_sec = temp->dispatch_time.tv_sec;
    dispatch_time->tv_usec = temp->dispatch_time.tv_usec;
    //dispatch_time = &temp_tim;
    //struct timeval temp2_tim;
    arrival_time->tv_sec = temp->arrival_time.tv_sec;
    arrival_time->tv_usec = temp->arrival_time.tv_usec;
    //arrival_time = &temp2_tim;
    free(temp);
    queue->size--;
    if(queue->size + requests_in_progress < queue->max_size)
        pthread_cond_signal(&queue->addingAllowed);
    //pthread_mutex_unlock(&queue->mutex);
    return data;
}
void RemoveTailWorker(Workers queue){
    if(queue == NULL){
        return ;
    }
    //pthread_mutex_lock(&queue->mutex);
    int data;
    while(queue->size <= 0 ){ //check if queue empty
        pthread_cond_wait(&queue->removingAllowed,&queue->mutex);
    }
    Node temp = queue->tail;
    data = temp -> connfd;
    if(queue->size != 1) {
        temp->prev->next = NULL;
        queue->tail = temp->prev;
    }
    else{
        queue->head = NULL;
        queue->tail = NULL;
    }
    close(data);
    free(temp);
    queue->size--;
    pthread_cond_signal(&queue->addingAllowed);
    //pthread_mutex_unlock(&queue->mutex);
}
void RemoveRandomWorker(Workers queue){
    if(queue == NULL){
        return ;
    }
    //pthread_mutex_lock(&queue->mutex);
    int data;
    while(queue->size <= 0 ){ //check if queue empty
        pthread_cond_wait(&queue->removingAllowed,&queue->mutex);
    }
    int random_num = rand() % queue->size;
    Node temp = queue->head;
    for (int i = 0; i < random_num; ++i) {
        if(temp->next != NULL)
            temp = temp->next;
    }

    if(temp == queue->head && temp == queue->tail){ ///// temp is the only element
        queue->head = NULL;
        queue->tail = NULL;
    }
    else if(temp == queue->head && temp != queue->tail){  ///// temp at first
        queue->head = temp->next;
        temp->next->prev = NULL;
    }
    else if (temp == queue->tail && temp != queue->head){ ///// temp at last
        queue->tail = temp->prev;
        temp->prev->next = NULL;
    }
    else if(temp != queue->head && temp != queue->tail){//////temp in the middle
        temp->next->prev = temp->prev;
        temp->prev->next = temp->next;
    }

    data = temp -> connfd;
    close(data);
    free(temp);
    queue->size--;
    pthread_cond_signal(&queue->addingAllowed);
    //pthread_mutex_unlock(&queue->mutex);
}

void queueDestroy(Workers queue){
    pthread_cond_destroy(&queue->addingAllowed);
    pthread_cond_destroy(&queue->removingAllowed);
    pthread_mutex_destroy(&queue->mutex);
    free(queue);
}