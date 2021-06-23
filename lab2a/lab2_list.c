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
#include <stdio.h>
#include <signal.h> 
#include <stdlib.h>
#include "SortedList.h"

int opt_yield = 0; 
long num_iterations = 1;
char sync_type = 'n'; //n for no sync , by default
SortedListElement_t *listhead;
SortedListElement_t *pool; 
pthread_mutex_t mutex;
long num_threads; 
long lock = 0; 

static inline unsigned long get_nanosec_from_timespec(struct timespec* spec){
    unsigned long ret = spec->tv_sec; 
    ret = ret * 1000000000 + spec->tv_nsec; 
    return ret; 
}
void segfault_handler(){
        fprintf(stderr,"caught a segmentation fault. sync type (n means no sync): %c\n",sync_type); 
        exit(2); 
}

void* thread_worker(void *threadNum){
   long i;
    int thread_num = *((int*)threadNum);
    long startIndex = thread_num*num_iterations;     
   for (i = startIndex; i < startIndex+num_iterations; i++){
	    if(sync_type=='s'){
        while (__sync_lock_test_and_set(&lock,1));
       }
      else if(sync_type=='m'){
        pthread_mutex_lock(&mutex);

        }
                        SortedList_insert(listhead,&pool[i]); 
	    if(sync_type=='s'){
        __sync_lock_release(&lock);
         }
          else if(sync_type=='m'){
        pthread_mutex_unlock(&mutex);
        }
    }
    if(sync_type=='s'){
        while (__sync_lock_test_and_set(&lock,1));
    }
    else if(sync_type=='m'){
        pthread_mutex_lock(&mutex);

    }
                            if(SortedList_length(listhead)==-1){  
                                fprintf(stderr,"corrupt list. sync type (n means no sync):  %c \n",sync_type); 
                                exit(2);
                            }
       if(sync_type=='s'){
        __sync_lock_release(&lock);
         }
          else if(sync_type=='m'){
        pthread_mutex_unlock(&mutex);
        }  
    for (i = startIndex; i < startIndex+num_iterations; i++){
       if(sync_type=='s'){
        while (__sync_lock_test_and_set(&lock,1));
    }
    else if(sync_type=='m'){
        pthread_mutex_lock(&mutex);

    }
  
                    SortedListElement_t* e = SortedList_lookup(listhead, (pool[i].key));
                        if(e == NULL){
                            fprintf(stderr,"corrupt list, since couldnt not find the item in list. sync type (n means no sync): %c\n",sync_type); 
                            exit(2); 
                        }
                        if(SortedList_delete(e)==1){
                            fprintf(stderr,"corrupt list, error in deleting. sync type (n means no sync): %c\n",sync_type); 
                            exit(2);
                        }
       if(sync_type=='s'){
        __sync_lock_release(&lock);
         }
          else if(sync_type=='m'){
        pthread_mutex_unlock(&mutex);
        }
    }
    return NULL; 
}

int main(int argc, char* argv[]){
    struct option args[] = {
		{"threads",1,NULL,'t'},  
        {"iterations",1,NULL,'i'},
        {"yield",1,NULL,'y'},
        {"sync",1,NULL,'s'},
		{0,0,0,0}
	}; 
    signal(SIGSEGV,segfault_handler); 
    num_threads = 1;  
    int i; 
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
                for (size_t x = 0; x < strlen(optarg); x++){
                    if(optarg[x] == 'i'){
                        opt_yield = opt_yield | INSERT_YIELD;
                    }
                    if(optarg[x] == 'd'){
                        opt_yield = opt_yield | DELETE_YIELD; 
                    }
                    if(optarg[x] == 'l'){
                        opt_yield = opt_yield | LOOKUP_YIELD; 
                    }
                }
                break; 
            case 's':
                sync_type = *optarg; 
                if(sync_type == 'm'){
                    pthread_mutex_init(&mutex,NULL);
                }
                break;
            default:
                fprintf(stderr, "usage: accepted options are [--threads=#,--iterations=#,--yield=[idl]]\n");
                exit(1); 
                break;
        }
    }
    struct timespec begin, end;
    unsigned long time_diff = 0;
    pthread_t threads[num_threads]; 
    //initialize the list head
    listhead = (SortedList_t*) malloc(sizeof(SortedList_t)); 
    listhead -> prev= listhead;
    listhead -> key = NULL; 
    listhead -> next= listhead;
    pool = (SortedList_t*) malloc(num_threads*num_iterations*sizeof(SortedListElement_t)); 
    char* my_keys = (char*) malloc(num_threads*num_iterations*sizeof(char)); 
   for (i = 0; i < num_threads * num_iterations; i++){
      my_keys[i] = 'a'+(rand()%26);
      pool[i].key = &my_keys[i];  
    }
    if(clock_gettime(CLOCK_MONOTONIC, &begin)!=0){
        fprintf(stderr,"error in getting time"); 
        exit(1);
    } 
    //Do the timed tasks
    int* my_thread_nums = (int*) malloc(num_threads*sizeof(int)); 
    for (i = 0; i < num_threads; i++){
        my_thread_nums[i] = i; 
        if(pthread_create(&threads[i], NULL, &thread_worker, &my_thread_nums[i])!=0){
            fprintf(stderr,"error in creating threads");
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
    int list_len = SortedList_length(listhead);
    if(list_len==-1){
        fprintf(stderr,"corrupt list. sync type (n means no sync): %c \n",sync_type);
        exit(2);
    }
    if(list_len!=0){
	fprintf(stderr,"corrupt list. length did not end at 0. sync type (n means no sync):%c \n", sync_type);
	exit(2);
    }
    time_diff = get_nanosec_from_timespec(&end)-get_nanosec_from_timespec(&begin); 
    free(pool); 
   free(listhead);
   free(my_keys);
   free(my_thread_nums);
   
    //Print stuff 
    char test_name[] = "list-"; 
    bool at_least_one_yield = 0; 
    if(opt_yield & INSERT_YIELD){
        strcat(test_name,"i");
	at_least_one_yield = 1;
    }
    if(opt_yield & DELETE_YIELD){
        strcat(test_name,"d");
	at_least_one_yield = 1;
    }
    if(opt_yield & LOOKUP_YIELD){
        strcat(test_name,"l"); 
    	at_least_one_yield = 1;
	}
    if(at_least_one_yield == 1){
   strcat(test_name,"-");
    }
    else{
	strcat(test_name,"none-"); 
    }
    char syncopts[] = "none";
    if(sync_type=='s'){
        strcpy(syncopts,"s");
    }
    if(sync_type=='m'){
        strcpy(syncopts,"m"); 
    }
    strcat(test_name,syncopts);

    printf("%s,%lu,%lu,%d,%lu,%lu,%lu\n",test_name,num_threads,num_iterations,1,num_threads*num_iterations*3,time_diff,time_diff/(num_threads*num_iterations*3));


    return 0; 
}



