/* 
 * File:   Server.c
 * Author: Raghu
 *
 * Created on October 14, 2012, 3:37 AM
 */
#include "scheduler.h"
#include "Threadpool_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
/*
 * 
 */
#define TRUE 1


int t=60;
int thread_pool_size=4;
void* scheduler_thread_func(void *arg)
{
    //break 1;
    struct threadpool* tp=(struct threadpool*)arg;
    struct job_details* job_info=NULL;
    job_info=(struct job_details*) malloc(sizeof(struct job_details));
    sleep(t);
    while(1)
    {
    printf("entered scheduler\n");
    job scheduled_job=scheduler_retrive_job();
    job_info->filesize=scheduled_job.cmp;
    job_info->client_socket=scheduled_job.clientsocket;
    job_info->request=scheduled_job.request;
    printf("front in scheduler:%s\n",job_info->request);
    printf("assigning job in scheduler\n");
    threadpool_assign_task(tp,job_info);
    }
}                                                 
 
void* listener_thread_func(void* arg)
{
    char input_buffer[2048],output_buffer[2048];
    //break 1;
    int client_socket,fd,buffer_length;
    socklen_t length;
    int server_socket= *((int *) arg);
    long filesize;
    char *filename,*filetype,*method_name;
    char temp_buffer[1024];
    struct sockaddr_in client_address;
    length=sizeof(client_address);
    //int *server_socket=(int *)arg;
    printf("entered listener thread func \n");
    while(TRUE)
    {
    printf("entered listener thread func:inside while \n");
    client_socket=accept(server_socket,(struct sockaddr*) &client_address,&length);//accept connection
    printf("%d",client_socket);
    if(read(client_socket,input_buffer,sizeof(input_buffer))==-1)
    {
        printf("problem here");//receive request
    }
    //temp_buffer=input_buffer;
    //printf("input_buffer,initial value:%s\n",input_buffer);
    strcpy(temp_buffer,input_buffer);
    //printf("input_buffer,before:%s\n",input_buffer);
    //printf("temp_buffer,before:%s\n",temp_buffer);
    method_name=strtok(temp_buffer," ");//Get Method
    
    //printf("input_buffer,after:%s\n",input_buffer);
    //printf("temp_buffer,after:%s\n",temp_buffer);
    if(strcmp(method_name,"GET")==0)
    {
        filename=strtok(NULL," ");//Get filename
        printf("%s\n",filename);
        //fd=open(&filename[1],O_RDONLY,S_IREAD | S_IWRITE);
        FILE *fp=fopen(&filename[1],"r");
        fseek(fp, 0L, SEEK_END);
        filesize =(long)ftell(fp);
        printf("%ld\n",filesize);
       
    }
    else if(strcmp(method_name,"HEAD")==0)
    {
        filesize=0;
    }
    
    job new_job=init_job(client_socket,input_buffer,filesize);
    scheduler_add_job(new_job);
    //close(client_socket); 
    //i=((i+1)%10);
    
    }
}

int main(int argc, char** argv) {
    
    
    int server_socket;
    int port,check,mode=0;
    pthread_t listener_thread,scheduler_thread;
    pthread_attr_t attr;
    struct sockaddr_in server_address,client_address;
    server_socket=socket(AF_INET,SOCK_STREAM,0);//create socket
    
    port=atoi(argv[1]);
    server_address.sin_family=AF_INET;
    server_address.sin_addr.s_addr=htons(INADDR_ANY);
    server_address.sin_port=htons(port);
    
    bind(server_socket,(struct sockaddr *)&server_address,sizeof(server_address));//bind
    printf("server running on port %d \n",port);
   
    listen(server_socket,10);//listen 
    
    printf("before create\n");
    scheduler_init(mode);
    printf("after scheduler_init\n");
    struct threadpool *tp;
    tp=(struct threadpool*)malloc(sizeof(struct threadpool));
    tp=threadpool_init(thread_pool_size);
    printf("after initiating thread pool\n");
    //t=argv[]; initialize t
    pthread_attr_init(&attr);
    pthread_create(&listener_thread,&attr,listener_thread_func,(void *)&server_socket);//create listener thread
    printf("after listener creation\n");
    pthread_create(&scheduler_thread,&attr,scheduler_thread_func,tp);//create scheduler thread
    printf("after scheduler create\n");
    
    pthread_join(listener_thread,NULL);
    pthread_join(scheduler_thread,NULL);
    printf("after join\n");
    //close(server_socket);
    return (0);
}
