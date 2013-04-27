
#include<semaphore.h>
#include<pthread.h>


struct job_details
{
    int client_socket;
    long filesize;
    char *request;
};

struct th_job
{
    struct th_job *link;
    struct job_details *param;
};

struct th_job_queue
{
    struct th_job *front;
    struct th_job *rear;
    int NoOfJobs;
    sem_t *sem_queue;
};

struct threadpool
{
    pthread_t *thread_ptr;
    int NoOfThreads;
    struct th_job_queue* job_queue;
};


struct threadpool* threadpool_init(int noofthreads);
void* request_assign(void *tp);
void threadpool_assign_task(struct threadpool *tp,struct job_details *parameter);
void add_job_queue(struct threadpool *tp,struct th_job *job);
void remove_job_queue(struct threadpool *tp);
void threadpool_finish(struct threadpool* tp);
void request_handle(struct job_details *parameter);
