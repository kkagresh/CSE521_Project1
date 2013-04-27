/**
 compile: g++ -c scheduler.cpp
 or: gcc -c scheduler.cpp -lstdc++
*/


#include <pthread.h>
#include <semaphore.h>


const int FIFO = 0;
const int SJF = 1;

using namespace std;

class job {
    public:
        
        int clientsocket;
        char * request;
        long cmp;
        job()
        {
            
        }
        job(int s,char * f, long c){
            clientsocket= s;
            request=f;
            cmp = c;            
        }
};
// comparator object for min heap
class CompareFoo
{
    public:
        bool operator()(job& x, job& y)
        {
           return x.cmp > y.cmp;
        }
};

void scheduler_init(int arg_mode);
job init_job(int s,char * f, long c);
void scheduler_add_job(job new_job);
job scheduler_retrive_job();
