#include "scheduler.h"
#include <algorithm>
#include <queue>
#include <vector>
#include <iostream>
#include <stdio.h>
vector<job> job_v;
queue<job> job_q;

int mode = -1;
pthread_mutex_t scheduler_joblist_mutex=PTHREAD_MUTEX_INITIALIZER;
sem_t sem_full, sem_empty;
void scheduler_init(int arg_mode){
    mode = arg_mode;
    pthread_mutex_init(&scheduler_joblist_mutex, NULL);
    sem_init(&sem_full, 0, 0);
    sem_init(&sem_empty, 0, 2000);
}

job init_job(int client_socket,char * request, long len){
    
    //job front = new job();//Java Effect
    //break 2;
    printf("In init_job\n");
    job newjob(client_socket,request,len);
    return newjob;
}

void scheduler_add_job(job new_job){
    //break 3;
    job temp1,temp2;
    sem_wait(&sem_empty);
    printf("sem_empty: %d\n",sem_empty);
    pthread_mutex_lock(&scheduler_joblist_mutex);
    
    printf("In scheduler_add_job\n");
    
     switch (mode){
        case FIFO:
            job_q.push(new_job);
            printf("adding:%s\n",new_job.request);
            temp1=job_q.front();
            printf("queue front:%s\n",temp1.request);
            temp2=job_q.back();
            printf("queue back:%s\n",temp2.request);
            break;
        case SJF:
            job_v.push_back(new_job);
            push_heap(job_v.begin(), job_v.end(), CompareFoo());
            break;
        default:
            job_q.push(new_job);
            break;
    }
    pthread_mutex_unlock(&scheduler_joblist_mutex);
    sem_post(&sem_full);
}

job scheduler_retrive_job(){
    job output;
    job temp;
    printf("In retrieve job :before request\n");
    sem_wait(&sem_full);
    
    pthread_mutex_lock(&scheduler_joblist_mutex);
    printf("In retrieve job: after a request arrival\n");
    switch (mode){
        case FIFO:
            output=job_q.front();
            //output=job_q.back();
            //printf("queue length:%d\n",job_q.size());
            printf("front:%s\n",output.request);
           //printf("back:%s\n",temp.request);*/
            job_q.pop();
            
            break;
        case SJF:
            output = job_v.front();
            pop_heap(job_v.begin(), job_v.end(), CompareFoo());
            job_v.pop_back();
            break;
        default:
            output=job_q.front();
            printf("retrieving:%s\n",output.request);
            job_q.pop();
            break;
    }
    pthread_mutex_unlock(&scheduler_joblist_mutex);
    sem_post(&sem_empty);
    return output;
}
