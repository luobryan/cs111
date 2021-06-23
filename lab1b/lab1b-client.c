//NAME: Bryan Luo
//EMAIL: luobryan@ucla.edu
//ID: 605303956

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <getopt.h> 
#include <stdbool.h> 
#include "zlib.h"
#include <sys/stat.h>
#include <fcntl.h> 

#define BUFFERSIZE 2048
char buffer[BUFFERSIZE]; 

void terminal_setup(void){
	struct termios tmp;
	if(tcgetattr(0,&tmp)<0){
		fprintf(stderr,"error in getting terminal settings"); 
		exit(1); 
	}
	tmp.c_iflag = ISTRIP; 
	tmp.c_oflag = 0; 
	tmp.c_lflag = 0; 
	if(tcsetattr(0, TCSANOW, &tmp)<0){
		fprintf(stderr,"error in setting terminal settings."); 
		exit(1); 
	} 

}

int client_connect(char* hostname, unsigned int port){
    int sockfd; 
    struct sockaddr_in serv_addr; 
    struct hostent* server;

    //create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //fill in socket address information
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); 
    server = gethostbyname(hostname); 
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length); 
    memset(serv_addr.sin_zero, '\0', sizeof(serv_addr.sin_zero)); 

    //connect socket with corresponding address
    connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)); 
    return sockfd; 
}

int main(int argc, char* argv[]){
    //process commandline arguments 
    int port = -1; 
    bool compress = false; 
    bool log = false;
    struct option args[] = {
		{"port",1,NULL,'p'},
        {"compress",0,NULL,'c'},
        {"log",1,NULL,'l'},
		{0,0,0,0}
	}; 
    int i; 
    int ofd; 
    int output_errno; 
    char output_file_name[BUFFERSIZE]; 
    while ( (i=getopt_long(argc,argv,"",args,NULL)) != -1){
        switch(i)
        {
            case 'p':
                port = strtol(optarg,NULL,10); 
		        break;
            case 'c':
                compress = true;
                break; 
            case 'l':
                log = true;
                ofd = open(optarg, O_RDWR | O_CREAT | O_TRUNC, 0777);
                strcpy(output_file_name,optarg);
                if (ofd < 0){
                    output_errno = errno; 
                }
                break; 
            default:
                fprintf(stderr, "usage: accepted options are [--port= --log= --compress]\n");
                exit(1); 
                break;
        }
    }

    //deal with bad output file
    if (ofd < 0){
        fprintf(stderr,"%s: %s\n",output_file_name,strerror(output_errno)); 
        exit(1); 
    }
    //from piazza post, we can assume the port # will be valid
    if (port == -1){ //means no port # was inputted
        fprintf(stderr, "error: you must input a port # using the --port option"); 
        exit(1); 
    }


    //set terminal to non-canonical, no echo mode 
    struct termios original;
    if(tcgetattr(0, &original)<0){
		fprintf(stderr,"error in getting terminal settings");
		exit(1); 
	}
    terminal_setup();
    
    //connect socket
    int socket_fd = client_connect("localhost",port); 
    
    //set up poll
    struct pollfd pollfds[2];
	pollfds[0].fd = 0;
	pollfds[0].events = POLLIN + POLLHUP + POLLERR;
	pollfds[1].fd = socket_fd; 
	pollfds[1].events = POLLIN + POLLHUP + POLLERR; 

    //continuous loop until error
    while(1){
        //do the poll
        if(poll(pollfds,2,-1)<0){
			fprintf(stderr,"error in doing poll()"); 
			exit(1); 
		} //2 means 2 items in fds array, -1 means no timeout

        //check if anything to read in from stdin
        if(pollfds[0].revents & POLLIN){
          
            int numBytesRead = read(0,buffer,BUFFERSIZE); //read input from keyboard (stdin)
			if (numBytesRead<0){
				fprintf(stderr,"error in reading from stdin");
				exit(1); 
			}
            unsigned char outbuf[BUFFERSIZE];
            if(compress){
                z_stream strm;
                //initialize
                strm.zalloc = Z_NULL;
                strm.zfree = Z_NULL;
                strm.opaque = Z_NULL;
                int res = deflateInit(&strm,Z_DEFAULT_COMPRESSION); 
                if (res != Z_OK){
                    fprintf(stderr,"error in the compression");
                    exit(1); 
                }
                //do the deflation
                strm.avail_in = numBytesRead; 
                strm.next_in = (unsigned char *) buffer;
                strm.next_out = outbuf; 
                strm.avail_out = BUFFERSIZE; 
                while(strm.avail_in > 0){
                    deflate(&strm, Z_SYNC_FLUSH); 
                }
                //
                if(write(socket_fd, outbuf, BUFFERSIZE-strm.avail_out)<0){
                        fprintf(stderr,"error in writing to server"); 
                        exit(1);
                }   
                if(log){
                    char to_log[BUFFERSIZE]; 
                    char num_bytes[BUFFERSIZE]; 
                    sprintf(num_bytes,"%d",BUFFERSIZE-strm.avail_out);  //the # of bytes we have read is buffersize of compressed data minus available bytes
                    strcpy(to_log,"SENT ");
                    strcat(to_log, num_bytes);
                    strcat(to_log," bytes: ");
                    strcat(to_log,(char*)outbuf);
                    strcat(to_log,"\n");
                    if(write(ofd, to_log,strlen(to_log))<0){
                        fprintf(stderr,"error printing to log");
                        exit(1); 
                    } 
                }
                //
                deflateEnd(&strm); 
            }
            else{
                if(write(socket_fd, buffer, numBytesRead)<0){
                        fprintf(stderr,"error in writing to server"); 
                        exit(1);
                }                   
                if(log){
                    char to_log[BUFFERSIZE]; 
                    char num_bytes[BUFFERSIZE]; 
                    sprintf(num_bytes,"%d",numBytesRead); 
                    strcpy(to_log,"SENT ");
                    strcat(to_log, num_bytes);
                    strcat(to_log," bytes: ");
                    strcat(to_log,buffer);
                    strcat(to_log,"\n");
                    if(write(ofd, to_log,strlen(to_log))<0){
                        fprintf(stderr,"error in writing to log");
                        exit(1); 
                    } 
                }
            }
            for(i = 0; i < numBytesRead; i++){
                    if(buffer[i] == '\r' || buffer[i] == '\n'){
                        if(write(1,"\r\n",2)<0){
                            fprintf(stderr,"error writing to stdout");
                            exit(1);
                        }
                    }
                    else{
                        if(write(1,&buffer[i],1)<0){
                            fprintf(stderr,"error writing to stdout");
                            exit(1); 
                        }
                    }
            }  
        }

        
        //check if socket_fd has anything to read in
        if(pollfds[1].revents & POLLIN){
            int numBytes = read(socket_fd,buffer,BUFFERSIZE); 
            if (numBytes<0){
                fprintf(stderr,"error in reaeding from server"); 
                exit(1);
            }
            if (numBytes == 0){
                //close file descriptor
                 close(socket_fd);

                //reset terminal mode
                if(tcsetattr(0, TCSANOW,&original)<0){
	        	    fprintf(stderr,"error in setting terminal settings back to original");
	        	    exit(1); 
        	    }
                 return 0; 
            }
            
            if(log){
                    char to_log[BUFFERSIZE]; 
                    char num_bytes[BUFFERSIZE]; 
                    sprintf(num_bytes,"%d",numBytes); 
                    strcpy(to_log,"RECEIVED ");
                    strcat(to_log, num_bytes);
                    strcat(to_log," bytes: ");
                    strcat(to_log,buffer);
                    strcat(to_log,"\n");
                    if(write(ofd, to_log,strlen(to_log))<0){
                        fprintf(stderr,"error writing to output file");
                        exit(1);
                    } 
            }
            unsigned char outbuf[BUFFERSIZE];
            if(compress){
                z_stream strm;
                //initialize
                strm.zalloc = Z_NULL;
                strm.zfree = Z_NULL;
                strm.opaque = Z_NULL;
                int res = inflateInit(&strm); 
                if (res != Z_OK){
                    fprintf(stderr,"error in the compression");
                    exit(1); 
                }
                //do the inflation
                strm.avail_in = numBytes; 
                strm.next_in = (unsigned char *) buffer;
                strm.next_out =  outbuf; 
                strm.avail_out = BUFFERSIZE; 
                while(strm.avail_in > 0){
                    inflate(&strm, Z_SYNC_FLUSH); 
                }
                //
                memcpy(buffer,outbuf,BUFFERSIZE-strm.avail_out); 
                numBytes = BUFFERSIZE-strm.avail_out;
                //
                inflateEnd(&strm); 
            }
            for(i = 0; i < numBytes; i++){
                if(buffer[i] == '\n'){  
							if(write(1,"\r\n",2)<0){
								fprintf(stderr,"error in wriitng to stdout"); 
								exit(1); 
							}
				}
				else{
							if(write(1,&buffer[i],1)<0){
								fprintf(stderr,"error in writing to stdout"); 
								exit(1); 
							}
				}
            }
        }

        //check for errors
       if(pollfds[0].revents & (POLLHUP | POLLERR)){
            //close file descriptor
            close(socket_fd);

            //reset terminal mode
            if(tcsetattr(0, TCSANOW,&original)<0){
	        	fprintf(stderr,"error in setting terminal settings back to original");
	        	exit(1); 
        	}
            return 0; 
        }
        if(pollfds[1].revents & (POLLHUP | POLLERR)){
            //close file descriptor
            close(socket_fd);

            //reset terminal mode
            if(tcsetattr(0, TCSANOW,&original)<0){
	        	fprintf(stderr,"error in setting terminal settings back to original");
	        	exit(1); 
        	}
            return 0; 
        }
}
}