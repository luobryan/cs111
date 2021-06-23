/*
NAME: Bryan Luo
EMAIL: luobryan@ucla.edu
ID: 605303956
*/
#include <getopt.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h> 
#include <stdio.h> 
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#define BUFFERSIZE 1024
char buffer[BUFFERSIZE];

void sigsegv_handler(){
    fprintf(stderr, "%s", "error: segmentation fault caught\n");
    exit(4); 
}
struct option args[] = {
    {"input",1,NULL,'i'},   
    {"output",1,NULL,'o'},
    {"segfault",0,NULL,'s'},
    {"catch",0,NULL,'c'},
    {0,0,0,0}
};

int main(int argc, char* argv[]){
    int ifd = 0; 
    int ofd = 0; 
    char* input_file_name;
    char* output_file_name;
    bool catch = false; 
    bool seg = false; 
    bool input = false;
    bool output = false;
    int input_errno = 0; 
    int output_errno = 0; 
    int i;
    while ( (i=getopt_long(argc,argv,"",args,NULL)) != -1){
        switch(i)
        {
            case 'i':
		input = true;
                ifd = open(optarg,O_RDONLY); 
                input_file_name = malloc(strlen(optarg)); 
                strcpy(input_file_name,optarg);
                if (ifd < 0){
			input_errno = errno;		
		}
		break;
            case 'o':
		output = true;
                ofd = open(optarg,O_RDWR | O_CREAT, 0644);
                output_file_name = malloc(strlen(optarg)); 
                strcpy(output_file_name,optarg);
                if (ofd < 0){
			output_errno = errno;
		}
		break;
            case 's':
                seg = true;
                break;
            case 'c':
                catch = true;
                break;
            default:
                fprintf(stderr, "usage: accepted options are [--input filename --output filename --segfault --catch]\n");
                exit(1); 
                break;
        }
    }
    if (ifd > 0){
        close(0);
        dup(ifd);
        close(ifd);
    }
    else if (ifd < 0){
        fprintf(stderr,"%s: %s\n", input_file_name, strerror(input_errno));
        exit(2);
    }
    if (ofd > 0){
        close(1);
        dup(ofd);
        close(ofd); 
    }
    else if (ofd < 0){
        fprintf(stderr,"%s: %s\n", output_file_name, strerror(output_errno));
        exit(3); 
    }
    if (catch){
        signal(SIGSEGV, sigsegv_handler); 
    }
    if (seg){
        char* ptr = NULL;
        (*ptr) = 0; 
    }
    int ret; 
    while((ret = read(0,buffer,BUFFERSIZE)) > 0){
        write(1,buffer,ret); 
    }

    if(input){
	free(input_file_name);
    }
    if(output){
        free(output_file_name);
    }
    exit(0);
}
