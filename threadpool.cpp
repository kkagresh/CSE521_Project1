#include "Threadpool_utils.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <iostream>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//For threads to synchonize queue access

struct threadpool* threadpool_init(int noofthreads)
{
    int i;
    struct threadpool *tp;
    tp=(struct threadpool*)malloc(sizeof(struct threadpool));
    tp->NoOfThreads=noofthreads;
    tp->thread_ptr=(pthread_t*)malloc(sizeof(pthread_t));
    tp->job_queue=((struct th_job_queue*)malloc(sizeof(struct th_job_queue)));
    tp->job_queue->NoOfJobs=0;
    tp->job_queue->front=NULL;
    tp->job_queue->rear=NULL;
    tp->job_queue->sem_queue=((sem_t *)malloc(sizeof(sem_t)));
    sem_init(tp->job_queue->sem_queue,0,0);
    
    for(i=0;i<noofthreads;i++)
    {
        pthread_create(&(tp->thread_ptr[i]),0,request_assign,tp);
    }
    
    return tp;
}
void *request_assign(void *threadpool)
{
    struct threadpool *tp= (struct threadpool *)threadpool;
    printf("In request assign\n");
    while(1)
    {
        if(sem_wait(tp->job_queue->sem_queue))
        {
            printf("semaphore_wait in request assign");
            exit(1);
        }
	   //break 3;
        printf("In request assign:thread pool alive\n");
        struct job_details *parameter;
        parameter=(struct job_details*) malloc(sizeof(struct job_details));
        struct th_job *job; 
        job=(struct th_job*) malloc(sizeof(struct th_job));
        pthread_mutex_lock(&mutex);
        job=tp->job_queue->front;
        parameter=job->param;
        remove_job_queue(tp);
        pthread_mutex_unlock(&mutex);
        printf("in request assign:%s",parameter->request);
        request_handle(parameter);//from server.c -to do parameter is the job variable from scheduler.h
        //free(job);
        //free(parameter);
    }
}
void threadpool_assign_task(struct threadpool *tp,struct job_details *parameter)
{
    printf("entered thread pool assign task\n");
    struct th_job* job;
    printf("before malloc");
    job=(struct th_job*) malloc(sizeof(struct th_job));
    printf("Working till here");
    job->param=parameter;
    printf("in tp assign task:%s",job->param->request);
    pthread_mutex_lock(&mutex);
    add_job_queue(tp,job);
    pthread_mutex_unlock(&mutex);
    //free(job->link);
    //free(job->param);
    //return;
}
void add_job_queue(struct threadpool *tp,struct th_job *job)
{
    printf("entered add_job_queue\n");
    job->link=NULL;
    int sem_value;
    struct th_job* first;
    first= (struct th_job*)malloc(sizeof(struct th_job));
    first=tp->job_queue->front;
    if(tp->job_queue->NoOfJobs==0)
    {
        
        tp->job_queue->front=job;
        tp->job_queue->rear=job;
        printf("Added in 2nd queue:%s\n",tp->job_queue->front->param->request);
    }
    else
    {
        tp->job_queue->rear->link=job;
        tp->job_queue->rear=job;
    }
    printf("okay\n");
    tp->job_queue->NoOfJobs++;
    printf("number of jobs:%d\n",tp->job_queue->NoOfJobs);
    sem_post(tp->job_queue->sem_queue);
    sem_getvalue((tp->job_queue->sem_queue),&sem_value);
}
void remove_job_queue(struct threadpool *tp)
{   
    int sem_value;
    printf("remove job queue");

    if(tp->job_queue->NoOfJobs==1)
    {
        printf("Problem here");
        tp->job_queue->front=NULL;
        tp->job_queue->rear=NULL;
    }
    else
    {
        tp->job_queue->front=tp->job_queue->front->link;
    }
    tp->job_queue->NoOfJobs--;
    sem_getvalue((tp->job_queue->sem_queue),&sem_value);
}
void threadpool_finish(struct threadpool* tp)
{
    int i;
    for(i=0;i<tp->NoOfThreads;i++)
    {
        if(sem_post(tp->job_queue->sem_queue))
        {
            //releases all threads waiting for semaphore but has no job
        }
    }
    if(sem_destroy(tp->job_queue->sem_queue)==0)
    {
        //semaphore destroyed
    }
    for(i=0;i<tp->NoOfThreads;i++)
    {
        pthread_join(tp->thread_ptr[i],0);//Wait for all the threads to finish
    }
    struct th_job* job_at_hand=tp->job_queue->front;
    for(i=0;i<tp->job_queue->NoOfJobs;i++)
    {
        tp->job_queue->front=tp->job_queue->front->link;
        job_at_hand->link=NULL;
        free(job_at_hand);
        job_at_hand=tp->job_queue->front;
        tp->job_queue->NoOfJobs--;
    }
    tp->job_queue->front=NULL;
    tp->job_queue->rear=NULL;
    free(tp->thread_ptr);
    free(tp->job_queue->sem_queue);
    free(tp->job_queue);
    free(tp);    
}

void request_handle(struct job_details *parameter)
{
    printf("In request handle\n");
    struct job_details *job_request;
    job_request=(struct job_details*) malloc(sizeof(struct job_details));
    job_request=parameter;
    char *filename,*filetype,*method_name;
    char temp_buffer[1024];
    char output_buffer[1024],input_buffer[1024];
    int client_socket=job_request->client_socket;
    int buffer_length;
    int fd,filesize;
    filesize=job_request->filesize;
    strcpy(input_buffer,job_request->request);
    printf("%s",input_buffer);
    printf("%d",filesize);
    printf("%d",client_socket);
    strcpy(temp_buffer,input_buffer);
    strtok(input_buffer," ");
    filename = strtok(NULL," ");
    method_name = strtok(temp_buffer," ");//GET or HEAD
    fd=open(&filename[1],O_RDONLY,S_IREAD | S_IWRITE);
    strtok(filename,".");
    filetype = strtok(NULL," ");
    printf("%s\n",filetype);
    printf("%s\n",filename);
    printf("%s\n",method_name);
    if(strcmp(method_name,"GET")==0)
    {
        if(fd==-1)//write the directory code here
        { 
            strcpy(output_buffer,"HTTP/1.0 404 NotFound\r\nContent-Type:text/html\n\n");
            send(client_socket,output_buffer,strlen(output_buffer),0);
            printf("%s",output_buffer);
            strcpy(output_buffer,"<html><body><h1>FILE NOT FOUND</h1></body></html>");

            send(client_socket,output_buffer,strlen(output_buffer),0);
            printf("%s",output_buffer);
        }
        else if((strcmp(filetype,"jpeg")==0) | (strcmp(filetype,"jpg")==0) | (strcmp(filetype,"gif")==0) | (strcmp(filetype,"png")==0))
        {   
            //image file needs to be handled
            printf("image File found 200");
            strcpy(output_buffer,"HTTP/1.0 200 OK\nContent-Type:image/gif\n\n");
            send(client_socket,output_buffer,strlen(output_buffer),0);

            buffer_length=1;
            while(buffer_length>0)
            {       printf("Image File Found");
                buffer_length=read(fd,output_buffer,sizeof(output_buffer));
                if(buffer_length>0)
                {
                    send(client_socket,output_buffer,strlen(output_buffer),0);

                }
            }
        }   
        else 
        {   
            printf("Text File found 200\n");
            strcpy(output_buffer,"HTTP/1.0 200 OK\nContent-Type:text/html\n\n");
            send(client_socket,output_buffer,strlen(output_buffer),0);

            buffer_length=1;
            while(buffer_length>0)
            {       printf("Text File Found\n");
                buffer_length=read(fd,output_buffer,sizeof(output_buffer));
                if(buffer_length>0)
                {
                    send(client_socket,output_buffer,strlen(output_buffer),0);

                }
            }
            printf("done\n");
        }
    }
    else if(strcmp(method_name,"HEAD")==0)
    {
        strcpy(output_buffer,"HTTP/1.0 200 OK\nServer:myhhtd\nContent-Type:text/html\nContent-Length:0\r\n\n");
        send(client_socket,output_buffer,strlen(output_buffer),0);
    }
        //printf("server exiting");
        close(fd);
        close(client_socket);

}
