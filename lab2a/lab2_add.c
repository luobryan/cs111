/*
NAME: Bryan Luo
EMAIL: luobryan@ucla.edu
ID: 605303956
*/
#include <getopt.h> 
#include <time.h> 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sched.h>
#include <string.h> 
#include <math.h>

long long sum = 0; 
bool yield = false; 
pthread_mutex_t mutex;
long lock = 0;
char lock_type = 'd'; //d for default (i.e. no synchorization)

static inline unsigned long get_nanosec_from_timespec(struct timespec* spec){
    unsigned long ret = spec->tv_sec; 
    ret = ret * 1000000000 + spec->tv_nsec; 
    return ret; 
}
void add(long long *pointer, long long value, char x){
    long long my_sum; 
    long long original_sum;
    switch(x){
        case 'm':
            pthread_mutex_lock(&mutex); 
            my_sum = *pointer + value; 
            if(yield)
                sched_yield(); 
            *pointer = my_sum; 
            pthread_mutex_unlock(&mutex); 
            break;
        case 's':
            while (__sync_lock_test_and_set(&lock,1)); 
            my_sum = *pointer + value; 
            if(yield)
                sched_yield(); 
            *pointer = my_sum; 
            __sync_lock_release(&lock);
            break;
        case 'c':
            do{
                original_sum = sum; //2nd parT?
                my_sum = original_sum + value; 
                if(yield)
                    sched_yield(); 
            }while(__sync_val_compare_and_swap(&sum,original_sum, my_sum)!=original_sum);
            break;
        default:
            my_sum = *pointer + value; 
            if(yield)
                sched_yield(); 
            *pointer = my_sum; 
            break; 
    }
}

void* thread_worker(void *iteration){
    long unsigned int i;
    unsigned long iter = *((unsigned long*)iteration); 
    for (i = 0; i < iter; i++){
        add(&sum,1,lock_type);
    }
    for (i = 0; i < iter; i++){
        add(&sum,-1,lock_type);
    }
    return NULL; 
}
int main(int argc, char* argv[]){
    int i;
    struct option args[] = {
		{"threads",1,NULL,'t'},  
        {"iterations",1,NULL,'i'},
        {"sync",1,NULL,'s'},
        {"yield",0,NULL,'y'},
		{0,0,0,0}
	}; 
    long num_threads = 1; 
    long num_iterations = 1; 
    while ( (i=getopt_long(argc,argv,"",args,NULL)) != -1){
        switch(i)
        {
            case 't':
                num_threads = strtol(optarg,NULL,10); 
		        break;
            case 'i':
                num_iterations = strtol(optarg,NULL,10); 
                break; 
            case 'y':
                yield = true; 
                break; 
            case 's':
                lock_type = *optarg; 
                if(lock_type=='m'){
                    pthread_mutex_init(&mutex,NULL); 
                }
                break; 
            default:
                fprintf(stderr, "usage: accepted options are [--threads=#, --iterations=#, --sync=[m,s,c], --yield]\n");
                exit(1); 
                break;
        }
    }
    struct timespec begin, end;
    unsigned long time_diff = 0;
    pthread_t threads[num_threads]; 
    clock_gettime(CLOCK_MONOTONIC, &begin); 
    //Do the timed tasks
    for (i = 0; i < num_threads; i++){
        if(pthread_create(&threads[i], NULL, &thread_worker, &num_iterations)!=0){
            fprintf(stderr,"error with creating threads"); 
            exit(1);
        }
    }
    for (i = 0; i < num_threads; i++){
        if(pthread_join(threads[i], NULL)!=0){
            fprintf(stderr,"error in joining threads"); 
            exit(1);
        }
    }

    //End the timed tasks
    if(clock_gettime(CLOCK_MONOTONIC, &end)!=0){
        fprintf(stderr,"error in getting time"); 
        exit(1);
    } 
    time_diff = get_nanosec_from_timespec(&end)-get_nanosec_from_timespec(&begin); 

    //test type
    char s[] = "add-";
    if(yield){
        strcat(s,"yield-");
    }
    switch(lock_type){
        case 'm':
            strcat(s,"m");
            break;
        case 's':
            strcat(s,"s");
            break;
        case 'c':
            strcat(s,"c");
            break;
        default:
            strcat(s,"none");
            break; 
    }
    //Print stuff 
    printf("%s,%lu,%lu,%lu,%lu,%lu,%lld\n", s, num_threads, num_iterations, 2*num_threads*num_iterations, time_diff, time_diff/(2*num_threads*num_iterations),sum);

    return 0; 
}
