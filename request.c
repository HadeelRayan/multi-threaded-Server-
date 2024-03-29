
//
// request.c: Does the bulk of the work for the web server.
// 

#include "request.h"

Workers workers;
extern int requests_in_progress;
// requestError(      fd,    filename,        "404",    "Not found", "OS-HW3 Server could not find this file");
void requestError(int fd,struct timeval* arrival_time, struct timeval* dispatch_time,struct requestData* thread_data,char *cause, char *errnum, char *shortmsg, char *longmsg)
{
   char buf[MAXLINE], body[MAXBUF];

   // Create the body of the error message
   sprintf(body, "<html><title>OS-HW3 Error</title>");
   sprintf(body, "%s<body bgcolor=""fffff"">\r\n", body);
   sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
   sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
   sprintf(body, "%s<hr>OS-HW3 Web Server\r\n", body);

   // Write out the header information for this response
   sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Content-Type: text/html\r\n");
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   // sleep(10);
   sprintf(buf, "Content-Length: %lu\r\n\r\n", strlen(body));
    sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, arrival_time->tv_sec, arrival_time->tv_usec);
    sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, dispatch_time->tv_sec, dispatch_time->tv_usec);
    sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, thread_data->thread_id);
    sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, thread_data->thread_count);
    sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, thread_data->thread_static);
    sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n\r\n", buf, thread_data->thread_dynamic);
    Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   // Write out the content
   Rio_writen(fd, body, strlen(body));
   printf("%s", body);

}


//
// Reads and discards everything up to an empty text line
//
void requestReadhdrs(rio_t *rp)
{
   char buf[MAXLINE];

   Rio_readlineb(rp, buf, MAXLINE);
   while (strcmp(buf, "\r\n")) {
      Rio_readlineb(rp, buf, MAXLINE);
   }
   return;
}

//
// Return 1 if static, 0 if dynamic content
// Calculates filename (and cgiargs, for dynamic) from uri
//
int requestParseURI(char *uri, char *filename, char *cgiargs) 
{
   char *ptr;

   if (strstr(uri, "..")) {
      sprintf(filename, "./public/home.html");
      return 1;
   }

   if (!strstr(uri, "cgi")) {
      // static
      strcpy(cgiargs, "");
      sprintf(filename, "./public/%s", uri);
      if (uri[strlen(uri)-1] == '/') {
         strcat(filename, "home.html");
      }
      return 1;
   } else {
      // dynamic
      ptr = index(uri, '?');
      if (ptr) {
         strcpy(cgiargs, ptr+1);
         *ptr = '\0';
      } else {
         strcpy(cgiargs, "");
      }
      sprintf(filename, "./public/%s", uri);
      return 0;
   }
}

//
// Fills in the filetype given the filename
//
void requestGetFiletype(char *filename, char *filetype)
{
   if (strstr(filename, ".html")) 
      strcpy(filetype, "text/html");
   else if (strstr(filename, ".gif")) 
      strcpy(filetype, "image/gif");
   else if (strstr(filename, ".jpg")) 
      strcpy(filetype, "image/jpeg");
   else 
      strcpy(filetype, "text/plain");
}

void requestServeDynamic(int fd,struct timeval* arrival_time, struct timeval* dispatch_time,struct requestData* thread_data, char *filename, char *cgiargs)
{
   char buf[MAXLINE], *emptylist[] = {NULL};

   // The server does only a little bit of the header.  
   // The CGI script has to finish writing out the header.
   sprintf(buf, "HTTP/1.0 200 OK\r\n");
   sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);
    sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, arrival_time->tv_sec, arrival_time->tv_usec);
    sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, dispatch_time->tv_sec, dispatch_time->tv_usec);
    sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, thread_data->thread_id);
    sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, thread_data->thread_count);
    sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, thread_data->thread_static);
    sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n\r\n", buf, thread_data->thread_dynamic);
   Rio_writen(fd, buf, strlen(buf));

   if (Fork() == 0) {
      /* Child process */
      Setenv("QUERY_STRING", cgiargs, 1);
      /* When the CGI process writes to stdout, it will instead go to the socket */
      Dup2(fd, STDOUT_FILENO);
      Execve(filename, emptylist, environ);
   }
   Wait(NULL);
}


void requestServeStatic(int fd,struct timeval* arrival_time, struct timeval* dispatch_time,struct requestData* thread_data, char *filename, int filesize)
{
   int srcfd;
   char *srcp, filetype[MAXLINE], buf[MAXBUF];

   requestGetFiletype(filename, filetype);

   srcfd = Open(filename, O_RDONLY, 0);

   // Rather than call read() to read the file into memory, 
   // which would require that we allocate a buffer, we memory-map the file
   srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
   Close(srcfd);

   // put together response
   sprintf(buf, "HTTP/1.0 200 OK\r\n");
   sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);
   sprintf(buf, "%sContent-Length: %d\r\n", buf, filesize);
   sprintf(buf, "%sContent-Type: %s\r\n\r\n", buf, filetype);
    sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, arrival_time->tv_sec, arrival_time->tv_usec);
    sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, dispatch_time->tv_sec, dispatch_time->tv_usec);
    sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, thread_data->thread_id);
    sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, thread_data->thread_count);
    sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, thread_data->thread_static);
    sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n\r\n", buf, thread_data->thread_dynamic);
   Rio_writen(fd, buf, strlen(buf));

   //  Writes out to the client socket the memory-mapped file 
   Rio_writen(fd, srcp, filesize);
   Munmap(srcp, filesize);

}
int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y)
{
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_usec < y->tv_usec) {
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000) {
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
       tv_usec is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}


// handle a request
void requestHandle(void* data)
{
   // printf("%d",workers->size);

    //printf("got to request handler\n");
    struct timeval arrival_time,dispatch_time,res_time;
    struct requestData* thread_data = (struct requestData*)(data);
    while (1) {
        //printf("before get worker\n");
        pthread_mutex_lock(&workers->mutex);
        int fd = getWorker(workers,&dispatch_time,&arrival_time);
        requests_in_progress ++;
        thread_data->thread_count ++;
        pthread_mutex_unlock(&workers->mutex);


        //printf("%d",thread_data->thread_count);
        //struct timeval res_time;
        timeval_subtract(&res_time, &dispatch_time, &arrival_time);

        int is_static;
        struct stat sbuf;
        char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
        char filename[MAXLINE], cgiargs[MAXLINE];
        rio_t rio;

        Rio_readinitb(&rio, fd);
        Rio_readlineb(&rio, buf, MAXLINE);
        sscanf(buf, "%s %s %s", method, uri, version);

        printf("%s %s %s\n", method, uri, version);
        //pthread_mutex_t curr_mutex;
        if (strcasecmp(method, "GET")) {
            requestError(fd,&arrival_time,&res_time,thread_data, method, "501", "Not Implemented", "OS-HW3 Server does not implement this method");
            pthread_mutex_lock(&workers->mutex);
            requests_in_progress--;
            if(strcmp((thread_data->policy), "block")==0) {
                pthread_cond_signal(&workers->addingAllowed);
            }
            Close(fd);
            pthread_mutex_unlock(&workers->mutex);
            continue;
        }
        requestReadhdrs(&rio);
        is_static = requestParseURI(uri, filename, cgiargs);
        if (stat(filename, &sbuf) < 0) {
            requestError(fd,&arrival_time,&res_time,thread_data, filename, "404", "Not found", "OS-HW3 Server could not find this file");
            pthread_mutex_lock(&workers->mutex);
            requests_in_progress--;
            if(strcmp((thread_data->policy), "block")==0) {
                pthread_cond_signal(&workers->addingAllowed);
            }
            Close(fd);
            pthread_mutex_unlock(&workers->mutex);

            continue;
        }

        if (is_static) {
            if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
                requestError(fd,&arrival_time,&res_time,thread_data, filename, "403", "Forbidden", "OS-HW3 Server could not read this file");
                pthread_mutex_lock(&workers->mutex);
                requests_in_progress--;
                if(strcmp((thread_data->policy), "block")==0) {
                    pthread_cond_signal(&workers->addingAllowed);
                }
                Close(fd);
                pthread_mutex_unlock(&workers->mutex);

                continue;
            }
            thread_data->thread_static ++;
            requestServeStatic(fd,&arrival_time,&res_time,thread_data, filename, sbuf.st_size);
        } else {
            if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
                requestError(fd,&arrival_time,&res_time,thread_data, filename, "403", "Forbidden", "OS-HW3 Server could not run this CGI program");
                pthread_mutex_lock(&workers->mutex);
                requests_in_progress--;
                if(strcmp((thread_data->policy), "block") == 0) {
                    pthread_cond_signal(&workers->addingAllowed);
                }
                Close(fd);
                pthread_mutex_unlock(&workers->mutex);
                continue;
            }
            thread_data->thread_dynamic ++;
            requestServeDynamic(fd, &arrival_time,&res_time,thread_data,filename, cgiargs);
        }
        pthread_mutex_lock(&workers->mutex);
        close(fd);
        requests_in_progress --;
        if(workers->size + requests_in_progress < workers->max_size)
            pthread_cond_signal(&workers->addingAllowed);
        pthread_mutex_unlock(&workers->mutex);
    }
}


