//
// Created by student on 1/2/22.
//
#ifndef WEBSERVER_FILES_SERVER_H
#define WEBSERVER_FILES_SERVER_H
#include "segel.h"
#include "request.h"
#include <pthread.h>
//#include "queue.h"
extern Workers workers;
int requests_in_progress = 0;
void getargs(int *port,int* threads_num, int* queue_size, char** policy, int argc, char *argv[]);

#endif //WEBSERVER_FILES_SERVER_H
