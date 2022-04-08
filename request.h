#ifndef __REQUEST_H__

#include "queue.h"
#include "segel.h"
struct requestData{
    int thread_id;
    int thread_count;
    int thread_static;
    int thread_dynamic;
    char* policy;
};

int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y);
void requestHandle(void* data);

#endif
