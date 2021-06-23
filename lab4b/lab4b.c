//NAME: Bryan Luo
//EMAIL: luobryan@ucla.edu
//ID: 605303956

#include <sys/time.h>
#include <getopt.h> 
#include <stdio.h> 
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#define B 4275
#define R0 100000.0

#ifdef DUMMY
#define MRAA_GPIO_IN 0 
#define MRAA_GPIO_EDGE_RISING 1
typedef int mraa_aio_context;
typedef int mraa_gpio_context;
mraa_aio_context mraa_aio_init(int p){
    p = 1; 
    return p; 
}
int mraa_aio_read(mraa_aio_context c){
    	c = 650; 
	return c; 
}
void mraa_aio_close(mraa_aio_context c){
	int temp = c;
	(void)temp; 
}

mraa_gpio_context mraa_gpio_init(int p){
    p = 1; 
    return p; 
}
void mraa_gpio_dir(mraa_gpio_context c, int d){
    int x = c+d;
    (void)x; 
}
int mraa_gpio_read(mraa_gpio_context c){
    	c = 0; 
	return c; 
}
void mraa_gpio_close(mraa_gpio_context c){
	c = 5;
	(void)c;
}
int mraa_gpio_isr(int a, int b, void * fptr, void* arg){
	(void)a;
	(void)b;
	(void)fptr;
	(void)arg;
	return 0;
}
#else
#include <mraa.h>
#include <mraa/aio.h>
#endif
int A0= 1; 
int GPIO_115 = 60; 

bool logging = false; 
bool program_stopped = false;
char* log_file_name;
FILE* file_fd; 
char buffer[1024]; 
float convert_temper_reading(int reading, char type){
    float R1 = 1023.0/((float)reading)-1.0; 
    R1 = R0*R1; 
    double result = R1/R0;
    float C = 1.0/(log(result)/B+1/298.15)-273.15; 
    float F = (C*9)/5+32; 

    if(type=='C'){
        return C;
    }
    return F; 
}
void do_when_pushed(){
    struct timespec ts; 
    struct tm* tm; 
    clock_gettime(CLOCK_REALTIME, &ts);
    tm = localtime(&(ts.tv_sec)); 
//    if(program_stopped==false){
        printf("%02d:%02d:%02d SHUTDOWN\n",tm->tm_hour,tm->tm_min,tm->tm_sec); 
        if(logging){
            fprintf(file_fd,"%02d:%02d:%02d SHUTDOWN\n",tm->tm_hour,tm->tm_min,tm->tm_sec);
        }
 //   }
    exit(0);
}

int main(int argc, char *argv[]){
    log_file_name = (char*) malloc(1024*sizeof(char));
    
    struct option args[] = {
		{"period",1,NULL,'p'},  
        {"scale",1,NULL,'s'},
        {"log",1,NULL,'l'},
		{0,0,0,0}
	}; 
    long period = 1; 
    char scale = 'F';

    int i; 
    while((i=getopt_long(argc,argv,"",args,NULL)) != -1){
        switch(i){
            case 'p':
                period = strtol(optarg,NULL,10); 
                break;
            case 's':
                scale = *optarg; 
                break;
            case 'l':
                logging = true; 
                strcpy(log_file_name,optarg); 
                break;
            default:
                fprintf(stderr, "usage: accepted options are [--period=#,--scale=F/C,--log=filename\n");
                exit(1); 
                break;
        }
    }

    if(logging){
        file_fd = fopen(log_file_name,"a");
        if(file_fd==NULL){
            fprintf(stderr,"error in opening file\n");
            exit(1); 
        }
    }
    mraa_aio_context dev = mraa_aio_init(A0);

    mraa_gpio_context button = mraa_gpio_init(GPIO_115); 
    mraa_gpio_dir(button,MRAA_GPIO_IN); 
    mraa_gpio_isr(button,MRAA_GPIO_EDGE_RISING,&do_when_pushed,NULL); 

    struct pollfd pollStdin = {0, POLLIN, 0}; 
    struct timeval clock; 
    int past_time = 0; 
    while(true){
        //print time and temp as appropriate
        gettimeofday(&clock,0);
        if(program_stopped==false&&clock.tv_sec-past_time>=period){ //will initially be greater than period, then past_time will be set to clock.tv_sec
            int reading = mraa_aio_read(dev);
            float t = convert_temper_reading(reading,scale); 

            struct timespec ts; 
            struct tm* time; 
            clock_gettime(CLOCK_REALTIME, &ts);
            time = localtime(&(ts.tv_sec)); 

            printf("%02d:%02d:%02d %.1f\n",time->tm_hour,time->tm_min,time->tm_sec,t);
            if(logging){
                fprintf(file_fd,"%02d:%02d:%02d %.1f\n",time->tm_hour,time->tm_min,time->tm_sec,t);
            }
            past_time = clock.tv_sec; 
        }
        int ret = poll(&pollStdin,1,50);
        if(ret>=1){
            if(pollStdin.revents & POLLIN){
                int n_bytes = read(0,buffer,1024); 
                if (n_bytes < 0){
                    fprintf(stderr,"error in reading from stdin\n");
                    exit(1); 
                }
                int index_of_command_beginning = 0;
                for(i = 0; i < n_bytes; i++){
                    if(buffer[i]=='\n'){
                        buffer[i] = '\0';
                        bool off = false; 
                        if(strcmp(buffer+index_of_command_beginning,"SCALE=F")==0){
                            scale = 'F'; 
                        }
                        else if(strcmp(buffer+index_of_command_beginning,"SCALE=C")==0){
                            scale = 'C'; 
                        }
                        else if(strncmp(buffer+index_of_command_beginning,"PERIOD=",7)==0){
                            int temp_index_of_command_beginning = index_of_command_beginning; 
                            while(temp_index_of_command_beginning!=n_bytes&&buffer[temp_index_of_command_beginning]!='='){
				temp_index_of_command_beginning++; 
                            }
                            //at this point, we're at the equal sing
                            temp_index_of_command_beginning++; 
                            //now, we're at the digit
                            long period_val = strtol(buffer+temp_index_of_command_beginning,NULL,10);
                            if(period_val<=0){
                                fprintf(stderr,"a period must be a positive integer\n"); 
                                exit(1); 
                            }
                            period = period_val; 
                        }
                        else if(strcmp(buffer+index_of_command_beginning,"STOP")==0){
                            program_stopped = true;
                        }
                        else if(strcmp(buffer+index_of_command_beginning,"START")==0){
                            program_stopped = false; 
                        }
                        else if(strncmp(buffer+index_of_command_beginning,"LOG",3)==0){
                            //don't need to do anything for now
                        }
                        else if(strcmp(buffer+index_of_command_beginning,"OFF")==0){
                            off = true;
                        }

                        if(logging){
                            fprintf(file_fd,"%s\n",buffer+index_of_command_beginning);
                            fflush(file_fd); 
                        }
                        if(off){
                            do_when_pushed(); 
                        }
                        index_of_command_beginning = i + 1; 
                    }
                }

            }

        }
        else if(ret == 0){
            //time out
        }
        else{
            fprintf(stderr,"poll error\n");
            exit(1); 
        }

    }

    mraa_gpio_close(button);
    mraa_aio_close(dev); 
    free(log_file_name);

}
